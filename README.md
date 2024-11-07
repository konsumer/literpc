This is a very lightweight RPC protocol I designed to be easy to parse in any language, for sending commands over wasm.

The basic idea is that the firs byte is a "command" (0-255) and the rest is encoded bytes, and both sides need to know the structure. Think of it as very low-end grpc/protobuf.

This allows you to expose a single function in your wasm-host (or whatever else) that can respond to RPC-requests, and get complex params, as well as return complex responses.

Here is how you compile the test:

```
gcc main.c -o lightrpctest
```

Here is how you define a struct:
```c
typedef struct {
    float x;
    float y;
} Point;

// Field descriptors for Point
const FieldDescriptor point_fields[] = {
    FIELD_DESC(Point, x, FIELD_FLOAT),
    FIELD_DESC(Point, y, FIELD_FLOAT)
};

typedef struct {
    int id;
    Point location;
    char* name;
} Entity;

// Field descriptors for Entity
const FieldDescriptor entity_fields[] = {
    FIELD_DESC(Entity, id, FIELD_INT),
    STRUCT_FIELD_DESC(Entity, location, point_fields, 2),
    FIELD_DESC(Entity, name, FIELD_STRING)
};
const int entity_fields_count = 3;

// this defines the operations you can do in your RPC
typedef enum {
  OP_MESS_WITH_ENTITY
} Op;

```

Then encode/decode like this:

```c
// setup on both sides
uint8_t buffer[1024];

// create an entiry
Entity entity = {
  .id = 123,
  .location = { .x = 1.0f, .y = 2.0f },
  .name = strdup("Test Entity")
};

// call OP_MESS_WITH_ENTITY(entity) somewhere else (send it buffer, using len)
int len = lightrpc_serialize(buffer, &entity, OP_MESS_WITH_ENTITY, entity_fields, entity_fields_count);

// do this in your host
Entity decoded = {0};
lightrpc_deserialize(buffer, len, &decoded, entity_fields, 3);

// you can also get the op that was called
Op op = buffer[0];
```
