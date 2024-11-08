const encoder = new TextEncoder()
const decoder = new TextDecoder()

const viewTypes = ['Int64', 'Uint64', 'Float64', 'Int32', 'Uint32', 'Float32', 'Int16', 'Uint16', 'Int8', 'Uint8', 'BigInt64', 'BigUint64']
const sizes = viewTypes.reduce((a, c) => ({...a, [c]: parseInt(c.replace(/[a-zA-Z]+/, '')) / 8 }), {})

export function serialize(op, defs, name, thing) {
  const a = new ArrayBuffer(1024*1024)
  const v = new DataView(a)
  const b = new Uint8Array(a)
  v.setUint16(0, op, true)
  let o = 2
  for (let [fieldName, fieldType] of Object.entries(defs[name])) {
    if (typeof thing[fieldName] === 'undefined') {
      v.setUint16(o, 0, true)
      o += 2
      continue
    }
    if (fieldType === 'String') {
      const str = encoder.encode(thing[fieldName])
      v.setUint16(o, str.byteLength, true)
      b.set(str, o+2)
      o += 2 + str.byteLength
    } else if (sizes[fieldType]) {
      if (fieldType === 'Uint64') {
        fieldType = 'BigUint64'
      }
      if (fieldType === 'Int64') {
        fieldType = 'BigInt64'
      }
      v.setUint16(o, sizes[fieldType], true)
      v[`set${fieldType}`](o+2, thing[fieldName], true)
      o += 2 + sizes[fieldType]
    } else {
      const sub = serialize(0, defs, fieldType, thing[fieldName])
      b.set(sub, o)
      v.setUint16(o, sub.byteLength, true)
      o += sub.byteLength
    }
  }
  return b.slice(0, o)
}

export function deserialize(defs, name, bytes) {
  const v = new DataView(bytes.buffer)
  const out = {}
  let o = 2
  for (let [fieldName, fieldType] of Object.entries(defs[name])) {
    const len = v.getUint16(o, true)
    o += 2
    if (len == 0) {
      continue
    }
    if (fieldType === 'String') {
      out[fieldName] = decoder.decode(bytes.slice(o, o+len))
      o += len
    } else if (sizes[fieldType]) {
      if (fieldType === 'Uint64') {
        fieldType = 'BigUint64'
      }
      if (fieldType === 'Int64') {
        fieldType = 'BigInt64'
      }
      out[fieldName] = v[`get${fieldType}`](o, true)
      o += len
    } else {
      out[fieldName] = deserialize(defs, fieldType, bytes.slice(o-2, o-2+len))[1]
      o += len - 2
    }
  }

  return [v.getUint16(0, true), out]
}
