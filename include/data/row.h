#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

#include "util/object.h"
#include "data/schema.h"
#include "data/fielder.h"

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
    using schema_variant = std::variant<std::optional<int>, std::optional<double>, std::optional<bool>, std::optional<std::string>>;

    size_t _width;
    char *_types;
    std::vector<schema_variant> _values;
    size_t _index;
    std::string _name;

public:
    /** Build a row following a schema. */
    Row(const Schema& scm);

    ~Row();

    /** Setters: set the given column with the given value. Setting a column with
    * a value of the wrong type is undefined. */
    void set(size_t col, std::optional<int> val);

    void set(size_t col, std::optional<double> val);

    void set(size_t col, std::optional<bool> val);

    /** The string is external. */
    void set(size_t col, std::optional<std::string> val);

    /** Set/get the index of this row (ie. its position in the dataframe. This is
    *  only used for informational purposes, unused otherwise */
    void set_index(size_t idx);

    size_t get_index() const;

    /** Getters: get the value at the given column. If the column is not
    * of the requested type, the result is undefined. */
    std::optional<int> get_int(size_t col) const;

    std::optional<bool> get_bool(size_t col) const;

    std::optional<double> get_double(size_t col) const;

    std::optional<std::string> get_string(size_t col) const;

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

    std::shared_ptr<Object> clone() const override;
};
