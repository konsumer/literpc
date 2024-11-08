// this is a tester for literpc

#include <stdio.h>
#include <assert.h>
#include "../literpc.h"

void dumphex(const void* data, size_t size) {
  char ascii[17];
  size_t i, j;
  ascii[16] = '\0';
  for (i = 0; i < size; ++i) {
    printf("%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      printf(" ");
      if ((i+1) % 16 == 0) {
        printf("|  %s \n", ascii);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          printf(" ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          printf("   ");
        }
        printf("|  %s \n", ascii);
      }
    }
  }
}

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    int id;
    Point location;
    char* name;
} Entity;

// Field descriptors for Point
const FieldDescriptor point_fields[] = {
    LITERPC_FIELD(Point, x, FIELD_FLOAT),
    LITERPC_FIELD(Point, y, FIELD_FLOAT)
};
const int point_fields_len = 2;

// Field descriptors for Entity
const FieldDescriptor entity_fields[] = {
    LITERPC_FIELD(Entity, id, FIELD_INT),
    LITERPC_FIELD_STRUCT(Entity, location, point_fields, point_fields_len),
    LITERPC_FIELD(Entity, name, FIELD_STRING)
};
const int entity_fields_len = 3;

int main(int argc, char *argv[]) {
  Entity entity = {
      .id = 123,
      .location = { .x = 1.0f, .y = 2.0f },
      .name = strdup("Test Entity")
  };

  uint8_t buffer[1024];
  int len = literpc_serialize(buffer, &entity, 1, entity_fields, entity_fields_len);

  Entity decoded = {0};
  int cmd = literpc_deserialize(buffer, len, &decoded, entity_fields, entity_fields_len);

  uint8_t expectedBuffer[] = {
    1,0,               // op = 1

    4,0,               // id:len = 4
    123,0,0,0,         // id:i32 = 123

    14,0,              // location:len = 14
    4,0,               // location.x:len = 4
    0,0,128,63,        // location.x:f32 = 1.0
    4,0,               // location.y:len = 4
    0,0,0,64,          // location.y:f32 = 2.0

    11,0,              // name:len = 11

    84, 101, 115, 116, // name:bytes
    32,  69, 110, 116,
    105, 116, 121
  };

  printf("Expected:\n");
  dumphex(expectedBuffer, sizeof(expectedBuffer));

  printf("\nReceived:\n");
  dumphex(buffer, len);

  assert(cmd == 1);
  assert(decoded.id == entity.id);
  assert(decoded.location.x == entity.location.x);
  assert(decoded.location.y == entity.location.y);
  assert(strcmp(decoded.name, entity.name) == 0);

  free(entity.name);
  free(decoded.name);
  printf("\nTest passed!\n");
  return 0;
}
