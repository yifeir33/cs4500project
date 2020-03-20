#pragma once

#include <memory>
#include <string>
#include <thread>

#include "util/object.h"
#include "data/schema.h"
#include "data/rower.h"
#include "data/fielder.h"
#include "data/column.h"
#include "store/kvstore.h"

#define MAX_THREADS    8
#define THREAD_ROWS    5000

/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 */
class DataFrame : public Object {
private:
    std::shared_ptr<Schema> _schema;
    std::vector<std::unique_ptr<Column>> _columns;

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
    void _pmap_helper(size_t row_start, size_t row_end, Rower *rower) const;

    /* Helper Rower to print the dataframe */
    class PrintRower : public Rower {
    private:
        /* Helper Fielder to print the dataframe */
        class PrintFielder : public Fielder {
        public:
            /** Called for fields of the argument's type with the value of the field. */
            void accept(bool b) override;

            void accept(double f) override;

            void accept(int i) override;

            void accept(std::weak_ptr<std::string> s) override;

            /** Called when all fields have been seen. */
            void done() override;

            size_t hash() const override;

            bool equals(const Object *other) const override;

            Object *clone() const override;
        };

        PrintFielder pf;
    public:
        PrintRower() = default;

        bool accept(Row& r) override;

        void join_delete(Rower* other) override;

        size_t hash() const override;

        bool equals(const Object *other) const override;
        
        Object *clone() const override;
    };

public:
    static std::shared_ptr<DataFrame> from_array(KVStore& kvs, KVStore::Key k, int *arr, size_t arr_len);
    static std::shared_ptr<DataFrame> from_array(KVStore& kvs, KVStore::Key k, double *arr, size_t arr_len);
    static std::shared_ptr<DataFrame> from_array(KVStore& kvs, KVStore::Key k, bool *arr, size_t arr_len);
    // TODO: string

    // Creates a dataframe with an empty schema
    DataFrame();

    /** Create a data frame with the same columns as the given df but with no rows or rownames */
    DataFrame(const DataFrame& df);

    /** Create a data frame from a schema and columns. All columns are created
    * empty. */
    DataFrame(std::shared_ptr<Schema> schema);

    virtual ~DataFrame();

    /** Returns the dataframe's schema. Modifying the schema after a dataframe
    * has been created in undefined. */
    const Schema& get_schema() const;

    /** Adds a column this dataframe, updates the schema, the new column
    * is external, and appears as the last column of the dataframe, the
    * name is optional and external. A nullptr colum is undefined. */
    void add_column(std::unique_ptr<Column> col, std::shared_ptr<std::string> name);

    /** Return the value at the given column and row. Accessing rows or
    *  columns out of bounds, or request the wrong type is undefined.*/
    int get_int(size_t col, size_t row) const;

    bool get_bool(size_t col, size_t row) const;

    double get_double(size_t col, size_t row) const;

    std::weak_ptr<std::string> get_string(size_t col, size_t row) const;

    /** Return the offset of the given column name or -1 if no such col. */
    int get_col(std::string& col) const;

    /** Return the offset of the given row name or -1 if no such row. */
    int get_row(std::string& row) const;

    /** Set the value at the given column and row to the given value.
    * If the column is not  of the right type or the indices are out of
    * bound, the result is undefined. */
    void set(size_t col, size_t row, int val);

    void set(size_t col, size_t row, bool val);

    void set(size_t col, size_t row, double val);

    void set(size_t col, size_t row, std::shared_ptr<std::string> val);

    /** Set the fields of the given row object with values from the columns at
    * the given offset.  If the row is not form the same schema as the
    * dataframe, results are undefined.
    */
    void fill_row(size_t idx, Row& row) const;

    /** Add a row at the end of this dataframe. The row is expected to have
    *  the right schema and be filled with values, otherwise undedined.  */
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
    DataFrame* filter(Rower& r) const;

    /** Print the dataframe in SoR format to standard output. */
    void print() const;

    bool equals(const Object* other) const override;

    Object* clone() const override;

    size_t hash() const override;

    bool operator==(const DataFrame& other) const;
};
