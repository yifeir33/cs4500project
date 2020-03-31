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
 * equality. */
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
    * undefined behavior. **/
    virtual void push_back(std::optional<int> val);
    virtual void push_back(std::optional<bool> val);
    virtual void push_back(std::optional<double> val);
    virtual void push_back(std::optional<std::string> val);

    /** Returns the number of elements in the column. */
    virtual size_t size() const = 0;

    /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. */
    virtual char get_type() const = 0;

    size_t hash() const override;
};
 
/*************************************************************************
 * IntColumn::
 * Holds int values.
 */
class IntColumn : public Column {
private:
    NullableArray<int> _data;
    /* std::vector<std::optional<int>> _data; */

public:
    IntColumn() = default;
    IntColumn(const IntColumn& other) = default;
    IntColumn(IntColumn&& other) = default;
    IntColumn(int n, ...);

    void push_back(std::optional<int> val) override;

    std::optional<int> get(size_t idx) const;

    IntColumn* as_int() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<int> set(size_t idx, std::optional<int> val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;
    
    std::shared_ptr<Object> clone() const override;

    std::vector<uint8_t> serialize() const override;
};

// specialization of deserialize
template<>
inline IntColumn Serializable::deserialize<IntColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<int>::deserialize(data, pos);
    IntColumn ic;
    for(size_t i = 0; i < na.size(); ++i){
        ic.push_back(na.get(i));
    }
    return ic;
    /* IntColumn ic; */
    /* size_t len = Serializable::deserialize<size_t>(data, pos); */
    /* for(size_t i = 0; i < len; ++i) { */
    /*     bool exists = Serializable::deserialize<bool>(data, pos); */
    /*     if(exists) { */
    /*         ic.push_back(Serializable::deserialize<int>(data, pos)); */
    /*     } else { */
    /*         ic.push_back(std::nullopt); */
    /*     } */
    /* } */
    /* return ic; */
}
 
/*************************************************************************
 * FloatColumn::
 * Holds floating point values.
 */
class FloatColumn : public Column {
private:
    NullableArray<double> _data;
    /* std::vector<std::optional<double>> _data; */

public:
    FloatColumn() = default;
    FloatColumn(const FloatColumn& other) = default;
    FloatColumn(FloatColumn&& other) = default;
    FloatColumn(int n, ...);

    void push_back(std::optional<double> val) override;

    std::optional<double> get(size_t idx) const;

    FloatColumn* as_float() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<double> set(size_t idx, std::optional<double> val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    std::shared_ptr<Object> clone() const override;

    std::vector<uint8_t> serialize() const override;
};

// specialization of deserialize
template<>
inline FloatColumn Serializable::deserialize<FloatColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<double>::deserialize(data, pos);
    FloatColumn fc;
    for(size_t i = 0; i < na.size(); ++i){
        fc.push_back(na.get(i));
    }
    return fc;
    /* FloatColumn fc; */
    /* size_t len = Serializable::deserialize<size_t>(data, pos); */
    /* for(size_t i = 0; i < len; ++i) { */
    /*     bool exists = Serializable::deserialize<bool>(data, pos); */
    /*     if(exists) { */
    /*         fc.push_back(Serializable::deserialize<double>(data, pos)); */
    /*     } else { */
    /*         fc.push_back(std::nullopt); */
    /*     } */
    /* } */
    /* return fc; */
}

/*************************************************************************
 * BoolColumn::
 * Holds boolean values.
 */
class BoolColumn : public Column {
private:
    NullableArray<bool> _data;
    /* std::vector<std::optional<bool>> _data; */

public:
    BoolColumn() = default;
    BoolColumn(const BoolColumn& other) = default;
    BoolColumn(BoolColumn&& other) = default;
    BoolColumn(int n, ...);

    void push_back(std::optional<bool> val) override;

    std::optional<bool> get(size_t idx) const;

    BoolColumn* as_bool() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<bool> set(size_t idx, std::optional<bool> val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    std::shared_ptr<Object> clone() const override;

    std::vector<uint8_t> serialize() const override;
};

// specialization of deserialize
template<>
inline BoolColumn Serializable::deserialize<BoolColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<bool>::deserialize(data, pos);
    BoolColumn bc;
    for(size_t i = 0; i < na.size(); ++i){
        bc.push_back(na.get(i));
    }
    return bc;
    /* BoolColumn bc; */
    /* size_t len = Serializable::deserialize<size_t>(data, pos); */
    /* for(size_t i = 0; i < len; ++i) { */
    /*     bool exists = Serializable::deserialize<bool>(data, pos); */
    /*     if(exists) { */
    /*         bc.push_back(Serializable::deserialize<bool>(data, pos)); */
    /*     } else { */
    /*         bc.push_back(std::nullopt); */
    /*     } */
    /* } */
    /* return bc; */
}
 
/*************************************************************************
 * StringColumn::
 * Holds string pointers. The strings are external.  Nullptr is a valid
 * value.
 */
class StringColumn : public Column {
private:
    NullableArray<std::string> _data;
    /* std::vector<std::optional<std::string>> _data; */

public:
    StringColumn() = default;
    StringColumn(const StringColumn& other) = default;
    StringColumn(StringColumn&& other) = default;
    StringColumn(int n, ...);

    void push_back(std::optional<std::string> val) override;

    std::optional<std::string> get(size_t idx);

    StringColumn* as_string() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::optional<std::string> set(size_t idx, std::optional<std::string> val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    std::shared_ptr<Object> clone() const override;

    std::vector<uint8_t> serialize() const override;
};

// specialization of deserialize
template<>
inline StringColumn Serializable::deserialize<StringColumn>(const std::vector<uint8_t>& data, size_t& pos) {
    auto na = NullableArray<std::string>::deserialize(data, pos);
    StringColumn sc;
    for(size_t i = 0; i < na.size(); ++i){
        sc.push_back(na.get(i));
    }
    return sc;
    /* StringColumn sc; */
    /* size_t len = Serializable::deserialize<size_t>(data, pos); */
    /* for(size_t i = 0; i < len; ++i) { */
    /*     bool exists = Serializable::deserialize<bool>(data, pos); */
    /*     if(exists) { */
    /*         sc.push_back(Serializable::deserialize<std::string>(data, pos)); */
    /*     } else { */
    /*         sc.push_back(std::nullopt); */
    /*     } */
    /* } */
    /* return sc; */
}
