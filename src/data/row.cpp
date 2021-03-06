#include "data/row.h"

Row::Row(const Schema& scm) : _width(scm.width()), _types(new char[_width]), _values(_width), _index(0) {
    for(size_t i = 0; i < _width; ++i) {
        _types[i] = scm.col_type(i);
    }
}

void Row::set(size_t col, std::optional<int> val) {
    assert(col < _width);
    assert(_types[col] == 'I');
    _values[col] = std::move(val);
}

void Row::set(size_t col, std::optional<double> val) {
    assert(col < _width);
    assert(_types[col] == 'F');
    _values[col] = std::move(val);
}

void Row::set(size_t col, std::optional<bool> val) {
    assert(col < _width);
    assert(_types[col] == 'B');
    _values[col] = std::move(val);
}

void Row::set(size_t col, std::optional<std::string> val){
    _values[col] = std::move(val);
}

void Row::set_index(size_t idx) {
    _index = idx;

}

size_t Row::get_index() const {
    return _index;
}

std::optional<int> Row::get_int(size_t col) const {
    return std::get<std::optional<int>>(_values[col]);
}

std::optional<bool> Row::get_bool(size_t col) const {
    return std::get<std::optional<bool>>(_values[col]);
}

std::optional<double> Row::get_double(size_t col) const {
    return std::get<std::optional<double>>(_values[col]);
}

std::optional<std::string> Row::get_string(size_t col) const {
    return std::get<std::optional<std::string>>(_values[col]);
}

size_t Row::width() const {
    return _width;
}

char Row::col_type(size_t idx) const {
    assert(idx < _width);
    return _types[idx];
}

void Row::visit(size_t idx, Fielder& f) const {
    assert(idx == _index);

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
                {
                    auto val = this->get_int(i);
                    hash += (val ? *val : 1) * i;
                    break;
                }
            case 'F':
                {
                    auto val = this->get_double(i);
                    hash += (val ? *val : 1) * i;
                    break;
                }
            case 'B':
                {
                    auto val = this->get_bool(i);
                    hash += (val ? *val + 2 : 1) * i;
                    break;
                }
            case 'S':
                {
                    auto s = this->get_string(i);
                    if(s) {
                        hash += s->length();
                        for(size_t i = 0; i < s->length(); ++i){
                            hash += s->at(i) * i;
                        }
                    } else {
                        hash += 1;
                    }
                    break;
                }
        }
    }
    return hash;
}

std::shared_ptr<Object> Row::clone() const {
    return std::make_shared<Row>(*this);
}
