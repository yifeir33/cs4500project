#pragma once

#include <memory>
#include <unordered_map>

#include "util/serializable.h"
#include "data/kvstore.h"
#include "data/dataframe.h"
#include "data/rower.h"

#define MIN_CHUNK_ROWS 5000

class DistributedStore {
public:
    static DistributedStore& get_instance();

    struct MetaData : public Serializable {
        std::string local_key;
        std::pair<size_t, size_t> row_range;
        sockaddr_in node;

        MetaData(std::string k, size_t lower, size_t upper, sockaddr_in n);

        std::vector<uint8_t> serialize() const override;

        size_t hash() const override;

        bool equals(const Object* other) const override;

        std::shared_ptr<Object> clone() const override;
    };


    void insert(std::string key, std::shared_ptr<DataFrame> df);

    void insert(std::string key, std::vector<DistributedStore::MetaData> md,
                std::vector<std::string> keys, std::vector<std::shared_ptr<DataFrame>> dfs);

    std::vector<uint8_t> local_map(std::string key, size_t opcode);

    std::vector<std::vector<uint8_t>> distributed_map(std::string key, size_t opcode);

private:
    std::unordered_map<std::string, std::vector<DistributedStore::MetaData>> _keymap;
    KVStore _store;

    static DistributedStore ds;

    DistributedStore() = default;
    // singleton deletes
    DistributedStore(const DistributedStore&) = delete;
    void operator=(const DistributedStore&) = delete;
    DistributedStore(DistributedStore&&) = delete;

    class ChunkFilterRower : public Rower {
    private:
        size_t _lower;
        size_t _upper;

    public:
        ChunkFilterRower(size_t lower, size_t upper);

        void set_bounds(size_t l, size_t u);

        bool accept(Row& r) override;

        void join(std::shared_ptr<Rower> other) override;

        size_t hash() const override;

        bool equals(const Object *other) const override;

        std::shared_ptr<Object> clone() const override;
    };

    std::vector<std::shared_ptr<DataFrame>> chunk_dataframe(std::shared_ptr<DataFrame> df) const;

    void _insert_chunks(std::string key, std::vector<std::shared_ptr<DataFrame>> chunks);
};

template<>
inline DistributedStore::MetaData Serializable::deserialize<DistributedStore::MetaData>(const std::vector<uint8_t>& data, size_t& pos){
    std::string lkey = Serializable::deserialize<std::string>(data, pos);

    size_t lower = Serializable::deserialize<size_t>(data, pos);

    size_t upper = Serializable::deserialize<size_t>(data, pos);

    sockaddr_in node = Serializable::deserialize<sockaddr_in>(data, pos);

    return DistributedStore::MetaData(lkey, lower, upper, node);
}
