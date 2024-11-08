import struct

vt = {
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
    out += struct.pack(vt['Uint16'], op)
  for fieldName in defs[name].keys():
    if fieldName not in thing:
      out += struct.pack(vt['Uint16'], 0)
    else:
      fieldType = defs[name][fieldName]
      if fieldType in vt.keys():
        v = struct.pack(vt[fieldType], thing[fieldName])
        out += struct.pack(vt['Uint16'], len(v))
        out += v
      elif fieldType == 'String':
        v = bytes(thing[fieldName], 'utf8')
        out += struct.pack(vt['Uint16'], len(v))
        out += v
      else:
        v = serialize(0, defs, fieldType, thing[fieldName], True)
        out += struct.pack(vt['Uint16'], len(v))
        out += v
  return out

def deserialize(defs, name, bytes_data):
    out = {}
    offset = 0
    cmd = struct.unpack_from(vt['Uint16'], bytes_data, offset)[0]
    offset += 2

    for fieldName, fieldType in defs[name].items():
        length = struct.unpack_from(vt['Uint16'], bytes_data, offset)[0]
        offset += 2

        if length == 0:
            continue

        if fieldType in vt.keys():
            out[fieldName] = struct.unpack_from(vt[fieldType], bytes_data, offset)[0]
            offset += length
        elif fieldType == 'String':
            out[fieldName] = bytes_data[offset:offset+length].decode('utf-8')
            offset += length
        else:
            # For nested structures, we need to process from offset-2 to include the length
            nested_data = bytes_data[offset-2:offset-2+length]
            nested_result = deserialize(defs, fieldType, nested_data)
            out[fieldName] = nested_result[1]
            offset += length - 2  # Subtract 2 because we already read the length

    return [cmd, out]
