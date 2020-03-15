#include "schema.h"

/** Copying constructor */
Schema::Schema(const Schema& from) : _columnNames(from._columnNames.length()),
_rowNames(from._rowNames.length()), _columnTypes(from._columnTypes.length()), _width(from._width),
_length(from._length) {};

/** Create an empty schema **/
Schema::Schema() : _columnNames(10), _rowNames(10), _columnTypes(10), _width(0), _length(0) {};

/** Create a schema from a string of types. A string that contains
* characters other than those identifying the four type results in
* undefined behavior. The argument is external, a nullptr argument is
* undefined. **/
Schema::Schema(std::string& types) : Schema(types.c_str()) {};

Schema::Schema(const char* types) : _columnNames(10), _rowNames(10),
_columnTypes(10), _width(0), _length(0) {
    assert(types);
    const char *c = types;
    while(*c !='\0') {
        if(*c != ' '){
            _columnTypes.push(*c);
        }
        ++c;
        ++_width;
    }
};

/** Add a column of the given type and name (can be nullptr), name
* is external. Names are expectd to be unique, duplicates result
* in undefined behavior. */
void Schema::add_column(char typ, std::string* name) {
    _columnTypes.push(typ);
    _columnNames.push(name);
    ++_width;
}

/** Add a row with a name (possibly nullptr), name is external.  Names are
*  expectd to be unique, duplicates result in undefined behavior. */
void Schema::add_row(std::string* name) {
    _rowNames.push(name);
    ++_length;
}

/** Return name of row at idx; nullptr indicates no name. An idx >= width
* is undefined. */
std::weak_ptr<std::string> Schema::row_name(size_t idx) const {
    return _rowNames.get(idx);
}

/** Return name of column at idx; nullptr indicates no name given.
*  An idx >= width is undefined.*/
std::weak_ptr<std::string> Schema::col_name(size_t idx) const {
    return _columnNames.get(idx);
}

/** Return type of column at idx. An idx >= width is undefined. */
char Schema::col_type(size_t idx) const {
    return _columnTypes.get(idx);
}

/** Given a column name return its index, or -1. */
int Schema::col_idx(std::string& name) const {
    return _columnNames.index_of(name);
}

/** Given a row name return its index, or -1. */
int Schema::row_idx(std::string& name) const {
    return _rowNames.index_of(name);
}

/** The number of columns */
size_t Schema::width() const {
    return _width;
}

/** The number of rows */
size_t Schema::length() const {
    return _length;
}

bool Schema::equals(Object* other) const {
    Schema *other_schema = dynamic_cast<Schema*>(other);
    if(other_schema && this->_width == other_schema->_width && this->_length == other_schema->_length) {
        return _rowNames.equals(&other_schema->_rowNames) 
               && _columnNames.equals(&other_schema->_columnNames) 
               && _columnTypes.equals(&other_schema->_columnTypes);
    }
    return false;
}

size_t Schema::hash() const {
    return _rowNames.hash() + _columnNames.hash() + _columnTypes.hash();
}
