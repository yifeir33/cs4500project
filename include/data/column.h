#pragma once

#include <stdarg.h>
#include <assert.h>
#include <string>
#include <vector>
#include <optional>

#include "util/serializable.h"
#include "data/nullable_array.h"

// Forward Declarations for Column
class IntColumn;
class BoolColumn;
class FloatColumn;
class StringColumn;

/**************************************************************************
 * Column ::
 * Represents one column of a data frame which holds values of a single type.
 * This abstract class defines methods overriden in subclasses. There is
 * one subclass per element type. Columns are mutable, equality is pointer
 * equality. For the most part these are implemented through the composition of
 * a nullable array. */
class Column : public Serializable {
public:
    virtual ~Column();

    /** Type converters: Return same column under its actual type, or
    *  nullptr if of the wrong type.  */
    virtual IntColumn* as_int();
    virtual BoolColumn*  as_bool();
    virtual FloatColumn* as_float();
    virtual StringColumn* as_string();

    /** Type appropriate push_back methods. Calling the wrong method is
    * undefined behavior. Pushes the given value onto the end
    * of the column. If the optional is null, then it marks 
    * that value as missing. **/
    virtual void push_back(std::optional<int> val);
    virtual void push_back(std::optional<bool> val);
    virtual void push_back(std::optional<double> val);
    virtual void push_back(std::optional<std::string> val);

    /** Returns the number of elements in the column. */
    virtual size_t size() const = 0;

    /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. */
    virtual char get_type() const = 0;

    /** Returns the hashcode of the column. */
    size_t hash() const override;
};
 
/*************************************************************************
 * IntColumn::
 * Holds int values.
 */
class IntColumn : public Column {
private:
    /** Internal data structure holding the data */
    NullableArray<int> _data;

public:
    IntColumn() = default;
    IntColumn(const IntColumn& other) = default;
    IntColumn(IntColumn&& other) = default;
    IntColumn(int n, ...);

    /** Pushes the given value onto the end
    * of the column. If the optional is null, then it marks 
    * that value as missing. **/
    void push_back(std::optional<int> val) override;

    /** Get the value at the given index. A null optional 
     * signifies a missing value. */
    std::optional<int> get(size_t idx) const;

    /** Get this as an integer column. */
    IntColumn* as_int() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<int> set(size_t idx, std::optional<int> val);

    /** Returns the number of elements in the column. */
    size_t size() const override;

    /** Gets the type of the column. Returns 'I' */
    char get_type() const override;

    /** Test for equality */
    bool equals(const Object *other) const override;

    /** Returns the hashcode of the column. */
    size_t hash() const override;
    
    /** Returns a new copy of the current column. */
    std::shared_ptr<Object> clone() const override;

    /** Serializes the current column into a byte representation. */
    std::vector<uint8_t> serialize() const override;
};

/** Specialization of deserialize for IntColumn
 *
 * Converts from the serialized form of this column to an object containing 
 * the data */
template<>
inline IntColumn Serializable::deserialize<IntColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<int>::deserialize(data, pos);
    IntColumn ic;
    for(size_t i = 0; i < na.size(); ++i){
        ic.push_back(na.get(i));
    }
    return ic;
}
 
/*************************************************************************
 * FloatColumn::
 * Holds floating point values.
 */
class FloatColumn : public Column {
private:
    /** Internal data structure holding the data */
    NullableArray<double> _data;

public:
    FloatColumn() = default;
    FloatColumn(const FloatColumn& other) = default;
    FloatColumn(FloatColumn&& other) = default;
    FloatColumn(int n, ...);

    /** Pushes the given value onto the end
    * of the column. If the optional is null, then it marks 
    * that value as missing. **/
    void push_back(std::optional<double> val) override;

    /** Get the value at the given index. A null optional 
     * signifies a missing value. */
    std::optional<double> get(size_t idx) const;

    /** Get this as an float column. */
    FloatColumn* as_float() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<double> set(size_t idx, std::optional<double> val);

    /** Returns the number of elements in the column. */
    size_t size() const override;

    /** Gets the type of the column. Returns 'F' */
    char get_type() const override;

    /** Test for equality */
    bool equals(const Object *other) const override;

    /** Returns the hashcode of the column. */
    size_t hash() const override;

    /** Returns a new copy of the current column. */
    std::shared_ptr<Object> clone() const override;

    /** Serializes the current column into a byte representation. */
    std::vector<uint8_t> serialize() const override;
};

/** Specialization of deserialize for FloatColumn
 *
 * Converts from the serialized form of this column to an object containing 
 * the data */
template<>
inline FloatColumn Serializable::deserialize<FloatColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<double>::deserialize(data, pos);
    FloatColumn fc;
    for(size_t i = 0; i < na.size(); ++i){
        fc.push_back(na.get(i));
    }
    return fc;
}

/*************************************************************************
 * BoolColumn::
 * Holds boolean values.
 */
class BoolColumn : public Column {
private:
    /** Internal data structure holding the data */
    NullableArray<bool> _data;

public:
    BoolColumn() = default;
    BoolColumn(const BoolColumn& other) = default;
    BoolColumn(BoolColumn&& other) = default;
    BoolColumn(int n, ...);

    /** Pushes the given value onto the end
    * of the column. If the optional is null, then it marks 
    * that value as missing. **/
    void push_back(std::optional<bool> val) override;

    /** Get the value at the given index. A null optional 
     * signifies a missing value. */
    std::optional<bool> get(size_t idx) const;

    /** Get this as an boolean column. */
    BoolColumn* as_bool() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<bool> set(size_t idx, std::optional<bool> val);

    /** Returns the number of elements in the column. */
    size_t size() const override;

    /** Gets the type of the column. Returns 'B' */
    char get_type() const override;

    /** Test for equality */
    bool equals(const Object *other) const override;

    /** Returns the hashcode of the column. */
    size_t hash() const override;

    /** Returns a new copy of the current column. */
    std::shared_ptr<Object> clone() const override;

    /** Serializes the current column into a byte representation. */
    std::vector<uint8_t> serialize() const override;
};

/** Specialization of deserialize for BoolColumn
 *
 * Converts from the serialized form of this column to an object containing 
 * the data */
template<>
inline BoolColumn Serializable::deserialize<BoolColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<bool>::deserialize(data, pos);
    BoolColumn bc;
    for(size_t i = 0; i < na.size(); ++i){
        bc.push_back(na.get(i));
    }
    return bc;
}
 
/*************************************************************************
 * StringColumn::
 * Holds string pointers. The strings are external.  Nullptr is a valid
 * value.
 */
class StringColumn : public Column {
private:
    /** Internal data structure holding the data */
    NullableArray<std::string> _data;

public:
    StringColumn() = default;
    StringColumn(const StringColumn& other) = default;
    StringColumn(StringColumn&& other) = default;
    StringColumn(int n, ...);

    /** Pushes the given value onto the end
    * of the column. If the optional is null, then it marks 
    * that value as missing. **/
    void push_back(std::optional<std::string> val) override;

    /** Get the value at the given index. A null optional 
     * signifies a missing value. */
    std::optional<std::string> get(size_t idx);

    /** Get this as an string column. */
    StringColumn* as_string() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<std::string> set(size_t idx, std::optional<std::string> val);

    /** Returns the number of elements in the column. */
    size_t size() const override;

    /** Gets the type of the column. Returns 'S' */
    char get_type() const override;

    /** Test for equality */
    bool equals(const Object *other) const override;

    /** Returns the hashcode of the column. */
    size_t hash() const override;

    /** Returns a new copy of the current column. */
    std::shared_ptr<Object> clone() const override;

    /** Serializes the current column into a byte representation. */
    std::vector<uint8_t> serialize() const override;
};

/** Specialization of deserialize for StringColumn
 *
 * Converts from the serialized form of this column to an object containing 
 * the data */
template<>
inline StringColumn Serializable::deserialize<StringColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<std::string>::deserialize(data, pos);
    StringColumn sc;
    for(size_t i = 0; i < na.size(); ++i){
        sc.push_back(na.get(i));
    }
    return sc;
}
