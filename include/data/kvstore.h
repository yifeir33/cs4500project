#pragma once

#include <unordered_map>
#include <unordered_set>
#include <netinet/in.h>
#include <memory>
#include <cassert>
#include <shared_mutex>

#include "util/object.h"
#include "util/serializable.h"

// forward declaration to avoid circular dependancy
class DataFrame;

/** A class representing a distributed mapping of keys to values. Stores Keys-Value pairs
 * locally, but also knows what keys are available on remote nodes, and can request and
 * access those remote values over the network.
 *
 * Uses the singleton pattern.
 */
class KVStore : public Object {
public:
    /** The Key class for the KVStore. Stores the string key, which is the actual
     * key, and the node is stored as metadata. As a result, the node is used for
     * neither hashing nor equality, to allow users to get values without know
     * the node on which it is stored. */
    class Key : public Serializable {
    private:
        /** The string key. */
        std::string _name;
        // note - this is not used in hashing or equality,
        // as one may not know which node the key is on,
        // so only the string is used as an identifier
        // it is mutable because it doesn't change the hash,
        // so can safetly be modified while in a set/map
        // mutable allows this to be modified in const methods
        mutable sockaddr_in _node;

        Key() = delete;

    public:
        /* Copy constructor. */
        Key(const Key& other) = default;

        /* Move constructor. */
        Key(Key&& other) = default;

        /* Constructs a key given both its name and node. */
        Key(const std::string& name, sockaddr_in node);

        /* Constructs a key given the name, but default initializing the node.
         * Since the node doesn't affect equality, this can be used to search
         * for a key in the map without knowing which node it is on. */
        Key(const std::string& name);

        /** Get the name of the key. */
        std::string get_name() const;

        /** Get the node of the key. */
        sockaddr_in get_node() const;

        /** Sets the node of the key. Due to the above described effects of the node
         * on equality and hashing, this method is const as the equality and hashcode
         * of the key remains unchanged, therefore it doesn't break the hashmap. */
        void set_node(const sockaddr_in& idx) const; 

        /** Return the hashcode of the key. */
        size_t hash() const override;

        /** Test for equality. */
        bool equals(const Object *other) const override;

        /** Constructs a new copy of the key. */
        std::shared_ptr<Object> clone() const override;

        /** Serializes the key into byte form. */
        std::vector<uint8_t> serialize() const override;

        /** Overload of the equality operator. Tests for equality. */
        bool operator==(const Key& k) const;
    };
    // implement hash function needed to use custom type as key
    struct KeyHasher {
        /** Specializes the hash operator for Keys. */
        std::size_t operator()(const Key& k) const {
            return k.hash();
        }
    };

    /** Static method to get the global singleton instance of the KVStore. */
    static KVStore& get_instance();

    /** Gets the dataframe associated with the given key only if it is stored 
     * on this node. If it cannot be found, returns nullptr. */
    std::shared_ptr<DataFrame> get_local(const Key& k);

    /** Gets the dataframe associated with the given key regardless of which node
     * it is on. If it cannot be found, returns nullptr. */
    std::shared_ptr<DataFrame> get(const Key& k);

    /** Gets the dataframe associated with the given key regardless of which node
     * it is on. If it cannot be found, this continues to try until it reaches
     * the given timeout. If the given timeout is 0, this method will wait indefinetly.
     * WARNING: This currently busy blocks the current thread. */
    std::shared_ptr<DataFrame> get_or_wait(const Key& k, time_t timeout_ms = 0);

    /** Stores the given value locally. If the key exists on another node, 
     * behavior is undefined. */
    void set(const Key& k, const std::shared_ptr<DataFrame>& df);

    /** Adds a key to the KVStore as a non-local key. This key must have the node on which
     * it exists properly set, or undefined behavior will occur. */
    void add_nonlocal(Key k);

    /** Returns a set containing all local keys. */
    std::unordered_set<std::string> get_local_keys() const;

    /** Returns the hashcode of the kvstore */
    size_t hash() const override;

    /** Tests for equality. Due to the singleton pattern, this always returns true
     * if it is a KVStore. */
    bool equals(const Object *other) const override;

    /** Returns nullptr. This object cannot be cloned. */
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

    /** Mutex protecting the local unordered map. mutable so that 
     * it can be locked in const methods. */
    mutable std::mutex _local_map_mutex;
    /** The local unordered map storing key value pairs. */
    std::unordered_map<Key, std::shared_ptr<DataFrame>, KeyHasher> _local_map;
    /** Mutex protecting the other node set. mutable so that
     * it can be locked in const methods. */
    mutable std::mutex _other_node_mutex;
    /** Set of keys found on other nodes. These keys must correctly store
     * which node they are on. */
    std::unordered_set<Key, KeyHasher> _other_nodes;
};


/** Specialization of deserialize for KVStore::Keys.
 *
 * Given the serialized version of the key, cosntructs a key object containing
 * the correct data corresponding with the key. */
template<>
inline KVStore::Key Serializable::deserialize<KVStore::Key>(const std::vector<uint8_t>& data, size_t& pos) {
    assert(data.size() - pos < sizeof(size_t));
    sockaddr_in node;
    memcpy(&node, data.data() + pos, sizeof(sockaddr_in));
    pos += sizeof(size_t);
    return KVStore::Key(Serializable::deserialize<std::string>(data, pos), node);
}
