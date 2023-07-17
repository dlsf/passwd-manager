#pragma once

#include "blockchain.h"

class EncryptBlockChain : public BlockChain {
    /*
    the EncryptBlockChain class represents a queue (vector) of EncryptBlocks
    it is used to encrypt data, its one type of BlockChain
    */
   public:
    EncryptBlockChain(const Hash* hash, const Bytes passwordhash, const Bytes enc_salt) : BlockChain(hash, passwordhash, enc_salt){};
   protected:
    bool addBlock() noexcept override;  // adds a new EncryptBlocks to the chain
};
