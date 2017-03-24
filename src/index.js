const mnemonic = require('bindings')('mnemonic')

module.exports = {
  hash (array, format = 'x x x x x|') {
    return mnemonic.hash(array, format)
  }
}