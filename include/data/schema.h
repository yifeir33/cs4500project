#pragma once

#include <optional>
#include <assert.h>
#include <vector>
#include <string>

#include "util/serializable.h"
#include "data/column.h"


/*************************************************************************
 * Schema::
 * A schema is a description of the contents of a data frame, the schema
 * knows the number of columns and number of rows, the type of each column,
 * optionally columns and rows can be named by strings.
 * The valid types are represented by the chars 'S', 'B', 'I' and 'F'.
 */
class Schema : public Serializable {
private:
    /** A list of column names. */
    std::vector<std::optional<std::string>> _columnNames;
    /** A list of row names. */
    std::vector<std::optional<std::string>> _rowNames;
    /** A list of characters representing the types of the columns. */
    std::vector<char> _columnTypes;
    /** The width of the schema. */
    size_t _width;
    /** The number of rows in the schema. */
    size_t _length;

public:
    /** Copy constructor */
    Schema(const Schema& from);

    /** Create an empty schema **/
    Schema();

    /** Create a schema from a string of types. A string that contains
    * characters other than those identifying the four type results in
    * undefined behavior. The argument is external, a nullptr argument is
    * undefined. **/
    Schema(const std::string& types);
    Schema(const char* types);

    /**
     * Move constructor so that serializing values, and
     * constructing a pointer from the stack allocated class is
     * more efficient
     */
    Schema(Schema&& other) = default;

    /** Add a column of the given type and name, name
    * is external. Names are expectd to be unique, duplicates result
    * in undefined behavior if they are not nullopt. 
    * The default arguement for the name is for the column to have no name. */
    void add_column(char typ, std::optional<std::string> name = std::nullopt);

    /** Add a row with a name, name is external.  Names are
    *  expectd to be unique, duplicates result in undefined behavior if they
    *  are not nullopt. The default argument for the name is for the row to have
    *  no name. */
    void add_row(std::optional<std::string> name = std::nullopt);

    /** Return name of row at idx; nullopt indicates no name. An idx >= width
    * is undefined. */
    std::optional<std::string> row_name(size_t idx) const;

    /** Return name of column at idx; nullopt indicates no name given.
    *  An idx >= width is undefined.*/
    std::optional<std::string> col_name(size_t idx) const;

    /** Return type of column at idx. An idx >= width is undefined. */
    char col_type(size_t idx) const;

    /** Given a column name return its index, or -1. */
    int col_idx(const std::string& name) const;

    /** Given a row name return its index, or -1. */
    int row_idx(const std::string& name) const;

    /** The number of columns */
    size_t width() const;

    /** The number of rows */
    size_t length() const;

    /** Create a copy of the schema. */
    std::shared_ptr<Object> clone() const override;

    /** Test for equality. */
    bool equals(const Object* other) const override;

    /** Get the hashcode of the schema. */
    size_t hash() const override;

    /** Serialize the schema into byte format. Drops row and column 
     * names. */
    std::vector<uint8_t> serialize() const override;

};

/**Sspecialization of deserialize for Schema
 *
 * Given a serialized schema, constructs a Schema object containing the appropriate
 * data from the serialized schema. Row and column names are dropped. */
template<>
inline Schema Serializable::deserialize<Schema>(const std::vector<uint8_t>& data, size_t& pos) {
    std::string s;
    size_t width = Serializable::deserialize<size_t>(data, pos);
    for(size_t i = 0; i < width; ++i) {
        s += Serializable::deserialize<char>(data, pos);
    }
    return Schema(s);
}

