#include "sha256.h"

Bytes sha256::hash(const Bytes bytes) const{
    unsigned char bytesin[bytes.getLen()];
    unsigned char bytesout[this->getHashSize()];
    auto inpbytes = bytes.getBytes();
    std::copy(inpbytes.begin(), inpbytes.end(), bytesin);
    SHA256(bytesin, bytes.getLen(), bytesout);
    std::vector<unsigned char> v(bytesout, bytesout + sizeof(bytesout) / sizeof(bytesout[0]));
    Bytes ret;
    ret.setBytes(v);
    return ret;
}

Bytes sha256::hash(const std::string str) const{
    unsigned char bytesin[str.length()];
    std::copy( str.begin(), str.end(), bytesin );
    unsigned char bytesout[this->getHashSize()];
    SHA256(bytesin, str.length(), bytesout);
    std::vector<unsigned char> v(bytesout, bytesout + sizeof(bytesout) / sizeof(bytesout[0]));
    Bytes ret;
    ret.setBytes(v);
    return ret;
}
