#include "lightrpc.h"

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
    FIELD_DESC(Entity, name, FIELD_VAR_STRING)
};

int main() {
  uint8_t buffer[1024];

      Entity entity = {
          .id = 123,
          .location = { .x = 1.0f, .y = 2.0f },
          .name = strdup("Test Entity")
      };

      int len = lightrpc_serialize(buffer, &entity, 1, entity_fields, 3);

      Entity decoded = {0};
      lightrpc_deserialize(buffer, len, &decoded, entity_fields, 3);

      // Use decoded entity...
      printf("%s\n%fx%f\n", decoded.name, decoded.location.x, decoded.location.y);

      lightrpc_free_strings(&entity, entity_fields, 3);
      lightrpc_free_strings(&decoded, entity_fields, 3);

      return 0;
}
