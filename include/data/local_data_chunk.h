#pragma once

#include "data/datachunk.h"
#include "data/rower.h"

#if 0 // comment out all of this easily
class LocalDataChunk : public DataChunk {
private:
    std::weak_ptr<Schema> _schema;
    std::vector<std::unique_ptr<Column>> _columns;

public:
    LocalDataChunk(const LocalDataChunk& ldc) = default;
    LocalDataChunk(LocalDataChunk&& ldc) = default;

    LocalDataChunk(std::weak_ptr<Schema> s, size_t row_start);

    LocalDataChunk(std::weak_ptr<Schema> s, size_t row_start, std::vector<std::unique_ptr<Column>> clist);

    void map(Rower& r) override;

    /** Return the value at the given column and row. Accessing rows or
    *  columns out of bounds, or request the wrong type is undefined.*/
    std::optional<int> get_int(size_t col, size_t row) const override;

    std::optional<bool> get_bool(size_t col, size_t row) const override;

    std::optional<double> get_double(size_t col, size_t row) const override;

    std::optional<std::string> get_string(size_t col, size_t row) const override;
};

template<>
inline LocalDataChunk Serializable::deserialize<LocalDataChunk>(std::vector<uint8_t>, size_t& pos){
    assert(false);
    auto s = std::make_shared<Schema>();
}
#endif
