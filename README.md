This is a very lightweight RPC protocol I designed to be easy to parse in any language, for sending commands over wasm.

The basic idea is that the firs byte is a "command" (0-255) and the rest is encoded bytes, and both sides need to know the structure.

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
    FIELD_DESC(Entity, name, FIELD_VAR_STRING)
};
```
