#pragma once

#include <unordered_map>
#include <unordered_set>
#include <netinet/in.h>
#include <memory>
#include <cassert>
#include <shared_mutex>

#include "util/object.h"
#include "util/serializable.h"

class DataFrame;

// Uses singleton pattern
class KVStore : public Object {
public:
    class Key : public Serializable {
    private:
        std::string _name;
        // note - this is not used in hashing or equality,
        // as one may not know which node the key is on,
        // so only the string is used as an identifier
        size_t _node_idx;

    public:
        Key() = default;

        Key(const Key& other) = default;

        Key(Key&& other) = default;

        Key(const std::string& name, size_t idx);

        Key(const std::string& name);

        std::string get_name() const;

        size_t get_node() const;

        size_t hash() const override;

        bool equals(const Object *other) const override;

        std::shared_ptr<Object> clone() const override;

        std::vector<uint8_t> serialize() const override;

        bool operator==(const Key& k) const;
    };
    // implement hash function needed to use custom type as key
    struct KeyHasher {
        std::size_t operator()(const Key& k) const {
            return k.hash();
        }
    };

    static KVStore& get_instance();

    std::shared_ptr<DataFrame> get_local(const Key& k);

    std::shared_ptr<DataFrame> get(const Key& k);

    std::shared_ptr<DataFrame> get_or_wait(const Key& k, time_t timeout_ms = 0);

    void set(const Key& k, const std::shared_ptr<DataFrame>& df);

    std::shared_ptr<std::unordered_set<Key, KVStore::KeyHasher>> get_keys();

    size_t hash() const override;

    bool equals(const Object *other) const override;

    std::shared_ptr<Object> clone() const override;

private:
    // private constructor so it can only be constructed by itself to enforce singleton
    KVStore() = default;

    // Delete Copy/Move Semantics (for singleton)
    KVStore(const KVStore&) = delete;
    KVStore& operator=(const KVStore&) = delete;
    KVStore(KVStore&&) = delete;

    // store single instance of class
    static KVStore kv_instance;

    mutable std::shared_mutex _local_map_mutex;
    std::unordered_map<Key, std::shared_ptr<DataFrame>, KeyHasher> _local_map;
    mutable std::shared_mutex _other_node_mutex;
    std::unordered_set<Key, KeyHasher> _other_nodes;
};


// specialization of deserialize
template<>
inline KVStore::Key Serializable::deserialize<KVStore::Key>(const std::vector<uint8_t>& data, size_t& pos) {
    assert(data.size() - pos < sizeof(size_t));
    size_t nidx;
    memcpy(&nidx, data.data() + pos, sizeof(size_t));
    pos += sizeof(size_t);
    return KVStore::Key(Serializable::deserialize<std::string>(data, pos), nidx);
}
