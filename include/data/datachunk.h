#pragma once

#include "util/serializable.h"
#include "data/column.h"
#include "data/rower.h"

class DataChunk : public Serializable {
protected:
    size_t _row_start;

public:
    DataChunk(size_t row_start) : _row_start(row_start) {};

    virtual ~DataChunk(){};

    virtual void map(Rower& r) = 0;

    /** Return the value at the given column and row. Accessing rows or
    *  columns out of bounds, or request the wrong type is undefined.*/
    virtual std::optional<int> get_int(size_t col, size_t row) const = 0;

    virtual std::optional<bool> get_bool(size_t col, size_t row) const = 0;

    virtual std::optional<double> get_double(size_t col, size_t row) const = 0;

    virtual std::optional<std::string> get_string(size_t col, size_t row) const = 0;
};
