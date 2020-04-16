#pragma once

#include <vector>
#include <unordered_set>
#include <mutex>

#include "data/rower.h"
#include "data/dataframe.h"

/** Rower that operates on a dataframe containing integers,
 * and adds all integers in that dataframe to a set (preventing 
 * duplicates), and returns said set when finish_set() is called. */
class IntSetGenerator : public Rower {
private:
    std::shared_ptr<std::mutex> _mutex;
    std::shared_ptr<std::unordered_set<int>> _set;

public:
    /** Constructor. Creates new mutex and set. */
    IntSetGenerator();

    /** Constructor for clone. Takes the pointer to the mutex and set
     * so that all instances created through clone are adding to the same 
     * set. */
    IntSetGenerator(std::shared_ptr<std::mutex> m, std::shared_ptr<std::unordered_set<int>> s);

    /** Override Rower.accept() method parsing a row. */
    bool accept(Row& row) override;

    /** Combines rowers at the end. In this case does nothing. */
    void join(std::shared_ptr<Rower> other) override;

    /** Returns the set of integers constructed by the IntSetGenerator */
    std::shared_ptr<std::unordered_set<int>> finish_set();

    /** Returns the hashcode of this object. */
    size_t hash() const override;

    /** Returns a new instance of the IntSetGenerator that adds to the
     * same set as this one. */
    std::shared_ptr<Object> clone() const override;

    /** Compares for pointer equality of the mutex and set. */
    bool equals(const Object *other) const override;
};


/** Abstract class that creates a dataframe of values from the
 * given set of values. */
class UnorderedFilter : public Rower {
protected:
    /** Mutex protecting the _gen_df we are constructing. */
    std::shared_ptr<std::mutex> _df_mutex;
    /** The dataframe we are constructing. */
    std::shared_ptr<DataFrame> _gen_df;
    /** The set of integers we are looking in. */
    std::shared_ptr<std::unordered_set<int>> _set; // read only
    /** A local row object to decrease the number of times
     * a row is constructed for add_row. */
    Row _row;

    /** Constructs the the filter from the schema of the dataframe, constructing
     * new mutex and dataframe. */
    UnorderedFilter(std::unique_ptr<Schema> s);

    /** Constructs a new filter pointing to the the same mutex and dataframe
     * so that they construct the dataframe in parallel. */
    UnorderedFilter(std::shared_ptr<std::mutex> dfm, std::shared_ptr<DataFrame> df,
                    std::shared_ptr<std::unordered_set<int>> s);

    /** Returns true if the set of integers contains the given value, false
     * otherwise. */
    bool _set_contains(int v);

    /** Adds the integer to the dataframe. */
    void _add_to_df(int v);

    /** Adds the given string to the dataframe. */
    void _add_to_df(std::string v);

    /** Compares the internal mutex, dataframe, and set for pointer
     * equality. */
    bool _ptr_equality(const UnorderedFilter& other_uf) const;

public:
    /** Returns the constructed dataframe. */
    std::shared_ptr<DataFrame> finish_filter();

    /** Once traversal of the data frame is complete the rowers that were
     split off will be joined.  There will be one join per split. The
     original object will be the last to be called join on. The join method
     is reponsible for cleaning up memory. In this case, does
     nothing. */
    void join(std::shared_ptr<Rower> other) override;

    /** Returns the hashcode of this object. */
    size_t hash() const override;
};

/** Given a dataframe of user ids stores a set of those ids. When mapped over
 * the commits dataframe constructs a unordered dataframe containing the PIDs
 * that those users have worked on. */
class UUIDsToProjectsFilter : public UnorderedFilter {
public:
    /* Constructor that creates new mutex and dataframe, and generates a set of
     * UUIDs from the given dataframe. */
    UUIDsToProjectsFilter(std::shared_ptr<DataFrame> uuid_df);

    /** Constructor that passes the pointer to the internal mutex, dataframe
     * we are constructing, and set. */
    UUIDsToProjectsFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                          std::shared_ptr<std::unordered_set<int>> uuids);

    /** the rower is taking the row that is going to be parse, 
     * and if the uuid of the writer or commiter is in the set,
     * adds the project to the dataframe we are constructing. */
    bool accept(Row& row) override;

    /** Returns a new object constructing the same dataframe. */ 
    std::shared_ptr<Object> clone() const override;

    /** Compares to ensure they are the same class and reference the
     * same dataframe, mutex, and set. */
    bool equals(const Object *other) const override;
};

/** Given a dataframe of pids, stores a set of those ids. When mapped over the
 * commits dataframe, constructs an unordered dataframe containing the UUIDs of
 * users who have worked on those projects. */
class ProjectsToUUIDsFilter : public UnorderedFilter {
public:
    /* Constructor that creates new mutex and dataframe, and generates a set of
     * PIDs from the given dataframe. */
    ProjectsToUUIDsFilter(std::shared_ptr<DataFrame> projects_df);

    /** Constructor that passes the pointer to the internal mutex, dataframe
     * we are constructing, and set. */
    ProjectsToUUIDsFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                          std::shared_ptr<std::unordered_set<int>> pids);

    /** the rower is taking the row that is going to be parsed, 
     * and if the pid of the project is in the set, then
     * adds the users who committed to or wrote the commit to the
     * dataframe we are constructing. */
    bool accept(Row& row) override;

    /** Returns a new object constructing the same dataframe. */ 
    std::shared_ptr<Object> clone() const override;

    /** Compares to ensure they are the same class and reference the
     * same dataframe, mutex, and set. */
    bool equals(const Object *other) const override;
};

/** Given a dataframe of uuids, stores a set of those ids. When mapped over the
 * users dataframe, constructs an unordered dataframe containing the user names
 * of the users in the set. */
class UUIDsToNamesFilter : public UnorderedFilter {
public:
    /* Constructor that creates new mutex and dataframe, and generates a set of
     * Usernames from the given dataframe. */
    UUIDsToNamesFilter(std::shared_ptr<DataFrame> uuid_df);

    /** Constructor that passes the pointer to the internal mutex, dataframe
     * we are constructing, and set. */
    UUIDsToNamesFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                       std::shared_ptr<std::unordered_set<int>> uuids);

    /** the rower is taking the row that is going to be parsed, 
     * and if the uuid is in the set, adds the username to the dataframe we
     * are constructing. */
    bool accept(Row& row) override;

    /** Returns a new object constructing the same dataframe. */ 
    std::shared_ptr<Object> clone() const override;

    /** Compares to ensure they are the same class and reference the
     * same dataframe, mutex, and set. */
    bool equals(const Object *other) const override;
};
