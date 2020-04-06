#pragma once

#include <arpa/inet.h>

#include "data/datachunk.h"

class RemoteDataChunk : public DataChunk {
private:
    sockaddr_in _home_node;

public:
    RemoteDataChunk();

    /* TODO
    virtual void map(Rower& r) override;
    */

    /** Return the value at the given column and row. Accessing rows or
    *  columns out of bounds, or request the wrong type is undefined.*/
    /* TODO
    std::optional<int> get_int(size_t col, size_t row) const override;

    virtual std::optional<bool> get_bool(size_t col, size_t row) const override;

    virtual std::optional<double> get_double(size_t col, size_t row) const override;

    virtual std::optional<std::string> get_string(size_t col, size_t row) const override;
    */
};
