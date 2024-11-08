import { readFile } from 'node:fs/promises'

const fieldTypes = {
  'Int64': 'FIELD_INT64',
  'Uint64': 'FIELD_UINT64',
  'Float64': 'FIELD_FLOAT64',
  'Int32': 'FIELD_INT',
  'Uint32': 'FIELD_UINT',
  'Float32': 'FIELD_FLOAT',
  'Int16': 'FIELD_INT16',
  'Uint16': 'FIELD_UINT16',
  'Int8': 'FIELD_INT8',
  'Uint8': 'FIELD_UINT8',
  'String': 'FIELD_STRING'
}
const viewTypes = Object.keys(fieldTypes)

const cTypes = {
  'Int64': 'int64_t',
  'Uint64': 'uint64_t',
  'Float64': 'double',
  'Int32': 'int32_t',
  'Uint32': 'uint32_t',
  'Float32': 'float',
  'Int16': 'int16_t',
  'Uint16': 'uint16_t',
  'Int8': 'int8_t',
  'Uint8': 'uint8_t',
  'String': 'char*'
}

function exportDefs(defs = {}, ops=[]) {
  const out = [`// Example usage of lightrpc
// Compile with gcc FILE.c -o PROGRAM

#include <stdio.h>
#include <stdint.h>
#include "lightrpc.h"

// these are the available RPC commands
typedef enum {
  ${ops.map(o => `RPC_${o}`).join(',\n  ')}
} RpcCommands;
`]
  for (const [typeName, fields] of Object.entries(defs)) {
    out.push(`#define ${typeName.toLowerCase()}_fields_len ${Object.keys(fields).length}`)
    out.push(`typedef struct {
  ${Object.entries(fields).map(([fieldName, fieldType]) =>`${cTypes[fieldType] || fieldType} ${fieldName};`).join('\n  ')}
} ${typeName};
`)
    out.push(`const FieldDescriptor ${typeName.toLowerCase()}_fields[] = {
  ${Object.entries(fields).map(([fieldName, fieldType]) => viewTypes.includes(fieldType) ? `LIGHTRPC_FIELD(${typeName}, ${fieldName}, ${fieldTypes[fieldType]})` : `LIGHTRPC_FIELD_STRUCT(${typeName}, ${fieldName}, ${typeName.toLowerCase()}_fields, ${typeName.toLowerCase()}_fields_len)`).join(',\n  ')}
};
`)
  }
  out.push(`int main(int argc, char *argv[]) {
  uint8_t buffer[1024];
  int len = 0;
  int cmd = 0;

  ${Object.entries(defs).map(([typeName, fields]) => `${typeName} ${typeName.toLowerCase()} = {0}; // Create your ${typeName} struct here\n  len = lightrpc_serialize(buffer, &${typeName.toLowerCase()}, ${ops[0] ? `RPC_${ops[0]}` : 0}, ${typeName.toLowerCase()}_fields, ${typeName.toLowerCase()}_fields_len);\n\n  ${typeName} decoded${typeName} = {0};\n  cmd = lightrpc_deserialize(buffer, len, &decoded${typeName}, ${typeName.toLowerCase()}_fields, ${typeName.toLowerCase()}_fields_len);`).join('\n\n  ')}
}`)
  return out.join('\n')
}

const [,NAME, DEFS] = process.argv
if (!DEFS) {
  console.error(`Usage: ${NAME} <JSON_DEF_FILE>`)
  process.exit(1)
}

const {ops, ...defs} = JSON.parse(await readFile(DEFS, 'utf8'))
console.log(exportDefs(defs, ops))
