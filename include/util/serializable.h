#pragma once

#include <vector>
#include <cstdint>
#include <type_traits>
#include <exception>
#include <string>
#include <cstring>

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

            if(pos > data.size() || data.size() - pos < sizeof(T) - 1) throw ShortSerializedDataException();

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
