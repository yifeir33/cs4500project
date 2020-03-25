#include <cstring>

#include "network/packet.h"

Packet::Packet() : type(0), value() {}

size_t Packet::get_size() const {
    return sizeof(type) + sizeof(value.size()) + value.size();
}

std::vector<uint8_t> Packet::pack() const {
    std::vector<uint8_t> packed(this->value); // copy construct
    uint8_t buf[2] = {};

    // prepend length of data
    uint16_t len = this->value.size();
    memcpy(buf, &len, sizeof(len));
    packed.insert(packed.begin(), buf, buf + sizeof(len));

    // prepend type of data
    memcpy(buf, &this->type, sizeof(this->type));
    packed.insert(packed.begin(), *buf);

    return packed;
}

size_t Packet::unpack(uint8_t *buffer, size_t buflen){
    /* p("unpack, buflen: ").p(buflen).p('\n'); */
    size_t pos = 0;
    this->type = 0;
    this->value.clear();

    // unpack type
    if(pos + sizeof(type) >= buflen){
        p("Too short for type!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        p(sizeof(type)).p('\n');
        return -1;
    }
    memcpy(&this->type, buffer + pos, sizeof(this->type));
    pos += sizeof(this->type);

    // unpack length
    if(pos + sizeof(size_t) >= buflen){
        p("Too short for length!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        this->type = 0;
        return -1;
    }
    uint16_t len = 0;
    memcpy(&len, buffer + pos, sizeof(len));
    pos += sizeof(len);
    
    // unpack value
    if(pos + len >= buflen){
        p("Too short for value!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        this->type = 0;
        return -1;
    }
    this->value.resize(len);
    memcpy(this->value.data(), buffer + pos, len);
    pos += len;

    return pos;
}

bool Packet::equals(const Object* other) const {
    auto opacket = dynamic_cast<const Packet*>(other);
    if(opacket) {
        if(type == opacket->type && value.size() == opacket->value.size()) {
            for(size_t i = 0; i < value.size(); ++i) {
                if(value[i] != opacket->value[i]){
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

size_t Packet::hash() const {
    size_t hash = this->type;
    hash += value.size();
    for(size_t i = 0; i < value.size(); ++i){
        hash += i * value[i];
    }
    return hash;
}

std::shared_ptr<Object> Packet::clone() const {
    // copy constructor
    return std::make_shared<Packet>(*this);
}
