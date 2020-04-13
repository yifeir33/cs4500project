#include "util/linus_rowers.h"

// IntSetGenerator
IntSetGenerator::IntSetGenerator() : _mutex(std::make_shared<std::mutex>()),
_set(std::make_shared<std::unordered_set<int>>()) {}

IntSetGenerator::IntSetGenerator(std::shared_ptr<std::mutex> m, std::shared_ptr<std::unordered_set<int>> s) : _mutex(m), _set(s) {}

bool IntSetGenerator::accept(Row& row) {
    for(size_t c = 0; c < row.width(); ++c) {
        if(row.col_type(c) == 'I') {
            std::lock_guard<std::mutex> guard(*_mutex);
            _set->insert(*row.get_int(c));
        }
    }
    return true;
}

// we don't need to do anything
void IntSetGenerator::join([[maybe_unused]] std::shared_ptr<Rower> other) {}

std::shared_ptr<std::unordered_set<int>> IntSetGenerator::finish_set() {
    std::lock_guard<std::mutex> guard(*_mutex);
    assert(_set.use_count() == 1);
    auto r = _set;
    _set = std::make_shared<std::unordered_set<int>>();
    return r;
}

size_t IntSetGenerator::hash() const {
    return reinterpret_cast<size_t>(_mutex.get()) ^ reinterpret_cast<size_t>(_set.get());
}

bool IntSetGenerator::equals(const Object *other) const {
    auto other_isg = dynamic_cast<const IntSetGenerator *>(other);
    if(other_isg) {
        return _mutex == other_isg->_mutex && _set == other_isg->_set;
    }
    return false;
}

std::shared_ptr<Object> IntSetGenerator::clone() const {
    return std::make_shared<IntSetGenerator>(_mutex, _set);
}

// UnorderedFilter
UnorderedFilter::UnorderedFilter(std::unique_ptr<Schema> s) : _df_mutex(std::make_shared<std::mutex>()), _gen_df(std::make_shared<DataFrame>(std::move(s))), _set(std::make_shared<std::unordered_set<int>>()), _row(_gen_df->get_schema()) {}

UnorderedFilter::UnorderedFilter(std::shared_ptr<std::mutex> dfm,
                                 std::shared_ptr<DataFrame> df,
                                 std::shared_ptr<std::unordered_set<int>> s)
: _df_mutex(dfm), _gen_df(df), _set(s), _row(_gen_df->get_schema()) {}

bool UnorderedFilter::_set_contains(int v) {
    // read only so can avoid locking
    return _set->find(v) != _set->end();
}

void UnorderedFilter::_add_to_df(int v) {
    std::lock_guard<std::mutex> guard(*_df_mutex);
    assert(_row.col_type(0) == 'I');
    _row.set(0, std::optional<int>(v));
    _gen_df->add_row(_row);
}

void UnorderedFilter::_add_to_df(std::string v) {
    std::lock_guard<std::mutex> guard(*_df_mutex);
    assert(_row.col_type(0) == 'S');
    _row.set(0, std::optional<std::string>(v));
    _gen_df->add_row(_row);
}

std::shared_ptr<DataFrame> UnorderedFilter::finish_filter() {
    std::lock_guard<std::mutex> guard(*_df_mutex);
    assert(_gen_df.use_count() == 1); // must only have one reference to finish
    auto r = _gen_df;
    _gen_df = std::make_shared<DataFrame>(std::make_unique<Schema>(r->get_schema()));
    return r;
}

// we don't need to do anything
void UnorderedFilter::join([[maybe_unused]] std::shared_ptr<Rower> other) {}

size_t UnorderedFilter::hash() const {
    return reinterpret_cast<size_t>(_df_mutex.get()) 
            ^ reinterpret_cast<size_t>(_gen_df.get())
            ^ reinterpret_cast<size_t>(_set.get());
}

bool UnorderedFilter::_ptr_equality(const UnorderedFilter& other_uf) const{
    return _df_mutex == other_uf._df_mutex && _gen_df == other_uf._gen_df
           && _set == other_uf._set;
}

// UUIDsToProjectsFilter
UUIDsToProjectsFilter::UUIDsToProjectsFilter(std::shared_ptr<DataFrame> uuid_df) 
: UnorderedFilter(std::make_unique<Schema>("I")) {
    IntSetGenerator isg;
    uuid_df->pmap(isg);
    _set = isg.finish_set();
}

UUIDsToProjectsFilter::UUIDsToProjectsFilter(std::shared_ptr<std::mutex> m,
                                             std::shared_ptr<DataFrame> df,
                                             std::shared_ptr<std::unordered_set<int>> uuids)
: UnorderedFilter(m, df, uuids) {}

bool UUIDsToProjectsFilter::accept(Row& r) {
    // this works on the commit dataframe
    if(this->_set_contains(*r.get_int(1)) || this->_set_contains(*r.get_int(2))) {
        this->_add_to_df(*r.get_int(0));
    }
    return true;
}

std::shared_ptr<Object> UUIDsToProjectsFilter::clone() const {
    return std::make_shared<UUIDsToProjectsFilter>(_df_mutex, _gen_df, _set);
}

bool UUIDsToProjectsFilter::equals(const Object *other) const {
    auto other_pf = dynamic_cast<const UUIDsToProjectsFilter *>(other);
    if(other_pf) return _ptr_equality(*other_pf);
    return false;
}

// ProjectsToUUIDsFilter
ProjectsToUUIDsFilter::ProjectsToUUIDsFilter(std::shared_ptr<DataFrame> projects_df) : UnorderedFilter(std::make_unique<Schema>("I")) {
    IntSetGenerator isg;
    projects_df->pmap(isg);
    _set = isg.finish_set();
}

ProjectsToUUIDsFilter::ProjectsToUUIDsFilter(std::shared_ptr<std::mutex> m,
                                             std::shared_ptr<DataFrame> df,
                                             std::shared_ptr<std::unordered_set<int>> pids)
: UnorderedFilter(m, df, pids) {}

bool ProjectsToUUIDsFilter::accept(Row& r) {
    // works on commit dataframe
    if(this->_set_contains(*r.get_int(0))) {
        this->_add_to_df(*r.get_int(1));
        this->_add_to_df(*r.get_int(2));
    }
    return true;
}

std::shared_ptr<Object> ProjectsToUUIDsFilter::clone() const {
    return std::make_shared<ProjectsToUUIDsFilter>(_df_mutex, _gen_df, _set);
}

bool ProjectsToUUIDsFilter::equals(const Object *other) const {
    auto other_pf = dynamic_cast<const ProjectsToUUIDsFilter *>(other);
    if(other_pf) return _ptr_equality(*other_pf);
    return false;
}

// UUIDsToNamesFilter
UUIDsToNamesFilter::UUIDsToNamesFilter(std::shared_ptr<DataFrame> uuid_df) 
    : UnorderedFilter(std::make_unique<Schema>("S")) {
    IntSetGenerator isg;
    uuid_df->pmap(isg);
    _set = isg.finish_set();
}

UUIDsToNamesFilter::UUIDsToNamesFilter(std::shared_ptr<std::mutex> m, std::shared_ptr<DataFrame> df,
                                        std::shared_ptr<std::unordered_set<int>> uuids)
: UnorderedFilter(m, df, uuids) {}

bool UUIDsToNamesFilter::accept(Row& row) {
    // works on users dataframe
    if(this->_set_contains(*row.get_int(0))) {
        this->_add_to_df(*row.get_string(1));
    }
    return true;
}

std::shared_ptr<Object> UUIDsToNamesFilter::clone() const {
    return std::make_shared<UUIDsToNamesFilter>(_df_mutex, _gen_df, _set);
}

bool UUIDsToNamesFilter::equals(const Object *other) const {
    auto other_uutnf = dynamic_cast<const UUIDsToNamesFilter *>(other);
    if(other_uutnf) return this->_ptr_equality(*other_uutnf);
    return false;
}
