const { expect } = require('chai')
const mnemonic = require('../src/index')

describe('mnemonic#hash', function () {
  const array = new Uint8Array([55, 62, 63, 74, 65])

  it('should return the readable hash of an Uint8Array', function () {
    const hash = mnemonic.hash(array)

    expect(hash).to.be.a('string');
  })

  it('should return the same hash for the same array', function () {
    const hash1 = mnemonic.hash(array)
    const hash2 = mnemonic.hash(array)

    expect(hash1).to.equal(hash2);
  })

  it('should return different hash for different arrays', function () {
    const hash1 = mnemonic.hash(array)
    const hash2 = mnemonic.hash(new Uint8Array([83, 43, 12, 43, 64, 32, 65]))

    expect(hash1).to.not.equal(hash2);
  })

})