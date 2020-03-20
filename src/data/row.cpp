#include "data/row.h"

Row::Row(const Schema& scm) : _width(scm.width()), _types(new char[_width]), _values(_width), _index(0) {
    for(size_t i = 0; i < _width; ++i) {
        _types[i] = scm.col_type(i);
    }
}

Row::~Row() {
    delete[] _types;
}

/** Setters: set the given column with the given value. Setting a column with
* a value of the wrong type is undefined. */
void Row::set(size_t col, int val) {
    assert(col < _width);
    assert(_types[col] == 'I');
    _values[col] = val;
}

void Row::set(size_t col, double val) {
    assert(col < _width);
    assert(_types[col] == 'F');
    _values[col] = val;
}

void Row::set(size_t col, bool val) {
    assert(col < _width);
    assert(_types[col] == 'B');
    _values[col] = val;
}

/** The string is external. */
void Row::set(size_t col, std::shared_ptr<std::string> val){
    _values[col] = val;
}

/** Set/get the index of this row (ie. its position in the dataframe. This is
*  only used for informational purposes, unused otherwise */
void Row::set_index(size_t idx) {
    _index = idx;

}

size_t Row::get_index() const {
    return _index;
}

/** Getters: get the value at the given column. If the column is not
* of the requested type, the result is undefined. */
int Row::get_int(size_t col) const {
    return std::get<int>(_values[col]);
}

bool Row::get_bool(size_t col) const {
    return std::get<bool>(_values[col]);
}

double Row::get_double(size_t col) const {
    return std::get<double>(_values[col]);
}

std::shared_ptr<std::string> Row::get_string(size_t col) const {
    return std::get<std::shared_ptr<std::string>>(_values[col]);
}

/** Number of fields in the row. */
size_t Row::width() const {
    return _width;
}

/** Type of the field at the given position. An idx >= width is  undefined. */
char Row::col_type(size_t idx) const {
    assert(idx < _width);
    return _types[idx];
}

/** Given a Fielder, visit every field of this row. The first argument is
* index of the row in the dataframe.
* Calling this method before the row's fields have been set is undefined. */
void Row::visit(size_t idx, Fielder& f) const {
    assert(idx = _index);

    for(size_t i = 0; i < _width; ++i) {
        switch(_types[i]){
            case 'I':
                f.accept(this->get_int(i));
                break;
            case 'F':
                f.accept(this->get_double(i));
                break;
            case 'B':
                f.accept(this->get_bool(i));
                break;
            case 'S':
                f.accept(this->get_string(i));
                break;
        }
    }
    f.done();
}

bool Row::equals(const Object *other) const {
    const Row *other_row = dynamic_cast<const Row*>(other);
    if(other_row) {
        if(_index == other_row->_index && _width == other_row->_width){
            for(size_t i = 0; i < _width; ++i){
                if(_types[i] != other_row->_types[i])
                    return false;

                switch(_types[i]){
                    case 'I':
                        if(this->get_int(i) != other_row->get_int(i))
                            return false;

                        break;
                    case 'F':
                        if(this->get_double(i) != other_row->get_double(i))
                            return false;

                        break;
                    case 'B':
                        if(this->get_bool(i) != other_row->get_bool(i))
                            return false;

                        break;
                    case 'S':
                        if(this->get_string(i)->compare(*other_row->get_string(i)) != 0)
                            return false;

                        break;
                }
            }
            return true;
        }
    }
    return false;
}

size_t Row::hash() const {
    size_t hash = _width + _index;
    for(size_t i = 0; i < _width; ++i) {
        hash += _types[i] * i;
        switch(_types[i]){
            case 'I':
                hash += this->get_int(i) * i;
                break;
            case 'F':
                hash += this->get_double(i) * i;
                break;
            case 'B':
                hash += this->get_bool(i) * i;
                break;
            case 'S':
                {
                    auto s = this->get_string(i);
                    hash += s->length();
                    for(size_t i = 0; i < s->length(); ++i){
                        hash += s->at(i) * i;
                    }
                    break;
                }
        }
    }
    return hash;
}

Object* Row::clone() const {
    return new Row(*this);
}
