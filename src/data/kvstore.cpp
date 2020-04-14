#include <functional>
#include <chrono>

#include "data/kvstore.h"
#include "data/dataframe.h"
#include "network/client.h"

KVStore::Key::Key(const std::string& n, sockaddr_in node) : _name(n), _node(node) {}

KVStore::Key::Key(const std::string& n) : _name(n), _node({}) {}

std::string KVStore::Key::get_name() const {
    return _name;
}

sockaddr_in KVStore::Key::get_node() const {
    return _node;
}

void KVStore::Key::set_node(const sockaddr_in& idx) const {
    _node = idx;
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
    return std::make_shared<KVStore::Key>(_name, _node);
}

std::vector<uint8_t> KVStore::Key::serialize() const {
    std::vector<uint8_t> serialized;
    serialized.insert(serialized.end(),
                      reinterpret_cast<uint8_t *>(&_node),
                      reinterpret_cast<uint8_t *>(&_node) + sizeof(_node));
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
    std::lock_guard<std::mutex> local_lock(_local_map_mutex);
    auto it = _local_map.find(k);
    if(it != _local_map.end()){
        return std::shared_ptr<DataFrame>((*it).second);
    }
    return nullptr;
}

std::shared_ptr<DataFrame> KVStore::get(const Key& k) {
    auto local = get_local(k);
    if(local) return local;

    std::unique_lock<std::mutex> other_lock(_other_node_mutex);
    if(_other_nodes.empty()) return nullptr;

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
    pln("get_or_wait");
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
    { // scope to control lock
        // ensure it doesn't exist remotely. This should be changed in the future.
        std::lock_guard<std::mutex> remote_lock(_other_node_mutex);
        assert(_other_nodes.find(k) == _other_nodes.end());
    }
    std::lock_guard<std::mutex> local_lock(_local_map_mutex);
    _local_map.insert_or_assign(k, df);
}

void KVStore::add_nonlocal(Key k) {
    std::lock_guard<std::mutex> other_lock(_other_node_mutex);

    if(_other_nodes.empty()){
        _other_nodes.insert(std::move(k));
    } else {
        auto it = _other_nodes.find(k);
        if(it != _other_nodes.end()){
            (*it).set_node(k.get_node());
        } else {
            _other_nodes.insert(std::move(k));
        }
    }
}


std::unordered_set<std::string> KVStore::get_local_keys() const {
    std::unordered_set<std::string> set;
    std::lock_guard<std::mutex> local_lock(_local_map_mutex);
    for(auto kv : _local_map) {
        set.insert(kv.first.get_name());
    }
    return set;
}

size_t KVStore::hash() const {
    std::lock_guard<std::mutex> local_lock(_local_map_mutex);
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
