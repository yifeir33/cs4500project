#pragma once

#include <memory>
#include <string>
#include <thread>
#include <optional>

#include "util/serializable.h"
#include "data/schema.h"
#include "data/rower.h"
#include "data/fielder.h"
#include "data/column.h"
#include "data/kvstore.h"

#define MAX_THREADS    8
#define THREAD_ROWS    5000


/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 */
class DataFrame : public Serializable {
private:
    /** Stores a schema representing the both the number and types of the columns
     * internally, and the names of the rows and columns */
    std::unique_ptr<Schema> _schema;
    /* A list of columns which store the data */
    std::vector<std::unique_ptr<Column>> _columns;

    /* Given a type (I, S, B, or F), returns a new column from that type. 
     * Used as a helper method when constructing or adding columns */
    std::unique_ptr<Column> _get_col_from_type(char type) const;

    /** This method is used to execute a map in parallel. The row_start
     * is the starting row for the rower in this thread, the row_end is
     * the non-inclusive end row for the rower in this thread. This method
     * should be passed to a thread as the method it is executing.
     *
     * If the rower is mutating shared objects or data, it must 
     * do that in a thread safe manner internally, or reults are
     * undefined behavior.
     */
    void _pmap_helper(size_t row_start, size_t row_end, Rower& rower) const;

    /* Helper Rower to print the dataframe */
    class PrintRower : public Rower {
    private:
        /* Helper Fielder to print the dataframe */
        class PrintFielder : public Fielder {
        public:
            /** Called for fields of the argument's type with the value of the field. */
            void accept(std::optional<bool> b) override;

            void accept(std::optional<double> f) override;

            void accept(std::optional<int> i) override;

            void accept(std::optional<std::string> s) override;

            /** Called when all fields have been seen. */
            void done() override;

            size_t hash() const override;

            bool equals(const Object *other) const override;

            std::shared_ptr<Object> clone() const override;
        };

        PrintFielder pf;
    public:
        PrintRower() = default;

        bool accept(Row& r) override;

        void join(std::shared_ptr<Rower> other) override;

        size_t hash() const override;

        bool equals(const Object *other) const override;
        
        std::shared_ptr<Object> clone() const override;
    };

public:
    /** Given an array of integers, constructs a single-column dataframe from it,
     * and stores it in the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_array(KVStore::Key k, int *arr, size_t arr_len);

    /** Given an array of floating point values, constructs a single-column dataframe from it,
     * and stores it in the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_array(KVStore::Key k, double *arr, size_t arr_len);

    /** Given an array of boolean values, constructs a single-column dataframe from it,
     * and stores it in the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_array(KVStore::Key k, bool *arr, size_t arr_len);

    /** Given a integer value, constructs a single-column with a single element dataframe
     * from it, and stores it int the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_scalar(KVStore::Key k, int val);

    /** Given a floating point value, constructs a single-column with a single element dataframe
     * from it, and stores it int the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_scalar(KVStore::Key k, double val);

    /** Given a boolean value, constructs a single-column with a single element dataframe
     * from it, and stores it int the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_scalar(KVStore::Key k, bool val);

    /** Given a string, constructs a single-column with a single element dataframe
     * from it, and stores it int the KVStore as the value associated with the given key. */
    static std::shared_ptr<DataFrame> from_scalar(KVStore::Key k, std::string val);

    // Creates a dataframe with an empty schema
    DataFrame();

    /** Create a data frame with the same columns as the given df but with no rows or rownames */
    DataFrame(const DataFrame& df);

    /** Create a data frame from a schema and columns. All columns are created
    * empty. */
    DataFrame(std::unique_ptr<Schema> schema);

    /** Default move constructor. */
    DataFrame(DataFrame&& other) = default;

    virtual ~DataFrame();

    /** Returns the dataframe's schema. Modifying the schema after a dataframe
    * has been created in undefined. */
    const Schema& get_schema() const;

    /** Adds a column this dataframe, updates the schema, the new column
    * is owned by the dataframe, and appears as the last column of the dataframe, the
    * name is optional and external. A nullptr colum is undefined. */
    void add_column(std::unique_ptr<Column> col, std::optional<std::string> name = std::nullopt);

    /** Return the value at the given column and row. Accessing rows or
    *  columns out of bounds, or request the wrong type is undefined.*/
    std::optional<int> get_int(size_t col, size_t row) const;

    std::optional<bool> get_bool(size_t col, size_t row) const;

    std::optional<double> get_double(size_t col, size_t row) const;

    std::optional<std::string> get_string(size_t col, size_t row) const;

    /** Return the offset of the given column name or -1 if no such col. */
    int get_col(const std::string& col) const;

    /** Return the offset of the given row name or -1 if no such row. */
    int get_row(const std::string& row) const;

    /** Set the value at the given column and row to the given value.
    * If the column is not  of the right type or the indices are out of
    * bound, the result is undefined. */
    void set(size_t col, size_t row, std::optional<int> val);

    void set(size_t col, size_t row, std::optional<bool> val);

    void set(size_t col, size_t row, std::optional<double> val);

    void set(size_t col, size_t row, std::optional<std::string> val);

    /** Set the fields of the given row object with values from the columns at
    * the given offset.  If the row is not using the same schema as the
    * dataframe, results are undefined.
    */
    void fill_row(size_t idx, Row& row) const;

    /** Add a row at the end of this dataframe. The row is expected to have
    *  the right schema and be filled with values, otherwise undefined behavior
    *  occurs.  */
    void add_row(Row& row);

    /** The number of rows in the dataframe. */
    size_t nrows() const;

    /** The number of columns in the dataframe.*/
    size_t ncols() const;

    /** Visit rows in order */
    void map(Rower& r) const;

    /** This method clones the Rower and executes the map in parallel. Join is
      * used at the end to merge the results. */
    void pmap(Rower& r) const;

    /** Create a new dataframe, constructed from rows for which the given Rower
    * returned true from its accept method. */
    std::shared_ptr<DataFrame> filter(Rower& r) const;

    /** Print the dataframe in SoR format to standard output. */
    void print() const;

    /** Test for equality. */
    bool equals(const Object* other) const override;

    /** Create a copy with the same schema, but all columns are empty and 
     * all names are dropped. */
    std::shared_ptr<Object> clone() const override;

    /** Returns the hashcode for this dataframe. */
    size_t hash() const override;

    /** Overload the equality operator. Tests for equality. */
    bool operator==(const DataFrame& other) const;

    /** Serialize the dataframe into byte format. Drops row and column names. */
    std::vector<uint8_t> serialize() const override;
};

/** Specialization of deserialize for DataFrames. 
 * 
 * Converts the serialized version of a datframe into an object with the approriate
 * data. All row and column names are dropped. */
template<>
inline DataFrame Serializable::deserialize<DataFrame>(const std::vector<uint8_t>& data, size_t& pos) {
    Schema schema = Serializable::deserialize<Schema>(data, pos);
    size_t col_cnt = Serializable::deserialize<size_t>(data, pos);
    assert(col_cnt == schema.width());
    DataFrame df;
    for(size_t i = 0; i < col_cnt; ++i){
        std::unique_ptr<Column> col = nullptr;
        switch(schema.col_type(i)){
            case 'I':
                col = std::make_unique<IntColumn>(Serializable::deserialize<IntColumn>(data, pos));
                break;
            case 'F':
                col = std::make_unique<FloatColumn>(Serializable::deserialize<FloatColumn>(data, pos));
                break;
            case 'B':
                col = std::make_unique<BoolColumn>(Serializable::deserialize<BoolColumn>(data, pos));
                break;
            case 'S':
                col = std::make_unique<StringColumn>(Serializable::deserialize<StringColumn>(data, pos));
                break;
        }
        assert(col);
        df.add_column(std::move(col));
    }
    return df;
}
