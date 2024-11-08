#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    FIELD_INT64,
    FIELD_UINT64,
    FIELD_FLOAT64,
    FIELD_INT,   // 32bit
    FIELD_UINT,  // 32bit
    FIELD_FLOAT, // 32bit
    FIELD_INT16,
    FIELD_UINT16,
    FIELD_INT8,
    FIELD_UINT8,
    FIELD_STRING,
    FIELD_SUBMESSAGE
} FieldType;

typedef struct {
    size_t offset;
    FieldType type;
    const void* submsg_fields;
    int submsg_field_count;
} FieldDescriptor;

// Helper macro to get offset of a field in a struct
#define FIELD_OFFSET(type, field) ((size_t)&((type*)0)->field)

// Macro to create a field descriptor for basic types
#define LIGHTRPC_FIELD(type, field, field_type) { FIELD_OFFSET(type, field), field_type, NULL, 0 }

// Macro to create a field descriptor for nested structs
#define LIGHTRPC_FIELD_STRUCT(type, field, nested_fields, count) { FIELD_OFFSET(type, field), FIELD_SUBMESSAGE, nested_fields, count }

// Helper functions for serialization
static void write_uint16(uint8_t* buf, uint16_t value) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
}

static uint16_t read_uint16(const uint8_t* buf) {
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

int lightrpc_serialize(uint8_t* buffer, const void* data, uint16_t cmd, const FieldDescriptor* fields, int field_count) {
    int offset = 0;
    write_uint16(buffer + offset, cmd);
    offset += 2;

    for (int i = 0; i < field_count; i++) {
        const FieldDescriptor* field = &fields[i];
        const uint8_t* field_data = (const uint8_t*)data + field->offset;

        if (field->type == FIELD_STRING) {
            const char* str = *(const char**)field_data;
            if (!str) {
                write_uint16(buffer + offset, 0);
                offset += 2;
                continue;
            }
            uint16_t len = strlen(str);
            write_uint16(buffer + offset, len);
            memcpy(buffer + offset + 2, str, len);
            offset += 2 + len;
        }
        else if (field->type == FIELD_SUBMESSAGE) {
            int sublen = lightrpc_serialize(buffer + offset, field_data, 0,
                field->submsg_fields, field->submsg_field_count);
            write_uint16(buffer + offset, sublen);
            offset += sublen;
        }
        else {
            int size;
            switch (field->type) {
                case FIELD_FLOAT64:
                case FIELD_INT64:
                case FIELD_UINT64: size = 8; break;
                case FIELD_INT:
                case FIELD_UINT:
                case FIELD_FLOAT: size = 4; break;
                case FIELD_INT16:
                case FIELD_UINT16: size = 2; break;
                case FIELD_INT8:
                case FIELD_UINT8: size = 1; break;
                default: continue;
            }
            write_uint16(buffer + offset, size);
            memcpy(buffer + offset + 2, field_data, size);
            offset += 2 + size;
        }
    }
    return offset;
}

int lightrpc_deserialize(const uint8_t* buffer, int buffer_len, void* data, const FieldDescriptor* fields, int field_count) {
    int offset = 2; // Skip command
    uint16_t cmd = read_uint16(buffer);

    for (int i = 0; i < field_count && offset < buffer_len; i++) {
        const FieldDescriptor* field = &fields[i];
        uint8_t* field_data = (uint8_t*)data + field->offset;
        uint16_t len = read_uint16(buffer + offset);
        offset += 2;

        if (len == 0) continue;

        if (field->type == FIELD_STRING) {
            char* str = malloc(len + 1);
            memcpy(str, buffer + offset, len);
            str[len] = '\0';
            *(char**)field_data = str;
            offset += len;
        }
        else if (field->type == FIELD_SUBMESSAGE) {
            lightrpc_deserialize(buffer + offset - 2, len + 2, field_data,
                field->submsg_fields, field->submsg_field_count);
            offset += len - 2;
        }
        else {
            memcpy(field_data, buffer + offset, len);
            offset += len;
        }
    }

    return cmd;
}
