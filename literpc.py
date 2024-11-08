import struct

viewTypes = ['Int64', 'Uint64', 'Float64', 'Int32', 'Uint32', 'Float32', 'Int16', 'Uint16', 'Int8', 'Uint8', 'BigInt64', 'BigUint64']
sizes = {
  'Float32': '<f',
  'Float64': '<d',
  'Int8': '<b',
  'Int16': '<h',
  'Int32': '<i',
  'Int64': '<q',
  'Uint8': '<B',
  'Uint16': '<H',
  'Uint32': '<I',
  'Uint64': '<Q'
}

def serialize(op, defs, name, thing, noCommand=False):
  out = bytearray()
  if not noCommand:
    out += struct.pack(sizes['Uint16'], op)
  for fieldName in defs[name].keys():
    if fieldName not in thing:
      out += struct.pack(sizes['Uint16'], 0)
    else:
      fieldType = defs[name][fieldName]
      if fieldType in viewTypes:
        v = struct.pack(sizes[fieldType], thing[fieldName])
        out += struct.pack(sizes['Uint16'], len(v))
        out += v
      elif fieldType == 'String':
        v = bytes(thing[fieldName], 'utf8')
        out += struct.pack(sizes['Uint16'], len(v))
        out += v
      else:
        v = serialize(0, defs, fieldType, thing[fieldName], True)
        out += struct.pack(sizes['Uint16'], len(v))
        out += v
  return out

def deserialize(defs, name, bytes):
  pass