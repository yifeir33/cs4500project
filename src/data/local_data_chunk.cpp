
#include "data/local_data_chunk.h"

#if 0 // comment out all of this easily
LocalDataChunk::LocalDataChunk(std::weak_ptr<Schema> s, size_t row_start) 
: DataChunk(row_start), _schema(s), _columns() {}

LocalDataChunk::LocalDataChunk(std::weak_ptr<Schema> s, size_t row_start, std::vector<std::unique_ptr<Column>> clist)
: DataChunk(row_start), _schema(s), _columns(clist) {}

void LocalDataChunk::map(Rower& rower) {
    auto schema_ptr = _schema.lock();
    assert(schema_ptr);

    Row row(*schema_ptr);
    for(size_t r = 0; r < _columns[0]->size(); ++r) {
        row.set_index(_row_start + r);
        for(size_t c = 0; c < _columns.size(); ++c){
            switch(schema_ptr->col_type(c)){
                case 'I':
                    row.set(c, _columns[c]->as_int()->get(r));
                    break;
                case 'F':
                    row.set(c, _columns[c]->as_int()->get(r));
                    break;
                case 'S':
                    row.set(c, _columns[c]->as_int()->get(r));
                    break;
                case 'B':
                    row.set(c, _columns[c]->as_int()->get(r));
                    break;
                default:
                    assert(false); // unreachable
            }
        }
        rower.accept(row);
    }
}

std::optional<int> LocalDataChunk::get_int(size_t col, size_t row) const {
    assert(col < _columns.size() && _row_start <= row && row < _columns[0]->size());
    assert(_columns[col]->get_type() == 'I');
    return _columns[col]->as_int()->get(row - _row_start);
}

std::optional<bool> LocalDataChunk::get_bool(size_t col, size_t row) const {
    assert(col < _columns.size() && _row_start <= row && row < _columns[0]->size());
    assert(_columns[col]->get_type() == 'B');
    return _columns[col]->as_bool()->get(row - _row_start);
}

std::optional<double> LocalDataChunk::get_double(size_t col, size_t row) const {
    assert(col < _columns.size() && _row_start <= row && row < _columns[0]->size());
    assert(_columns[col]->get_type() == 'F');
    return _columns[col]->as_float()->get(row - _row_start);
}

std::optional<std::string> LocalDataChunk::get_string(size_t col, size_t row) const {
    assert(col < _columns.size() && _row_start <= row && row < _columns[0]->size());
    assert(_columns[col]->get_type() == 'F');
    return _columns[col]->as_string()->get(row - _row_start);
}

#endif
