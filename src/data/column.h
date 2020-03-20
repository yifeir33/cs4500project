#pragma once

#include <stdarg.h>
#include <assert.h>
#include <string>
#include <memory>
#include <vector>

#include "util/object.h"

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
class Column : public Object {
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
    virtual void push_back(int val);
    virtual void push_back(bool val);
    virtual void push_back(double val);
    virtual void push_back(std::shared_ptr<std::string> val);

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
    std::vector<int> _data;

public:
    IntColumn() = default;
    IntColumn(int n, ...);

    void push_back(int val) override;

    int get(size_t idx) const;

    IntColumn* as_int() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    int set(size_t idx, int val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;
    
    Object *clone() const override;
};
 
/*************************************************************************
 * FloatColumn::
 * Holds floating point values.
 */
class FloatColumn : public Column {
private:
    std::vector<double> _data;

public:
    FloatColumn() = default;
    FloatColumn(int n, ...);

    void push_back(double val) override;

    double get(size_t idx) const;

    FloatColumn* as_float() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    double set(size_t idx, double val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    Object *clone() const override;
};

/*************************************************************************
 * BoolColumn::
 * Holds boolean values.
 */
class BoolColumn : public Column {
private:
    std::vector<bool> _data;

public:
    BoolColumn() = default;
    BoolColumn(int n, ...);

    void push_back(bool val) override;

    bool get(size_t idx) const;

    BoolColumn* as_bool() override;
    /** Set value at idx. An out of bound idx is undefined.  */
    bool set(size_t idx, bool val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    Object *clone() const override;
};
 
/*************************************************************************
 * StringColumn::
 * Holds string pointers. The strings are external.  Nullptr is a valid
 * value.
 */
class StringColumn : public Column {
private:
    std::vector<std::shared_ptr<std::string>> _data;

public:
    StringColumn() = default;
    StringColumn(int n, ...);

    void push_back(std::shared_ptr<std::string> val) override;

    std::weak_ptr<std::string> get(size_t idx);

    StringColumn* as_string() override;

    /** Set value at idx. An out of bound idx is undefined.  */
    std::shared_ptr<std::string> set(size_t idx, std::shared_ptr<std::string> val);

    size_t size() const override;

    char get_type() const override;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    Object *clone() const override;
};
