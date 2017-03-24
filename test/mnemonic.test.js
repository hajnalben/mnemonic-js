const { expect } = require('chai')
const { hash } = require('../src/index')

describe('mnemonic#hash', function () {
  const array = new Uint8Array([55, 62, 63, 74, 65])

  it('should return the readable hash of an Uint8Array', function () {
    const result = hash(array)

    expect(result).to.be.a('string')
  })

  it('should return the same hash for the same array', function () {
    const hash1 = hash(array)
    const hash2 = hash(array)

    expect(hash1).to.equal(hash2)
  })

  it('should return different hash for different arrays', function () {
    const hash1 = hash(array)
    const hash2 = hash(new Uint8Array([83, 43, 12, 43, 64, 32, 65]))

    expect(hash1).to.not.equal(hash2)
  })

  it('should return only 5 words for any size of input array', function () {
    const buff = []

    for (let i = 40; i < 100; i++) {
      buff.push(i)
    }

    const result = hash(new Uint8Array(buff))
    expect(result).to.match(/^([a-z]+\s){4}[a-z]+$/)
  })

  it('should accept the format parameter', function () {
    const result = hash(array, 'x x x|')

    expect(result).to.match(/^([a-z]+\s){2}[a-z]+$/)
  })
})
