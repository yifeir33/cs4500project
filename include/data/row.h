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
 * Edits to a row's data DO NOT affect the data in the dataframe that it was
 * constructed from. That is the data is copy constructed in the row internally.
 */
class Row : public Object {
private:
    // Convience typedef to shorten the very long type of the variant.
    using schema_variant = std::variant<std::optional<int>, std::optional<double>, std::optional<bool>, std::optional<std::string>>;

    // The width of the row.
    size_t _width;
    // The types of the columns
    std::string _types;
    // A list of the copy constructed values.
    std::vector<schema_variant> _values;
    // The index of this row in the dataframe it was constructed from. If the row
    // was constructed externally, this may not be accurate.
    size_t _index;

public:
    /** Build a row following a schema. */
    Row(const Schema& scm);

    /** Setters: set the given column with the given value. Setting a column with
    * a value of the wrong type is undefined. */
    void set(size_t col, std::optional<int> val);

    void set(size_t col, std::optional<double> val);

    void set(size_t col, std::optional<bool> val);

    void set(size_t col, std::optional<std::string> val);

    /** Set/get the index of this row (ie. its position in the dataframe. This is
    *  only used for informational purposes, unused otherwise */
    void set_index(size_t idx);

    /* Get the index this row claims to be. If the row was constructed inside
     * the dataframe, it can be assumed to have an accurate index. */
    size_t get_index() const;

    /** Getters: get the value at the given column. If the column is not
    * of the requested type, the result is undefined. A nullopt represents
    * a missing value in the data. */
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

    /** Tests for equality. */
    bool equals(const Object *other) const override;

    /** Returns the hashcode of the row. */
    size_t hash() const override;

    /** Returns a copy of the row object, using the same schema. */
    std::shared_ptr<Object> clone() const override;
};
