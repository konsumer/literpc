// this is a tester for lightrpc

#include <stdio.h>
#include "../lightrpc.h"

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
  FIELD_DESC(Point, x, FIELD_FLOAT),
  FIELD_DESC(Point, y, FIELD_FLOAT)
};

// Field descriptors for Entity
const FieldDescriptor entity_fields[] = {
  FIELD_DESC(Entity, id, FIELD_INT),
  STRUCT_FIELD_DESC(Entity, location, point_fields, 2),
  FIELD_DESC(Entity, name, FIELD_STRING)
};

int main() {
  uint8_t buffer[1024];

  Entity entity = {
    .id = 123,
    .location = { .x = 1.0f, .y = 2.0f },
    .name = strdup("Test Entity")
  };

  uint8_t expectedBuffer[] = {
      1,  0,  4,   0, 123,   0,   0,   0,  14,
      0,  4,  0,   0,   0, 128,  63,   4,   0,
      0,  0,  0,  64,  11,   0,  84, 101, 115,
    116, 32, 69, 110, 116, 105, 116, 121
  };

  printf("SERIALIZE:\n\n");

  int len = lightrpc_serialize(buffer, &entity, 1, entity_fields, 3);
  dumphex(buffer, len);
  printf("\n");

  for (int i=0;i<len;i++) {
    if (expectedBuffer[i] != buffer[i]) {
      printf("MISMATCH %d: %02X != %02X\n", i,  buffer[i], expectedBuffer[i]);
    }
  }

  printf("\nDESERIALIZE:\n");

  Entity decoded = {0};
  lightrpc_deserialize(buffer, len, &decoded, entity_fields, 3);
  printf("id: %d\nlocation: %fx%f\nname: %s\n", decoded.id, decoded.location.x, decoded.location.y, decoded.name);

  if (decoded.id != entity.id) {
    printf("MISMATCH id: %d != %d\n", decoded.id, entity.id);
  }
  if (strcmp(decoded.name, entity.name) != 0) {
    printf("MISMATCH name: %s != %s\n",  decoded.name, entity.name);
  }
  if (decoded.location.x != entity.location.x) {
    printf("MISMATCH location.x: %f != %f\n", decoded.location.x, entity.location.x);
  }
  if (decoded.location.y != entity.location.y) {
    printf("MISMATCH location.y: %f != %f\n", decoded.location.y, entity.location.y);
  }

  lightrpc_free_strings(&entity, entity_fields, 3);
  lightrpc_free_strings(&decoded, entity_fields, 3);

  return 0;
}
