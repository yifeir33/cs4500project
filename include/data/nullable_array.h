#pragma once

#include <optional>
#include <vector>
#include <functional>

#include "util/serializable.h"

/** A template class representing an array with the possibility of having missing
 * values. To acheive this is uses a bitmap to represent whether the value exists
 * or not, and a vector to actually store the data. The downside of this approach
 * is that the two internal arrays have bad locality, resulting in more cache
 * misses. Another note is the bitmap (Vector<bool>) does not work with
 * other STL algorithms. */
template < class T >
class NullableArray : public Serializable {
private:
    /** Store boolean values representing whether the value at that index exists
     * if true in a space efficient manner. */
    std::vector<bool> _bitmap;
    /** Store the actual data. Missing values are simply default initialized,
     * using them as non-missing values is undefined behavior. */
    std::vector<T> _data;

    /** Private constructor used for serialization to simplify the logic. */
    NullableArray(std::vector<bool> bitmap, std::vector<T> data){
        assert(bitmap.size() == data.size());
        _bitmap = bitmap;
        _data = data;
    }

public:
    NullableArray() = default;
    NullableArray(const NullableArray<T>&) = default;
    NullableArray(NullableArray<T>&&) = default;

    /** 
     * If the optional exists, puts the value on the back of the data
     * and marks it as existing in the bitmap. Otherwise, default constructs a value
     * on the back of the data and marks it as non-existient in the bitmap.
     */
    inline void push_back(std::optional<T> val){
        if(val){
            _data.push_back(*val);
            _bitmap.push_back(true);
        } else {
            _data.emplace_back();
            _bitmap.push_back(false);
        }
    }

    /** Returns the value at the given index if it exists as a non-null optional.
     * Otherwise returns a null optional. An invalid index is undefined behavior */
    inline std::optional<T> get(size_t pos) const {
        assert(pos < _data.size());
        if(_bitmap[pos]){
            return std::optional<T>(_data[pos]);
        }
        return std::nullopt;
    }

    /** Sets the value at the given index to the given value if it exists, and
     * records it as existing. Otherwise it sets it to a default constructed value,
     * and sets is as missing. */
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

    /** Removes the given value in the list the list, and returns it.
     * If it exists a non-null optional is returned, otherwise a null optional
     * is returned. */
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

    /** Returns the total number of elements in the array, including missing
     * values. */
    inline size_t size() const {
        return _data.size();
    }

    /** Tests for equality */
    inline bool equals(const Object *other) const override {
        auto ona = dynamic_cast<const NullableArray<T> *>(other);
        if(ona) {
            return _bitmap == ona->_bitmap && _data == ona->_data;
        }
        return false;
    }

    /** Overload of the equality operator. Tests for equality. */
    inline bool operator==(const NullableArray<T>& other) const {
        return _bitmap == other._bitmap && _data == other._data;
    }

    /** Returns the hashcode of the array */
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

    /** Returns a copy constructed instance of this object */
    inline std::shared_ptr<Object> clone() const override {
        return std::make_shared<NullableArray<T>>(*this);
    }

    /** Serializes the data into byte form */
    inline std::vector<uint8_t> serialize() const override {
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

    /** Implementation of its own deserialize method. Due to C++ template rules,
     * we cannot provide a specialization of Serializable::deserialize() that is
     * generic over the inner type T for a NullableArray. Therefore we implement our
     * own deserialize method. This is not ideal, but is the best solution we found. */
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

