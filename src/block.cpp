/*
contains the implementation of the abstract Block class
*/
#include "block.h"

Block::Block(const int len, const Bytes salt) {
    if (len <= 0) {
        // invalid block length
        throw std::range_error("length of the block cannot be negative or zero");
    }
    if (salt.getLen() != len) {
        // block length was not fulfilled by the salt
        throw std::length_error("length of salt bytes does not match with the block length");
    }
    this->block_len = len;
    this->data = Bytes();
    this->salt = salt;
}

int Block::getFreeSpace() const noexcept {
    // returns the number of bytes that can be added to the block until it is completed
    return this->block_len - this->data.getLen();
}

Bytes Block::getResult() const noexcept {
    // returns the resulting data of the block
    return this->data;
}