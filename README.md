# mnemonic-js
Hash To Readable Words

# Installation
Just install it with the help of npm.

    npm install git+https://git@github.com/hajnalben/mnemonic-js.git --save

# Usage
The only public function of the package is the 'hash'. It accepts an Uint8Array and returns 5 words from it. Allways produces the same result for the same input.

``` js
const { hash } = require('mnemonic-js')

const array = new Uint8Array([55, 62, 63, 74, 65])

const result = hash(array)
```
