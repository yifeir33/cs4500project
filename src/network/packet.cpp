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
