#pragma once

#include <memory>
#include <string>

#include "object.h"

/*****************************************************************************
 * Fielder::
 * A field vistor invoked by Row.
 */
class Fielder : public Object {
public:
    /** Called before visiting a row, the argument is the row offset in the
    dataframe. */
    virtual void start(size_t r);

    /** Called for fields of the argument's type with the value of the field. */
    virtual void accept(bool b);
    virtual void accept(float f);
    virtual void accept(int i);
    virtual void accept(std::weak_ptr<std::string> s);

    /** Called when all fields have been seen. */
    virtual void done();
};
