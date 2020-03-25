#pragma once

#include <unordered_map>
#include <unordered_set>
#include <netinet/in.h>
#include <memory>

#include "util/object.h"

class DataFrame;

class KVStore : public Object {
public:
    class Key : public Object {
    private:
        std::string _name;
        size_t _node_idx;

    public:
        Key() = default;

        Key(const Key& other) = default;

        Key(const std::string& name, size_t idx);

        std::string get_name() const;

        size_t get_node() const;

        size_t hash() const override;

        bool equals(const Object *other) const override;

        std::shared_ptr<Object> clone() const override;

        bool operator==(const Key& k) const;
    };
    // implement hash function needed to use custom type as key
    struct KeyHasher {
        std::size_t operator()(const Key& k) const {
            return k.hash();
        }
    };

    KVStore();

    std::shared_ptr<DataFrame> get(const Key& k);

    std::shared_ptr<DataFrame> get_and_wait(const Key& k);

    void set(const Key& k, const std::shared_ptr<DataFrame>& df);

    std::shared_ptr<std::unordered_set<Key, KVStore::KeyHasher>> get_keys();

    size_t hash() const override;

    bool equals(const Object *other) const override;

    std::shared_ptr<Object> clone() const override;

private:
    std::unordered_map<Key, std::shared_ptr<DataFrame>, KeyHasher> _local_map;
    /**
     * not sure for the sockaddr part
     */
    /* std::unordered_map<sockaddr_in, std::unordered_set<Key>> _other_nodes; */
};

