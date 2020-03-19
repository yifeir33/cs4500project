#include "column.h"

Column::~Column(){}

/** Type converters: Return same column under its actual type, or
*  nullptr if of the wrong type.  */
IntColumn* Column::as_int() { return nullptr; }
BoolColumn*  Column::as_bool() { return nullptr; }
FloatColumn* Column::as_float() { return nullptr; }
StringColumn* Column::as_string() { return nullptr; }

/** Type appropriate.push_back_back methods. Calling the wrong method is
* undefined behavior. **/

void Column::push_back([[maybe_unused]] int val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] bool val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] double val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] std::shared_ptr<std::string> val) { exit_if_not(false, "Wrong.push_back back method called on column"); }

size_t Column::hash() const {
    return 500;
}

// Int Column
IntColumn::IntColumn(int n, ...) : _data() {
    va_list args;
    va_start(args, n);

    for(int i = 0; i < n; ++i) {
        _data.push_back(va_arg(args, int));
    }
    va_end(args);
}

void IntColumn::push_back(int val) {
  _data.push_back(val);
}

int IntColumn::get(size_t idx) const {
  return _data[idx];
}

IntColumn* IntColumn::as_int() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
int IntColumn::set(size_t idx, int val) {
    int old = _data[idx];
    _data[idx] = val;
    return old;
}

size_t IntColumn::size() const {
  return _data.size();
}


char IntColumn::get_type() const {
  return 'I';
}

bool IntColumn::equals(const Object *other) const {
    const IntColumn *oic = dynamic_cast<const IntColumn*>(other);
    if(oic){
        return _data == oic->_data;
    }
    return false;
}

size_t IntColumn::hash() const {
    size_t hash = Column::hash() + 5;
    for(size_t i = 0; i < _data.size(); ++i) {
        hash += _data[i] * i;
    }
    return hash;
}

Object *IntColumn::clone() const {
    return new IntColumn(*this);
}

// Float Column
FloatColumn::FloatColumn(int n, ...) : _data() {
    va_list args;
    va_start(args, n);

    for(int i = 0; i < n; ++i) {
        _data.push_back(va_arg(args, double));
    }
    va_end(args);
}

void FloatColumn::push_back(double val) {
  _data.push_back(val);
}

double FloatColumn::get(size_t idx) const {
  return _data[idx];
}

FloatColumn* FloatColumn::as_float() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
double FloatColumn::set(size_t idx, double val) {
    double old = _data[idx];
    _data[idx] = val;
    return old;
}

size_t FloatColumn::size() const {
  return _data.size();
}


char FloatColumn::get_type() const {
  return 'F';
}

bool FloatColumn::equals(const Object *other) const {
    const FloatColumn *oic = dynamic_cast<const FloatColumn*>(other);
    if(oic){
        return _data == oic->_data;
    }
    return false;
}

size_t FloatColumn::hash() const {
    size_t hash = Column::hash() + 5;
    for(size_t i = 0; i < _data.size(); ++i) {
        hash += _data[i] * i;
    }
    return hash;
}

Object *FloatColumn::clone() const {
    return new FloatColumn(*this);
}

// Bool Column
BoolColumn::BoolColumn(int n, ...) : _data() {
    va_list args;
    va_start(args, n);

    for(int i = 0; i < n; ++i) {
        _data.push_back(va_arg(args, double));
    }
    va_end(args);
}

void BoolColumn::push_back(bool val) {
  _data.push_back(val);
}

bool BoolColumn::get(size_t idx) const {
  return _data[idx];
}

BoolColumn* BoolColumn::as_bool() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
bool BoolColumn::set(size_t idx, bool val) {
    bool old = _data[idx];
    _data[idx] = val;
    return old;
}

size_t BoolColumn::size() const {
  return _data.size();
}


char BoolColumn::get_type() const {
  return 'B';
}

bool BoolColumn::equals(const Object *other) const {
    const BoolColumn *oic = dynamic_cast<const BoolColumn*>(other);
    if(oic){
        return _data == oic->_data;
    }
    return false;
}

size_t BoolColumn::hash() const {
    size_t hash = Column::hash() + 5;
    for(size_t i = 0; i < _data.size(); ++i) {
        hash += _data[i] * i;
    }
    return hash;
}

Object *BoolColumn::clone() const {
    return new BoolColumn(*this);
}

// String Column
StringColumn::StringColumn(int n, ...) : _data() {
    va_list args;
    va_start(args, n);

    for(int i = 0; i < n; ++i) {
        _data.push_back(std::make_shared<std::string>(va_arg(args, char*)));
    }
    va_end(args);
}

void StringColumn::push_back(std::shared_ptr<std::string> val) {
  _data.push_back(val);
}

std::weak_ptr<std::string> StringColumn::get(size_t idx) {
  return _data[idx];
}

StringColumn* StringColumn::as_string() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
std::shared_ptr<std::string> StringColumn::set(size_t idx, std::shared_ptr<std::string> val) {
    auto old = _data[idx];
    _data[idx] = val;
    return old;
}

size_t StringColumn::size() const {
  return _data.size();
}


char StringColumn::get_type() const {
  return 'S';
}

bool StringColumn::equals(const Object *other) const {
    const StringColumn *oic = dynamic_cast<const StringColumn*>(other);
    if(oic){
        return _data == oic->_data;
    }
    return false;
}

size_t StringColumn::hash() const {
    size_t hash = Column::hash() + 5;
    for(size_t i = 0; i < _data.size(); ++i) {
        auto str = _data[i];
        size_t inner_hash = str->length() * i;
        for(size_t j = 0; j < str->length(); ++j){
            inner_hash += str->at(j) * j;
        }
        hash += inner_hash;
    }
    return hash;
}

Object *StringColumn::clone() const {
    return new StringColumn(*this);
}
