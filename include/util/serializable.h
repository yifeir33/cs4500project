#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <exception>
#include <string>
#include <cstring>
#include <cassert>
#include <netinet/ip.h>

#include "util/object.h"

class Serializable : public Object {
public:
    class ShortSerializedDataException : public std::exception {
        const char *what() const throw() override {
            return "Given data is too short to deserialize!";
        }
    };

    template< typename T > 
        static T deserialize(const std::vector<uint8_t>& data, size_t& pos) {
            static_assert(std::is_fundamental_v<T>, "Must specialize this function for base class!");

            if(pos > data.size() || data.size() - pos < sizeof(T)) throw ShortSerializedDataException();

            T val;
            memcpy(&val, data.data() + pos, sizeof(T));
            pos += sizeof(T);
            return val;
        }
    
    template < typename T >
        static std::vector<uint8_t> serialize(T t) {
            static_assert(std::is_fundamental_v<T>, "Must be a primitive type to serialize!");
            std::vector<uint8_t> vec;
            uint8_t *fake_ptr = reinterpret_cast<uint8_t *>(&t);
            vec.insert(vec.end(), fake_ptr, fake_ptr + sizeof(T));
            return vec;
        }

    virtual std::vector<uint8_t> serialize() const = 0;

    /* static std::vector<uint8_t> serialize_string(const std::string& s) { */
    /*     std::vector<uint8_t> vec = Serializable::serialize<size_t>(s.size()); */
    /*     for(size_t i = 0; i < s.size(); ++i){ */
    /*         std::vector<uint8_t> temp = Serializable::serialize<char>(s[i]); */
    /*         vec.insert(vec.end(), temp.begin(), temp.end()); */
    /*     } */
    /*     return vec; */
    /* } */
};

// specialize serialize/deserialze for std::string
template<>
inline std::vector<uint8_t> Serializable::serialize<std::string>(std::string s){
    std::vector<uint8_t> vec = Serializable::serialize<size_t>(s.size());
    for(size_t i = 0; i < s.size(); ++i){
        std::vector<uint8_t> temp = Serializable::serialize<char>(s[i]);
        vec.insert(vec.end(), temp.begin(), temp.end());
    }
    return vec;
}

template<>
inline std::string Serializable::deserialize<std::string>(const std::vector<uint8_t>& data, size_t& pos) {
    std::string s;
    size_t len = Serializable::deserialize<size_t>(data, pos);
    for(size_t i = 0; i < len; ++i) {
        s += Serializable::deserialize<char>(data, pos);
    }
    return s;
}

// specialize serialize/deserialize for std::vector<bool> (bitmap)
template<>
inline std::vector<uint8_t> Serializable::serialize<std::vector<bool>>(std::vector<bool> vb){
    std::vector<uint8_t> vec = Serializable::serialize<size_t>(vb.size());

    uint8_t bm_byte = 0;
    size_t pos = 0;
    for(size_t i = 0; i < vb.size(); ++i){
        if(pos == 8){
            vec.push_back(bm_byte);
            bm_byte = vb[i] & 1;
            pos = 1;
        } else {
            bm_byte |= (vb[i] & 1) << (pos++);
        }
    }
    if(pos > 0) vec.push_back(bm_byte);

    return vec;
}

template<>
inline std::vector<bool> Serializable::deserialize<std::vector<bool>>(const std::vector<uint8_t>& data, size_t& pos) {
    std::vector<bool> vec;

    size_t len = Serializable::deserialize<size_t>(data, pos);
    size_t byte_cnt = len / 8 + (len % 8 > 0 ? 1 : 0);
    size_t byte_bound = byte_cnt + pos;
     
    size_t inner_pos = 0;
    size_t data_pos = 0;
    while(pos < byte_bound && data_pos < len) {
        assert(pos < data.size());
        vec.push_back(data[pos] & (1 << inner_pos++));
        if(inner_pos == 8){
            inner_pos = 0;
            ++pos;
        }
        ++data_pos;
    }
    if(inner_pos != 0) ++pos;
    return vec;
}

// specialize serialize/deserialize for sockaddr_in
template<>
inline std::vector<uint8_t> Serializable::serialize<sockaddr_in>(sockaddr_in addr){
    std::vector<uint8_t> serialized;
    auto f_ptr = reinterpret_cast<uint8_t *>(&addr);
    serialized.insert(serialized.end(),
                      f_ptr,
                      f_ptr + sizeof(addr));

    return serialized;
}

template<>
inline sockaddr_in Serializable::deserialize<sockaddr_in>(const std::vector<uint8_t>& data, size_t& pos) {
    sockaddr_in addr;
    memcpy(&addr, data.data() + pos, sizeof(sockaddr_in));
    pos += sizeof(sockaddr);
    return addr;
}
