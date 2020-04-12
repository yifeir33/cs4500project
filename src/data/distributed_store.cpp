#include "util/operation_generator.h"
#include "data/distributed_store.h"
#include "network/client.h"
#include "network/socket_util.h"

// static
DistributedStore DistributedStore::ds;

DistributedStore& DistributedStore::get_instance() {
    return ds;
}

// instance
DistributedStore::MetaData::MetaData(std::string k, size_t lower, size_t upper, sockaddr_in n) :
local_key(k), row_range(lower, upper), node(n) {}

std::vector<uint8_t> DistributedStore::MetaData::serialize() const {
    std::vector<uint8_t> serialized = Serializable::serialize<std::string>(this->local_key);

    std::vector<uint8_t> temp = Serializable::serialize<size_t>(row_range.first);
    serialized.insert(serialized.end(), temp.begin(), temp.end());

    temp = Serializable::serialize<size_t>(row_range.second);
    serialized.insert(serialized.end(), temp.begin(), temp.end());

    temp = Serializable::serialize<sockaddr_in>(node);
    serialized.insert(serialized.end(), temp.begin(), temp.end());

    return serialized;
}

size_t DistributedStore::MetaData::hash() const {
    size_t hash = local_key.size();

    hash += row_range.first ^ row_range.second;

    hash += node.sin_addr.s_addr ^ node.sin_port;

    return hash;
}

bool DistributedStore::MetaData::equals(const Object* other) const {
    auto om = dynamic_cast<const MetaData *>(other);
    if(om) {
        return this->local_key == om->local_key && this->row_range == om->row_range 
                && socket_util::sockaddr_eq(node, om->node);
    }
    return false;
}

std::shared_ptr<Object> DistributedStore::MetaData::clone() const {
    return std::make_shared<DistributedStore::MetaData>(local_key, row_range.first, row_range.second, node);
}

DistributedStore::ChunkFilterRower::ChunkFilterRower(size_t l, size_t u) : _lower(l), _upper(u) {
    assert(_upper > _lower);
}

void DistributedStore::ChunkFilterRower::set_bounds(size_t l, size_t u) {
    _lower = l;
    _upper = u;
    assert(_upper > _lower);
}

bool DistributedStore::ChunkFilterRower::accept(Row &r) {
    return _lower <= r.get_index() && r.get_index() < _upper;
}

// do nothing - this is for filtering
void DistributedStore::ChunkFilterRower::join([[maybe_unused]] std::shared_ptr<Rower> other) {}

size_t DistributedStore::ChunkFilterRower::hash() const {
    return _lower ^ _upper;
}

bool DistributedStore::ChunkFilterRower::equals(const Object *other) const {
    auto ocfr = dynamic_cast<const ChunkFilterRower *>(other);
    if(ocfr) {
        return this->_lower == ocfr->_lower && this->_upper == ocfr->_upper;
    }
    return false;
}

std::shared_ptr<Object> DistributedStore::ChunkFilterRower::clone() const {
    return std::make_shared<ChunkFilterRower>(_lower, _upper);
}

std::vector<std::shared_ptr<DataFrame>> DistributedStore::chunk_dataframe(std::shared_ptr<DataFrame> df) const {
    if(df->nrows() < MIN_CHUNK_ROWS) {
        return std::vector<std::shared_ptr<DataFrame>>({df});
    }
    std::vector<std::shared_ptr<DataFrame>> chunks;
    size_t chunk_cnt = df->nrows() / MIN_CHUNK_ROWS;
    size_t lower = 0;
    ChunkFilterRower cfr(lower, MIN_CHUNK_ROWS);
    for(size_t i = 0; i < chunk_cnt; ++i) {
        cfr.set_bounds(lower, i == chunk_cnt - 1 ? df->nrows() : lower + MIN_CHUNK_ROWS);
        chunks.push_back(df->filter(cfr));
        lower += MIN_CHUNK_ROWS;
    }
    return chunks;
}

void DistributedStore::_insert_chunks(std::string key, std::vector<std::shared_ptr<DataFrame>> chunks) {
    std::vector<MetaData> md;
    size_t idx = 0;
    size_t lower = 0;
    std::shared_ptr<Client> client_instance = Client::get_instance().lock();
    std::vector<sockaddr_in> clients = client_instance->get_full_client_list();

    // generate metadata
    for(auto chunk : chunks) {
        // generate local key
        std::string local_key(key);
        local_key += "_";
        local_key += idx;
        // get target node
        sockaddr_in node = clients[idx % clients.size()];
        // Store metadata
        md.emplace_back(local_key, lower, lower + chunk->nrows(), node);

        lower += chunk->nrows();
        idx += 1;
    }
    assert(md.size() == chunks.size());

    _keymap.insert_or_assign(key, md);

    // pass metadata + dataframe(s) to other node
    for(auto node : clients) {
        std::vector<std::string> local_keys;
        std::vector<std::shared_ptr<DataFrame>> local_dfs;
        for(size_t i = 0; i < md.size(); ++i) {
            if(socket_util::sockaddr_eq(md[i].node, node)) {
                local_keys.push_back(md[i].local_key);
                local_dfs.push_back(chunks[i]);
            }
        }
        if(socket_util::sockaddr_eq(client_instance->get_self(), node)) {
            this->insert(key, md, local_keys, local_dfs);
        } else {
            client_instance->push_remote(node, key, md, local_keys, local_dfs);
        }
    }
}


void DistributedStore::insert(std::string key, std::shared_ptr<DataFrame> df) {
    std::vector<std::shared_ptr<DataFrame>> chunks = this->chunk_dataframe(df);
    this->_insert_chunks(key, chunks);
}

void DistributedStore::insert(std::string key, std::vector<DistributedStore::MetaData> md,
                              std::vector<std::string> keys, std::vector<std::shared_ptr<DataFrame>> dfs) {
    _keymap.insert_or_assign(key, md);
    // extract local keys for metadata
    assert(keys.size() == dfs.size());
    for(size_t i = 0; i < keys.size(); ++i) {
        _store.set(KVStore::Key(keys[i]), dfs[i]);
    }
}

std::vector<uint8_t> DistributedStore::local_map(std::string key, size_t opcode) {
    std::shared_ptr<Rower> rower = OperationGenerator::operation(opcode);
    assert(rower);


    std::vector<std::shared_ptr<DataFrame>> dfs;
    for(auto m : _keymap.at(key)) {
        auto df = _store.get(m.local_key);
        if(df) {
            dfs.push_back(df);
        }
    }

    // multi-thread map over chunks
    std::vector<std::shared_ptr<Rower>> rowers;
    std::vector<std::thread> threads;
    for(size_t i = 1; i < dfs.size(); ++i) {
        auto df = dfs[i];
        std::shared_ptr<Rower> r = std::static_pointer_cast<Rower>(rower->clone());
        assert(r);
        rowers.push_back(r);
        threads.emplace_back([df, r]{ df->map(*r); });
    }

    // main thread does first chunk
    dfs[0]->map(*rower);

    // join
    for(size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    // get results
    for(auto r : rowers) {
        rower->join(r);
    }

    return OperationGenerator::operation_result(opcode, rower);
}

std::vector<std::vector<uint8_t>> DistributedStore::distributed_map(std::string key, size_t opcode) {
    auto client_instance = Client::get_instance().lock();
    std::vector<sockaddr_in> nodes;

    bool local = false;
    for(auto md : _keymap.at(key)) {
        if(socket_util::sockaddr_eq(client_instance->get_self(), md.node)) {
            local = true;
        } else {
            nodes.push_back(md.node);
        }
    }

    std::mutex rmutex;
    std::vector<std::vector<uint8_t>> results;
    std::vector<std::thread> threads;
    for(auto node : nodes) {
        threads.emplace_back([client_instance, key, opcode, &rmutex, &results, node] {
                auto result = client_instance->operation_request(node, key, opcode);
                rmutex.lock();
                results.push_back(result);
                rmutex.unlock();
        });
    }

    if(local) {
        auto result = this->local_map(key, opcode);
        rmutex.lock();
        results.push_back(result);
        rmutex.unlock();
    }

    for(size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    return results;
}
