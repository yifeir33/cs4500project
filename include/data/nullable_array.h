#pragma once

#include <optional>
#include <vector>
#include <functional>

#include "util/serializable.h"

template < class T >
class NullableArray : public Serializable {
private:
    std::vector<bool> _bitmap;
    std::vector<T> _data;

    NullableArray(std::vector<bool> bitmap, std::vector<T> data){
        assert(bitmap.size() == data.size());
        _bitmap = bitmap;
        _data = data;
    }

public:
    NullableArray() = default;
    NullableArray(const NullableArray<T>&) = default;
    NullableArray(NullableArray<T>&&) = default;

    inline void push_back(std::optional<T> val){
        if(val){
            _data.push_back(*val);
            _bitmap.push_back(true);
        } else {
            _data.emplace_back();
            _bitmap.push_back(false);
        }
    }

    /* inline std::optional<const T&> operator[](size_t pos) const { */
    /*     assert(pos < _data.size()); */
    /*     if(_bitmap[pos]){ */
    /*         return std::optional<T>(_data[pos]); */
    /*     } */
    /*     return std::nullopt; */
    /* } */

    /* inline std::optional<T&> operator[](size_t pos) { */
    /*     assert(pos < _data.size()); */
    /*     if(_bitmap[pos]){ */
    /*         return std::optional<T>(_data[pos]); */
    /*     } */
    /*     return std::nullopt; */
    /* } */

    inline std::optional<T> get(size_t pos) const {
        assert(pos < _data.size());
        if(_bitmap[pos]){
            return std::optional<T>(_data[pos]);
        }
        return std::nullopt;
    }

    inline std::optional<T> set(size_t pos, std::optional<T> val){
        assert(pos < _data.size());
        std::optional<T> old = ((_bitmap[pos]) ? std::optional<T>(_data[pos]) : std::nullopt);
        if(val){
            _data[pos] = *val;
            _bitmap[pos] = true;
        } else {
            _data[pos] = {};
            _bitmap[pos] = false;
        }
        return old;
    }

    inline std::optional<T> pop(size_t pos){
        assert(pos < _data.size());
        std::optional<T> val = std::nullopt;
        if(_bitmap[pos]){
            val = std::optional<T>(_data[pos]);
        }
        _data.erase(_data.begin() + pos);
        _bitmap.erase(_bitmap.begin() + pos);
        return val;
    }

    inline size_t size() const {
        return _data.size();
    }

    inline bool equals(const Object *other) const override {
        auto ona = dynamic_cast<const NullableArray<T> *>(other);
        if(ona) {
            return _bitmap == ona->_bitmap && _data == ona->_data;
        }
        return false;
    }

    inline bool operator==(const NullableArray<T>& other) const {
        return _bitmap == other._bitmap && _data == other._data;
    }

    inline size_t hash() const override {
        size_t hash = _data.size();
        for(size_t i = 0; i < _data.size(); ++i){
            bool exists = _bitmap[i];
            hash += exists;
            if(exists){
                hash ^= std::hash<T>()(_data[i]) ^ i;
            }
        }
        return hash;
    }

    inline std::shared_ptr<Object> clone() const override {
        return std::make_shared<NullableArray<T>>(*this);
    }

    inline std::vector<uint8_t> serialize() const {
        // size - can we drop this?
        /* std::vector<uint8_t> serialized = Serializable::serialize<size_t>(_data.size()); */
        // bitmap
        std::vector<uint8_t> serialized = Serializable::serialize<std::vector<bool>>(_bitmap);
        // data
        std::vector<uint8_t> temp;
        if constexpr (std::is_same_v<bool, T>){
            temp = Serializable::serialize<std::vector<bool>>(_data);
            serialized.insert(serialized.end(), temp.begin(), temp.end());
        } else {
            for(size_t i = 0; i < _data.size(); ++i){
                if(_bitmap[i]){ // only serialize existing values
                    if constexpr (std::is_base_of_v<Serializable, T>){
                        temp = _data[i].serialize();
                    } else if constexpr (std::is_fundamental_v<T> 
                                            || std::is_same_v<std::string, T>){
                        temp = Serializable::serialize<T>(_data[i]);
                    }
                    serialized.insert(serialized.end(), temp.begin(), temp.end());
                }
            }
        }

        return serialized;
    }

    static inline NullableArray<T> deserialize(std::vector<uint8_t> data, size_t& pos){
        std::vector<bool> bitmap = Serializable::deserialize<std::vector<bool>>(data, pos);
        std::vector<T> arr_data;
        if constexpr (std::is_same_v<bool, T>){
            arr_data = Serializable::deserialize<std::vector<bool>>(data, pos);
        } else {
            for(size_t i = 0; i < bitmap.size(); ++i){
                if(bitmap[i]){
                    arr_data.push_back(Serializable::deserialize<T>(data, pos));
                } else {
                    arr_data.emplace_back();
                }
            }
        }
        return NullableArray(bitmap, arr_data);
    }
};

