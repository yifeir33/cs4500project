#include <functional>
#include <chrono>

#include "data/kvstore.h"
#include "data/dataframe.h"
#include "network/client.h"

KVStore::Key::Key(const std::string& n, size_t i) : _name(n), _node_idx(i) {}

KVStore::Key::Key(const std::string& n) : _name(n), _node_idx(0) {}

std::string KVStore::Key::get_name() const {
    return _name;
}

size_t KVStore::Key::get_node() const {
    return _node_idx;
}

size_t KVStore::Key::hash() const {
    return std::hash<std::string>()(_name);
}

bool KVStore::Key::equals(const Object *other) const {
    const KVStore::Key *okey = dynamic_cast<const KVStore::Key *>(other);
    if(okey) {
        // use overloaded operator
        return *this == *okey;
    }
    return false;
}

std::shared_ptr<Object> KVStore::Key::clone() const {
    return std::make_shared<KVStore::Key>(_name, _node_idx);
}

std::vector<uint8_t> KVStore::Key::serialize() const {
    std::vector<uint8_t> serialized;
    serialized.insert(serialized.end(),
                      reinterpret_cast<uint8_t *>(_node_idx),
                      reinterpret_cast<uint8_t *>(_node_idx) + sizeof(_node_idx));
    std::vector<uint8_t> svec = Serializable::serialize<std::string>(_name);
    serialized.insert(serialized.end(), svec.begin(), svec.end());
    return serialized;
}

bool KVStore::Key::operator==(const Key& other) const {
    if(_name.length() == other._name.length()) {
        for(size_t i = 0; i < _name.length(); ++i) {
            if(_name[i] != other._name[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// static variable/function
KVStore KVStore::kv_instance = KVStore();

KVStore& KVStore::get_instance() {
    return kv_instance;
}

// instance functions
std::shared_ptr<DataFrame> KVStore::get_local(const Key& k) {
    std::shared_lock local_lock(_local_map_mutex);
    if(_local_map.find(k) != _local_map.end()){
        return _local_map[k];
    }
    return nullptr;
}

std::shared_ptr<DataFrame> KVStore::get(const Key& k) {
    auto local = get_local(k);
    if(local) return local;

    std::shared_lock other_lock(_other_node_mutex);
    auto it = _other_nodes.find(k);
    if(it != _other_nodes.end()){
        auto real_key = Key(*it);
        other_lock.unlock();

        auto client = Client::get_instance().lock();
        assert(client);
        return client->get_value(real_key);
    }
    return nullptr;
}

std::shared_ptr<DataFrame> KVStore::get_or_wait(const Key& k, time_t timeout_ms) {
    std::shared_ptr<DataFrame> val = nullptr;
    auto timeout_dur = std::chrono::milliseconds(timeout_ms);
    auto start_time = std::chrono::steady_clock::now();

    while(!val 
            && (timeout_ms == 0
                  || (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time)
                      < timeout_dur))) {
        val = this->get(k);
    }
    return val;
}

void KVStore::set(const Key& k, const std::shared_ptr<DataFrame>& df){
    std::unique_lock local_lock(_local_map_mutex);
    _local_map.insert_or_assign(k, df);
}

std::shared_ptr<std::unordered_set<KVStore::Key, KVStore::KeyHasher>> KVStore::get_keys() {
    std::shared_lock local_lock(_local_map_mutex);
    auto set = std::make_shared<std::unordered_set<Key, KVStore::KeyHasher>>();
    for(auto kv : _local_map) {
        set->insert(kv.first);
    }
    return set;
}

size_t KVStore::hash() const {
    std::shared_lock local_lock(_local_map_mutex);
    size_t hash = _local_map.size();
    for(auto kv : _local_map) {
        hash += kv.first.hash();
        hash += kv.second->hash();
    }
    return hash;
}

bool KVStore::equals(const Object *other) const {
    auto okvs = dynamic_cast<const KVStore *>(other);
    if(okvs){
        return this->hash() == other->hash();
    }
    return false;
}

std::shared_ptr<Object> KVStore::clone() const {
    return nullptr;
}
