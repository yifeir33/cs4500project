#pragma once

#include <vector>
#include <unordered_set>
#include <mutex>

#include "data/rower.h"
#include "data/dataframe.h"

class IntSetGenerator : public Rower {
private:
    std::shared_ptr<std::mutex> _mutex;
    std::shared_ptr<std::unordered_set<int>> _set;

public:
    IntSetGenerator();

    IntSetGenerator(std::shared_ptr<std::mutex> m, std::shared_ptr<std::unordered_set<int>> s);

    bool accept(Row& row) override;

    void join(std::shared_ptr<Rower> other) override;

    std::shared_ptr<std::unordered_set<int>> finish_set();

    size_t hash() const override;

    std::shared_ptr<Object> clone() const override;

    bool equals(const Object *other) const override;
};

class UnorderedFilter : public Rower {
protected:
    std::shared_ptr<std::mutex> _df_mutex;
    std::shared_ptr<DataFrame> _gen_df;
    std::shared_ptr<std::unordered_set<int>> _set; // read only
    Row _row;

    UnorderedFilter(std::unique_ptr<Schema> s);

    UnorderedFilter(std::shared_ptr<std::mutex> dfm, std::shared_ptr<DataFrame> df,
                    std::shared_ptr<std::unordered_set<int>> s);

    bool _set_contains(int v);

    void _add_to_df(int v);

    void _add_to_df(std::string v);

    bool _ptr_equality(const UnorderedFilter& other_uf) const;

public:
    std::shared_ptr<DataFrame> finish_filter();

    /** Once traversal of the data frame is complete the rowers that were
     split off will be joined.  There will be one join per split. The
     original object will be the last to be called join on. The join method
     is reponsible for cleaning up memory. */
    void join(std::shared_ptr<Rower> other) override;

    size_t hash() const override;
};

class UUIDsToProjectsFilter : public UnorderedFilter {
public:
    UUIDsToProjectsFilter(std::shared_ptr<DataFrame> uuid_df);

    UUIDsToProjectsFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                          std::shared_ptr<std::unordered_set<int>> uuids);

    /* the rower is taking the row that is going to be edited*/
    bool accept(Row& row) override;

    std::shared_ptr<Object> clone() const override;

    bool equals(const Object *other) const override;
};

class ProjectsToUUIDsFilter : public UnorderedFilter {
public:
    ProjectsToUUIDsFilter(std::shared_ptr<DataFrame> projects_df);

    ProjectsToUUIDsFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                          std::shared_ptr<std::unordered_set<int>> pids);

    /* the rower is taking the row that is going to be edited*/
    bool accept(Row& row) override;

    std::shared_ptr<Object> clone() const override;

    bool equals(const Object *other) const override;
};

class UUIDsToNamesFilter : public UnorderedFilter {
public:
    UUIDsToNamesFilter(std::shared_ptr<DataFrame> uuid_df);

    UUIDsToNamesFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                       std::shared_ptr<std::unordered_set<int>> uuids);

    /* the rower is taking the row that is going to be edited*/
    bool accept(Row& row) override;

    std::shared_ptr<Object> clone() const override;

    bool equals(const Object *other) const override;
};
