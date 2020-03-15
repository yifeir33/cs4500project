#include <cstring>

#include "packet.h"

Packet::Packet() : type(0), length(0) {
    memset(value, 0, DATA_MAX);
}

size_t Packet::get_size() const {
    return sizeof(type) + sizeof(length) + length;
}

int Packet::pack(uint8_t *buffer, size_t buflen) const {
    if(this->get_size() > buflen) return -1;
    size_t pos = 0;

    // type
    memcpy(buffer + pos, &type, sizeof(type));
    pos += sizeof(type);

    // length
    memcpy(buffer + pos, &length, sizeof(length));
    pos += sizeof(length);

    // value
    memcpy(buffer + pos, &value, length);
    pos += length;

    return pos;
}

size_t Packet::unpack(uint8_t *buffer, size_t buflen){
    /* p("unpack, buflen: ").p(buflen).p('\n'); */
    size_t pos = 0;

    // unpack type
    if(pos + sizeof(type) >= buflen){
        p("Too short for type!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        p(sizeof(type)).p('\n');
        goto TOO_SHORT;
    }
    memcpy(&this->type, buffer + pos, sizeof(type));
    pos += sizeof(type);

    // unpack length
    if(pos + sizeof(length) >= buflen){
        p("Too short for length!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        goto TOO_SHORT;
    }
    memcpy(&this->length, buffer + pos, sizeof(length));
    pos += sizeof(length);
    
    // unpack value
    if(pos + length >= buflen){
        p("Too short for value!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        goto TOO_SHORT;
    }
    memcpy(&this->value, buffer + pos, length);
    pos += length;

    return pos;

    TOO_SHORT:
        p("Failed To Unpack!");
        this->type = 0;
        this->length = 0;
        memset(&this->value, 0, DATA_MAX);
        return -1;
}

bool Packet::equals(const Object* other) const {
    auto opacket = dynamic_cast<const Packet*>(other);
    if(opacket) {
        if(type == opacket->type && length == opacket->length) {
            for(int i = 0; i < length; ++i) {
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
    hash += length;
    for(size_t i = 0; i < length; ++i){
        hash += i * value[i];
    }
    return hash;
}

Object *Packet::clone() const {
    auto new_packet = new Packet();
    new_packet->type = type;
    new_packet->length = length;
    memcpy(new_packet->value, value, length);
    return new_packet;
}
