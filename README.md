This is a very lightweight RPC protocol I designed to be easy to parse in any language, for sending commands over wasm.

The basic idea is that the first byte is a "command" (0-255) and the rest is encoded bytes, and both sides need to know the structure. Think of it as very low-end grpc/protobuf. It's optimized for encoding/decoding-speed & simplicity, and although not as small as possible, the size beats other serialization-formats.

This allows you to expose a single function in your wasm-host (or whatever else) that can respond to RPC-requests, and get/set complex params/responses.

Here is how you compile the C test:

```
npm run test:c
```

## schema

I have a json format that can be used to generate bindings for different languages. It looks like this:

```json
{
  "Point": {
    "x": "Float32",
    "y": "Float32"
  },
  "MyThing": {
    "id": "Int32",
    "location": "Point",
    "name": "String"
  },
  "ops": [
    "NONE",
    "MESS_WITH_MY_THING"
  ]
}
```

Here are the valid-types:
- `Int64`
- `Uint64`
- 'Float64'
- `Int32`
- `Uint32`
- `Float32`
- `Int16`
- `Uint16`
- `Int8`
- `Uint8`
- `String`
- `SomethingElse` - use another message in the definition (like in above example, with `Point`)

Additionally, you can also append `[]` for an array, like `Uint8[]`.

The bytes look like this:

```js
[COMMAND, LENGTH, BYTES..., LENGTH, BYTES..., etc]
```

> [!NOTE]
> This requires lengths & counts to be `<65,536`, since the are `Uint16` type. If you need more than that, break your message up into smaller pieces.

- You can skip an unused field by setting size to 0.
- All numeric values are encoded little-endian (which matches WASM and most modern things.)

## using in other languages

You can do this in javascript:

```js
import { serialize, deserialize } from 'lightrpc'
import { readFile } from 'node:fs/promises'

const {ops, ...defs} = JSON.parse(await readFile('tools/defs.example.json', 'utf8'))

const thing = {
  id: 123,
  location: {x: 1, y: 2 },
  name: 'Test Entity'
}

const bytes = serialize(ops.indexOf('MESS_WITH_MY_THING'), defs, 'MyThing', thing)
const [command, decoded] = deserialize(defs, 'MyThing', bytes)
```
