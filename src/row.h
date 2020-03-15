#pragma once

#include <string>

#include "object.h"
#include "schema.h"
#include "fielder.h"

/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 */
class Row : public Object {
private:
    union Schema_Value {
        int i;
        float f;
        bool b;
        std::shared_ptr<std::string> *s;
    };

    size_t _width;
    char *_types;
    Schema_Value *_values;
    size_t _index;
    std::string _name;

public:
    /** Build a row following a schema. */
    Row(const Schema& scm);

    ~Row();

    /** Setters: set the given column with the given value. Setting a column with
    * a value of the wrong type is undefined. */
    void set(size_t col, int val);

    void set(size_t col, float val);

    void set(size_t col, bool val);

    /** The string is external. */
    void set(size_t col, std::shared_ptr<std::string> val);

    /** Set/get the index of this row (ie. its position in the dataframe. This is
    *  only used for informational purposes, unused otherwise */
    void set_index(size_t idx);

    size_t get_index() const;

    /** Getters: get the value at the given column. If the column is not
    * of the requested type, the result is undefined. */
    int get_int(size_t col) const;

    bool get_bool(size_t col) const;

    float get_float(size_t col) const;

    std::shared_ptr<std::string> get_string(size_t col) const;

    /** Number of fields in the row. */
    size_t width() const;

    /** Type of the field at the given position. An idx >= width is  undefined. */
    char col_type(size_t idx) const;

    /** Given a Fielder, visit every field of this row. The first argument is
    * index of the row in the dataframe.
    * Calling this method before the row's fields have been set is undefined. */
    void visit(size_t idx, Fielder& f) const;

    bool equals(const Object *other) const override;

    size_t hash() const override;

    Object *clone() const override;
};