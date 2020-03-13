#ifndef CS4500_PROJECT_VALUE_H
#define CS4500_PROJECT_VALUE_H

#include "src/utils/object.h"
class Value: public Object{
    char* data_;
    Value(char* ): data_() {}
};

#endif //CS4500_PROJECT_VALUE_H
