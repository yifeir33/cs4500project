#include <functional>

#include "kvstore.h"
#include "dataframe.h"

KVStore::Key::Key(const std::string& n, size_t i) : _name(n), _node_idx(i) {}

std::string KVStore::Key::get_name() const {
    return _name;
}

size_t KVStore::Key::get_node() const {
    return _node_idx;
}

size_t KVStore::Key::hash() const {
    return _node_idx + std::hash<std::string>()(_name);
}

bool KVStore::Key::equals(const Object *other) const {
    const KVStore::Key *okey = dynamic_cast<const KVStore::Key *>(other);
    if(okey) {
        // use overloaded operator
        return *this == *okey;
    }
    return false;
}

Object *KVStore::Key::clone() const {
    return new Key(_name, _node_idx);
}

bool KVStore::Key::operator==(const Key& other) const {
    if(_node_idx == other._node_idx
            && _name.length() == other._name.length()) {
        for(size_t i = 0; i < _name.length(); ++i) {
            if(_name[i] != other._name[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}


KVStore::KVStore() : _local_map() {}

std::shared_ptr<DataFrame> KVStore::get(const Key& k) {
    if(_local_map.find(k) != _local_map.end()){
        return _local_map[k];
    }
    return nullptr;
}

std::shared_ptr<DataFrame> KVStore::get_and_wait(const Key& k) {
    auto val = this->get(k);
    if(!val) {
        // TODO: search network
    }
    return val;
}

void KVStore::set(const Key& k, const std::shared_ptr<DataFrame>& df){
    _local_map.insert_or_assign(k, df);
}

std::shared_ptr<std::unordered_set<KVStore::Key, KVStore::KeyHasher>> KVStore::get_keys() {
    auto set = std::make_shared<std::unordered_set<Key, KVStore::KeyHasher>>();
    for(auto kv : _local_map) {
        set->insert(kv.first);
    }
    return set;
}

size_t KVStore::hash() const {
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

Object *KVStore::clone() const {
    return new KVStore();
}
