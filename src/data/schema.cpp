#include "data/schema.h"

/** Copying constructor */
Schema::Schema(const Schema& from) : _columnNames(),_rowNames(),
 _columnTypes(from._columnTypes), _width(from._width),
_length(from._length) {}

/** Create an empty schema **/
Schema::Schema() : _columnNames(), _rowNames(), _columnTypes(), _width(0), _length(0) {}

/** Create a schema from a string of types. A string that contains
* characters other than those identifying the four type results in
* undefined behavior. The argument is external, a nullptr argument is
* undefined. **/
Schema::Schema(const std::string& types) : Schema(types.c_str()) {}

Schema::Schema(const char* types) : _columnNames(), _rowNames(),
_columnTypes(), _width(0), _length(0) {
    const char *c = types;
    while(*c != '\0') {
        if(*c != ' '){
            _columnTypes.push_back(*c);
            ++_width;
        }
        ++c;
    }
}

/**
 * Move constructor so that serializing values, and
 * constructing a pointer from the stack allocated class is
 * more efficient
 */
/* Schema::Schema(Schema&& other) : _columnNames(), _rowNames(), _columnTypes(std::move(other._columnTypes)), _width(other._width), _length(other._length) {} */

/** Add a column of the given type and name (can be nullptr), name
* is external. Names are expectd to be unique, duplicates result
* in undefined behavior. */
void Schema::add_column(char typ, std::optional<std::string> name) {
    _columnTypes.push_back(typ);
    _columnNames.push_back(name);
    ++_width;
}

/** Add a row with a name (possibly nullptr), name is external.  Names are
*  expectd to be unique, duplicates result in undefined behavior. */
void Schema::add_row(std::optional<std::string> name) {
    _rowNames.push_back(name);
    ++_length;
}

/** Return name of row at idx; nullptr indicates no name. An idx >= width
* is undefined. */
std::optional<std::string> Schema::row_name(size_t idx) const {
    return _rowNames[idx];
}

/** Return name of column at idx; nullptr indicates no name given.
*  An idx >= width is undefined.*/
std::optional<std::string> Schema::col_name(size_t idx) const {
    return _columnNames[idx];
}

/** Return type of column at idx. An idx >= width is undefined. */
char Schema::col_type(size_t idx) const {
    assert(idx < _columnTypes.size());
    return _columnTypes[idx];
}

/** Given a column name return its index, or -1. */
int Schema::col_idx(std::string& name) const {
    for(size_t i = 0; i < _columnNames.size(); ++i) {
        if(_columnNames[i]->compare(name) == 0){
            return i;
        }
    }
    return -1;
}

/** Given a row name return its index, or -1. */
int Schema::row_idx(std::string& name) const {
    for(size_t i = 0; i < _rowNames.size(); ++i) {
        if(_rowNames[i]->compare(name) == 0){
            return i;
        }
    }
    return -1;
}

/** The number of columns */
size_t Schema::width() const {
    return _width;
}

/** The number of rows */
size_t Schema::length() const {
    return _length;
}

std::shared_ptr<Object> Schema::clone() const {
    return std::make_shared<Schema>(*this);
}

bool Schema::equals(const Object* other) const {
    const Schema *other_schema = dynamic_cast<const Schema*>(other);
    if(other_schema && this->_width == other_schema->_width 
            && this->_length == other_schema->_length  
            && this->_columnTypes.size() == other_schema->_columnTypes.size()
            && this->_columnNames.size() == other_schema->_columnNames.size() 
            && this->_rowNames.size() == other_schema->_rowNames.size()) {
        for(size_t i = 0; i < _columnNames.size(); ++i) {
            if(_columnNames[i]->compare(*(other_schema->_columnNames[i])) != 0) {
                return false;
            }
        }
        for(size_t i = 0; i < _rowNames.size(); ++i) {
            if(_rowNames[i]->compare(*(other_schema->_rowNames[i])) != 0) {
                return false;
            }
        }
        for(size_t i = 0; i < _columnTypes.size(); ++i) {
            if(_columnTypes[i] != other_schema->_columnTypes[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

size_t Schema::hash() const {
    size_t hash = _columnNames.size() + _rowNames.size() + _columnTypes.size();
    for(size_t i = 0; i < _columnNames.size(); ++i) {
        for(size_t j = 0; j < _columnNames.size(); ++j) {
            hash += _columnNames[i]->at(j) * j;
        }
    }
    for(size_t i = 0; i < _rowNames.size(); ++i) {
        for(size_t j = 0; j < _rowNames.size(); ++j) {
            hash += _rowNames[i]->at(j) * j;
        }
    }
    for(size_t i = 0; i < _columnTypes.size(); ++i) {
        hash += _columnTypes[i] * i;
    }
    return hash;
}

std::vector<uint8_t> Schema::serialize() const {
    auto serialized = Serializable::serialize<size_t>(this->_width);

    // schema type
    for(size_t i = 0; i < this->_width; ++i) {
        auto temp = Serializable::serialize<char>(_columnTypes[i]);
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }
    return serialized;
}

