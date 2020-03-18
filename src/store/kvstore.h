#ifndef CS4500_PROJECT_KVSTORE_H
#define CS4500_PROJECT_KVSTORE_H

#include "Key.h"
#include "Value.h"
#include "src/utils/object.h"
#include <unordered_map>
#include <unordered_set>
#include <netinet/in.h>

using namespace std;

class KVstore: public Object{
private:
    unordered_map<Key,Value, KeyHashFunction> _local_map;
    /**
     * not sure for the sockaddr part
     */
    unordered_map<sockaddr_in,unordered_set<Key>> _other_nodes;
public:
    KVstore();
    Value get(Key k){
        if(_local_map.find(k) != _local_map.end()){
            return _local_map[k];
        } else{
            for(auto &&client: _other_nodes){
                if(_other_nodes[client].find(k) != _other_nodes[client].end()){
                    //get key from network and return
                }
            }
        }
        return nullptr;
    }

    void set(Key k, Value v){
        for(auto &&client:_other_nodes[client].find(k) != _other_nodes[client].end()){
            //update
        }
        _local_map.insert_or_assign(k,v);
    }


};

#endif //CS4500_PROJECT_KVSTORE_H
