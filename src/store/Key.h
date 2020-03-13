#ifndef CS4500_PROJECT_KEY_H
#define CS4500_PROJECT_KEY_H

#include "src/utils/string.h"
#include "src/utils/object.h"
class Key: public Object {
    String name_;
    size_t home_;
    Key(String name): name_(name), home_(0) {}

};
#endif //CS4500_PROJECT_KEY_H
