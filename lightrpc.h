// use this in header in your C project to use lightrpc!
// https://github.com/konsumer/lightrpc

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum {
    FIELD_INT,
    FIELD_FLOAT,
    FIELD_STRING,
    FIELD_STRUCT,
    FIELD_CHAR
} FieldType;

// Extended field descriptor to include nested struct info
typedef struct {
    size_t offset;
    size_t size;
    FieldType type;
    const struct FieldDescriptor* nested_fields;
    int nested_field_count;
} FieldDescriptor;

// Modified macro for regular fields
#define FIELD_DESC(type, field, field_type) \
    { offsetof(type, field), sizeof(((type*)0)->field), field_type, NULL, 0 }

// New macro for struct fields
#define STRUCT_FIELD_DESC(type, field, nested_fields, count) \
    { offsetof(type, field), sizeof(((type*)0)->field), FIELD_STRUCT, nested_fields, count }

int lightrpc_serialize(uint8_t* buffer, const void* data,
                    uint8_t cmd,
                    const FieldDescriptor* fields,
                    int field_count) {
    int pos = 0;
    buffer[pos++] = cmd;

    for (int i = 0; i < field_count; i++) {
        const uint8_t* src = (const uint8_t*)data + fields[i].offset;

        switch (fields[i].type) {
            case FIELD_CHAR:
            case FIELD_INT:
            case FIELD_FLOAT:
                buffer[pos++] = fields[i].size;
                memcpy(buffer + pos, src, fields[i].size);
                pos += fields[i].size;
                break;

            case FIELD_STRING: {
                const char* str = *(const char**)src;
                uint8_t len = str ? strlen(str) : 0;
                buffer[pos++] = len;
                if (len > 0) {
                    memcpy(buffer + pos, str, len);
                    pos += len;
                }
                break;
            }

            case FIELD_STRUCT: {
                // Recursively serialize nested struct
                int nested_size = lightrpc_serialize(
                    buffer + pos + 1,  // +1 for size byte
                    src,
                    0,  // No cmd byte for nested structs
                    fields[i].nested_fields,
                    fields[i].nested_field_count
                );
                buffer[pos++] = nested_size;
                pos += nested_size;
                break;
            }
        }
    }

    return pos;
}

int lightrpc_deserialize(const uint8_t* buffer, int buffer_len,
                      void* data,
                      const FieldDescriptor* fields,
                      int field_count) {
    if (buffer_len < 1) return -1;

    int pos = 1;  // Skip command byte

    for (int i = 0; i < field_count; i++) {
        uint8_t* dst = (uint8_t*)data + fields[i].offset;

        if (pos >= buffer_len) return -1;
        uint8_t size = buffer[pos++];

        switch (fields[i].type) {
            case FIELD_INT:
            case FIELD_FLOAT:
                if (size != fields[i].size || pos + size > buffer_len) return -1;
                memcpy(dst, buffer + pos, size);
                pos += size;
                break;

            case FIELD_STRING: {
                if (pos + size > buffer_len) return -1;
                char** str_ptr = (char**)dst;
                *str_ptr = malloc(size + 1);
                if (size > 0) {
                    memcpy(*str_ptr, buffer + pos, size);
                }
                (*str_ptr)[size] = '\0';
                pos += size;
                break;
            }

            case FIELD_STRUCT: {
                // Recursively deserialize nested struct, but don't include size byte twice
                int result = lightrpc_deserialize(
                    buffer + pos,     // Start after size byte
                    size,             // Just use the size
                    dst,
                    fields[i].nested_fields,
                    fields[i].nested_field_count
                );
                if (result < 0) return -1;
                pos += size;
                break;
            }
        }
    }

    return pos;
}

// Helper to free dynamic strings
void lightrpc_free_strings(void* data,
                        const FieldDescriptor* fields,
                        int field_count) {
    for (int i = 0; i < field_count; i++) {
        if (fields[i].type == FIELD_STRING) {
            char** str_ptr = (char**)((uint8_t*)data + fields[i].offset);
            free(*str_ptr);
            *str_ptr = NULL;
        }
    }
}
