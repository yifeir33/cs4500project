#ifndef CS4500_PROJECT_KEY_H
#define CS4500_PROJECT_KEY_H

#include "src/utils/string.h"
#include "src/utils/object.h"

#define PRIME_NUM 13;
#define PRIME_NUM1 19;

using namespace std;
struct Key: public Object {
    string name_;
    size_t home_;
    Key(string n): name_(n), home_(n.length()){}
    Key():name_(0),home_(0){}
    bool operator==(const Key &other) const {
        return(name_ == other.name_ && home_ == other.home_);
    }

};

class KeyHashFunction {
public:
    size_t operator()(const Key& k) const {
        int hash = 0;
        for(int i = 0; i< k.name_.length();i++){
            hash = hash + (int)k.name_[i] * PRIME_NUM;
        }
        hash = hash + k.home_ * PRIME_NUM1;
        return hash;
    }
};
#endif //CS4500_PROJECT_KEY_H
