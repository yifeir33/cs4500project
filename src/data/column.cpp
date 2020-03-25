#include "data/column.h"

Column::~Column(){}

/** Type converters: Return same column under its actual type, or
*  nullptr if of the wrong type.  */
IntColumn* Column::as_int() { return nullptr; }
BoolColumn*  Column::as_bool() { return nullptr; }
FloatColumn* Column::as_float() { return nullptr; }
StringColumn* Column::as_string() { return nullptr; }

/** Type appropriate.push_back_back methods. Calling the wrong method is
* undefined behavior. **/
void Column::push_back([[maybe_unused]] std::optional<int> val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] std::optional<bool> val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] std::optional<double> val) { exit_if_not(false, "Wrong.push_back back method called on column"); }
void Column::push_back([[maybe_unused]] std::optional<std::string> val) { exit_if_not(false, "Wrong.push_back back method called on column"); }

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

void IntColumn::push_back(std::optional<int> val) {
  _data.push_back(val);
}

std::optional<int> IntColumn::get(size_t idx) const {
  return _data[idx];
}

IntColumn* IntColumn::as_int() {
  return this;
}

/** Set value at idx. An out of bound idx is undefined.  */
std::optional<int> IntColumn::set(size_t idx, std::optional<int> val) {
    auto old = _data[idx];
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
        hash += (_data[i] ? *_data[i] : 1) * i;
    }
    return hash;
}

std::shared_ptr<Object> IntColumn::clone() const {
    return std::make_shared<IntColumn>(*this);
}

std::vector<uint8_t> IntColumn::serialize() const {
    // we mark missing with a full byte, big waste of space
    // but byte alignment is a headache

    // Write length of vector
    std::vector<uint8_t> serialized = Serializable::serialize(this->_data.size());
    std::vector<uint8_t> temp;
    for(size_t i = 0; i < this->_data.size(); ++i) {
        if(_data[i]){
            temp = Serializable::serialize(true);
            serialized.insert(serialized.end(), temp.begin(), temp.end());
            temp = Serializable::serialize(*_data[i]);
        } else {
            temp = Serializable::serialize(false);
        }
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }

    return serialized;
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

void FloatColumn::push_back(std::optional<double> val) {
  _data.push_back(val);
}

std::optional<double> FloatColumn::get(size_t idx) const {
  return _data[idx];
}

FloatColumn* FloatColumn::as_float() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
std::optional<double> FloatColumn::set(size_t idx, std::optional<double> val) {
    auto old = _data[idx];
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
        hash += (_data[i] ? *_data[i] : 1) * i;
    }
    return hash;
}

std::shared_ptr<Object> FloatColumn::clone() const {
    return std::make_shared<FloatColumn>(*this);
}

std::vector<uint8_t> FloatColumn::serialize() const {
    // we mark missing with a full byte, big waste of space
    // but byte alignment is a headache

    // Write length of vector
    std::vector<uint8_t> serialized = Serializable::serialize(this->_data.size());
    std::vector<uint8_t> temp;
    for(size_t i = 0; i < this->_data.size(); ++i) {
        if(_data[i]){
            temp = Serializable::serialize(true);
            serialized.insert(serialized.end(), temp.begin(), temp.end());
            temp = Serializable::serialize(*_data[i]);
        } else {
            temp = Serializable::serialize(false);
        }
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }

    return serialized;
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

void BoolColumn::push_back(std::optional<bool> val) {
  _data.push_back(val);
}

std::optional<bool> BoolColumn::get(size_t idx) const {
  return _data[idx];
}

BoolColumn* BoolColumn::as_bool() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
std::optional<bool> BoolColumn::set(size_t idx, std::optional<bool> val) {
    auto old = _data[idx];
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
        hash += (_data[i] ? *_data[i] + 2 : 1) * i;
    }
    return hash;
}

std::shared_ptr<Object> BoolColumn::clone() const {
    return std::make_shared<BoolColumn>(*this);
}

std::vector<uint8_t> BoolColumn::serialize() const {
    // we mark missing with a full byte, big waste of space
    // but byte alignment is a headache

    // Write length of vector
    std::vector<uint8_t> serialized = Serializable::serialize(this->_data.size());
    std::vector<uint8_t> temp;
    for(size_t i = 0; i < this->_data.size(); ++i) {
        if(_data[i]){
            temp = Serializable::serialize(true);
            serialized.insert(serialized.end(), temp.begin(), temp.end());
            temp = Serializable::serialize(*_data[i]);
        } else {
            temp = Serializable::serialize(false);
        }
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }

    return serialized;
}

// String Column
StringColumn::StringColumn(int n, ...) : _data() {
    va_list args;
    va_start(args, n);

    for(int i = 0; i < n; ++i) {
        _data.push_back(std::optional<std::string>(va_arg(args, const char *)));
    }
    va_end(args);
}

void StringColumn::push_back(std::optional<std::string> val) {
  _data.push_back(val);
}

std::optional<std::string> StringColumn::get(size_t idx) {
  return _data[idx];
}

StringColumn* StringColumn::as_string() {
  return this;
}
/** Set value at idx. An out of bound idx is undefined.  */
std::optional<std::string> StringColumn::set(size_t idx, std::optional<std::string> val) {
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
        if(str) {
            size_t inner_hash = str->length() * i;
            for(size_t j = 0; j < str->length(); ++j){
                inner_hash += str->at(j) * j;
            }
            hash += inner_hash;
        } else {
            hash += i;
        }
    }
    return hash;
}

std::shared_ptr<Object> StringColumn::clone() const {
    return std::make_shared<StringColumn>(*this);
}

std::vector<uint8_t> StringColumn::serialize() const {
    // we mark missing with a full byte, big waste of space
    // but byte alignment is a headache

    // Write length of vector
    std::vector<uint8_t> serialized = Serializable::serialize(this->_data.size());
    std::vector<uint8_t> temp;
    for(size_t i = 0; i < this->_data.size(); ++i) {
        if(_data[i]){
            temp = Serializable::serialize(true);
            serialized.insert(serialized.end(), temp.begin(), temp.end());
            temp = Serializable::serialize_string(*_data[i]);
        } else {
            temp = Serializable::serialize(false);
        }
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }

    return serialized;
}
