#pragma once

#include <memory>
#include <string>
#include <optional>

#include "util/object.h"

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
    virtual void accept(std::optional<bool> b);
    virtual void accept(std::optional<double> d);
    virtual void accept(std::optional<int> i);
    virtual void accept(std::weak_ptr<std::string> s);

    /** Called when all fields have been seen. */
    virtual void done();
};
