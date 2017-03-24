# mnemonic-js
Hash To Readable Words

# Installation
Just install it with the help of npm.

    npm install git+https://git@github.com/hajnalben/mnemonic-js.git --save

# Usage
The only public function of the package is the 'hash'. It accepts an Uint8Array with a format (defaults to 'x x x x x|') and returns words from it. Allways produces the same result for the same input and format.

``` js
const { hash } = require('mnemonic-js')

const array = new Uint8Array([
    106, 10, 114, 140, 223, 171, 16, 2, 201, 233, 88, 34, 217, 53, 145, 110,
    77, 42, 4, 246, 227, 107, 69, 92, 102, 225, 177, 123, 194, 129, 34, 255
  ])

const result = hash(array) // result => 'channel happy fiber sponsor visual'
const result2 = hash(array, 'x x x|') // result2 => 'channel happy fiber'
```
