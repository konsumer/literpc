import sys
from pathlib import Path
sys.path.append(str(Path(__file__).absolute().parent.parent))

# pip install simple-hexdump
from hexdump import hexdump

import literpc
import json

with open('tools/defs.example.json') as data:
  defs = json.load(data)
  try:
    ops = defs['ops']
    del defs['ops']
  except:
    ops = []

expected = bytearray([
  1,0,               # op = 1

  4,0,               # id:len = 4
  123,0,0,0,         # id:i32 = 123

  14,0,              # location:len = 14
  4,0,               # location.x:len = 4
  0,0,128,63,        # location.x:f32 = 1.0
  4,0,               # location.y:len = 4
  0,0,0,64,          # location.y:f32 = 2.0

  11,0,              # name:len = 11

  84, 101, 115, 116, # name:bytes
  32,  69, 110, 116,
  105, 116, 121,
])

thing = {
  'id': 123,
  'location': {'x': 1, 'y': 2},
  'name': 'Test Entity'
}

b = literpc.serialize(ops.index('MESS_WITH_MY_THING'), defs, 'MyThing', thing)

print('Expected:\n')
print(hexdump(expected))

print('\nReceived:\n')
print(hexdump(b))

o = literpc.deserialize(defs, 'MyThing', expected)
print(o)

