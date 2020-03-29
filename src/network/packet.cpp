#include <cstring>

#include "network/packet.h"

Packet::Packet() : remaining_len(0), type(Type::NONE), value() {}

size_t Packet::get_size() const {
    return sizeof(type) + sizeof(value.size()) + value.size();
}

std::vector<uint8_t> Packet::pack() const {
    std::vector<uint8_t> packed(this->value); // copy construct

    // prepend length of data
    size_t len = this->value.size();
    uint8_t *f_ptr = reinterpret_cast<uint8_t *>(&len);
    packed.insert(packed.begin(), f_ptr, f_ptr + sizeof(len));

    // prepend type of data
    packed.insert(packed.begin(), this->type);

    return packed;
}

int Packet::unpack(uint8_t *buffer, size_t buflen){
    /* p("unpack, buflen: ").p(buflen).p('\n'); */
    size_t pos = 0;
    this->type = Type::NONE;
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
    if(pos + sizeof(remaining_len) >= buflen){
        p("Too short for length!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        this->type = Type::NONE;
        return -1;
    }
    memcpy(&remaining_len, buffer + pos, sizeof(remaining_len));
    pos += sizeof(remaining_len);
    
    // unpack value
    if(pos + remaining_len >= buflen){
        p("Too short for value!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        this->type = Type::NONE;
        return -1;
    }
    this->value.insert(this->value.end(), buffer + pos, buffer + pos + remaining_len);
    /* this->value.resize(len); */
    /* memcpy(this->value.data(), buffer + pos, len); */
    pos += remaining_len;
    remaining_len = 0;

    return pos;
}

size_t Packet::partial_unpack(bool front, uint8_t *buffer, size_t buflen, bool& finished){
    /* p("unpack, buflen: ").p(buflen).p('\n'); */
    size_t pos = 0;

    if(front) {
        this->type = Type::NONE;
        this->value.clear();
        remaining_len = 0;
        // unpack type
        if(pos + sizeof(type) > buflen){
            p("Too short for type!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
            p(sizeof(type)).p('\n');
            return pos;
        } else {
            memcpy(&this->type, buffer + pos, sizeof(this->type));
            pos += sizeof(this->type);
        }

        // unpack length
        if(pos + sizeof(remaining_len) > buflen){
            p("Too short for length!\n").p("Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
            this->type = Type::NONE;
            return pos;
        } else {
            memcpy(&remaining_len, buffer + pos, sizeof(remaining_len));
            pos += sizeof(remaining_len);
        }
    }
    
    // unpack value
    if(pos + remaining_len > buflen){
        p("Too short for value!\n").p("Remaining Len: ").p(remaining_len).p(" Pos: ").p(pos).p(" BufLen: ").p(buflen).p('\n');
        // read in as much as we can
        size_t to_read = buflen - pos;
        p("To Read: ").pln(to_read);
        this->value.insert(this->value.end(), buffer + pos, buffer + pos + to_read);
        pln("Read Complete");
        pos += to_read;
        remaining_len -= to_read;
        p("Remaining: ").pln(remaining_len);
    } else {
        this->value.insert(this->value.end(), buffer + pos, buffer + pos + remaining_len);
        pos += remaining_len;
        remaining_len = 0;
    }
    /* this->value.resize(len); */
    /* memcpy(this->value.data(), buffer + pos, len); */

    finished = (remaining_len == 0);
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
