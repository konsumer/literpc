import { readFile } from 'node:fs/promises'
import { serialize, deserialize } from './literpc.js'

const encoder = new TextEncoder()
const decoder = new TextDecoder()

const {ops, ...defs} = JSON.parse(await readFile('tools/defs.example.json', 'utf8'))

test('encode', () => {
  const thing = {
    id: 123,
    location: {x: 1, y: 2},
    name: 'Test Entity'
  }

  const expected = new Uint8Array([
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
    105, 116, 121,
  ])

  const bytes = serialize(ops.indexOf('MESS_WITH_MY_THING'), defs, 'MyThing', thing)
  expect(bytes).toEqual(expected)
})

test('decode', () => {
  const thing = {
    id: 123,
    location: {x: 1, y: 2},
    name: 'Test Entity'
  }
  const bytes = new Uint8Array([
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
    105, 116, 121,
  ])
  const [op,r] = deserialize(defs, 'MyThing', bytes)
  expect(op).toEqual(ops.indexOf('MESS_WITH_MY_THING'))
  expect(r).toEqual(thing)
})
