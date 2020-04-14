#include <cstring>

#include "data/dataframe.h"

// static functions
std::shared_ptr<DataFrame> DataFrame::from_array(KVStore::Key k, bool *arr, size_t arr_len) {
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<BoolColumn>();
    assert(col);
    for(size_t i = 0; i < arr_len; ++i) {
        col->push_back(arr[i]);
    }
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

std::shared_ptr<DataFrame> DataFrame::from_array(KVStore::Key k, double *arr, size_t arr_len) {
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<FloatColumn>();
    assert(col);
    for(size_t i = 0; i < arr_len; ++i) {
        col->push_back(arr[i]);
    }
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

std::shared_ptr<DataFrame> DataFrame::from_array(KVStore::Key k, int *arr, size_t arr_len) {
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<IntColumn>();
    assert(col);
    for(size_t i = 0; i < arr_len; ++i) {
        col->push_back(arr[i]);
    }
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}


std::shared_ptr<DataFrame> DataFrame::from_scalar(KVStore::Key k, int val){
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<IntColumn>();
    assert(col);
    col->push_back(val);
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

std::shared_ptr<DataFrame> DataFrame::from_scalar(KVStore::Key k, double val){
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<FloatColumn>();
    assert(col);
    col->push_back(val);
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

std::shared_ptr<DataFrame> DataFrame::from_scalar(KVStore::Key k, bool val){
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<BoolColumn>();
    assert(col);
    col->push_back(val);
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

std::shared_ptr<DataFrame> DataFrame::from_scalar(KVStore::Key k, std::string val){
    auto df = std::make_shared<DataFrame>();
    auto col = std::make_unique<StringColumn>();
    assert(col);
    col->push_back(val);
    df->add_column(std::move(col));
    KVStore::get_instance().set(k, df);
    return df;
}

DataFrame::DataFrame() : _schema(std::make_unique<Schema>()) {}

DataFrame::DataFrame(const DataFrame& df) : DataFrame(std::make_unique<Schema>(*df._schema)) {}

DataFrame::DataFrame(std::unique_ptr<Schema> schema) : _schema(std::move(schema)), _columns() {
    for(size_t i = 0; i < _schema->width(); ++i){
        auto col = _get_col_from_type(_schema->col_type(i));
        exit_if_not(!!col, "Failed to create column");
        _columns.push_back(std::move(col));
    }
}

DataFrame::~DataFrame() {}

std::unique_ptr<Column> DataFrame::_get_col_from_type(char type) const {
    switch(type) {
        case 'I':
            return std::make_unique<IntColumn>();
        case 'F':
            return std::make_unique<FloatColumn>();
        case 'B':
            return std::make_unique<BoolColumn>();
        case 'S':
            return std::make_unique<StringColumn>();
        default:
            p("Unknown Column Type: ").p(type).p('(').p((int) type).p(')').pln();
            return std::unique_ptr<Column>(nullptr);
    }
}

const Schema& DataFrame::get_schema() const {
    return *_schema;
}

/** Adds a column this dataframe, updates the schema, the new column
* is external, and appears as the last column of the dataframe, the
* name is optional and external. A nullptr colum is undefined. */
void DataFrame::add_column(std::unique_ptr<Column> col, std::optional<std::string> name){
    assert(col);
    _schema->add_column(col->get_type(), name);
    _columns.push_back(std::move(col));
}

/** Return the value at the given column and row. Accessing rows or
*  columns out of bounds, or request the wrong type is undefined.*/
std::optional<int> DataFrame::get_int(size_t col, size_t row) const {
    exit_if_not(col < _columns.size(), "Col index out of range.");
    auto column = _columns[col]->as_int();
    exit_if_not(column, "Column is not of type integer.");
    return column->get(row);
}

std::optional<bool> DataFrame::get_bool(size_t col, size_t row) const {
    exit_if_not(col < _columns.size(), "Col index out of range.");
    auto column = _columns[col]->as_bool();
    exit_if_not(column, "Column is not of type bool.");
    return column->get(row);
}

std::optional<double> DataFrame::get_double(size_t col, size_t row) const {
    exit_if_not(col < _columns.size(), "Col index out of range.");
    auto column = _columns[col]->as_float();
    exit_if_not(column, "Column is not of type float.");
    return column->get(row);
}

std::optional<std::string> DataFrame::get_string(size_t col, size_t row) const {
    exit_if_not(col < _columns.size(), "Col index out of range.");
    auto column = _columns[col]->as_string();
    exit_if_not(column, "Column is not of type string.");
    return column->get(row);
}

int DataFrame::get_col(const std::string& col) const {
    return _schema->col_idx(col);
}

int DataFrame::get_row(const std::string& row) const {
    return _schema->row_idx(row);
}

void DataFrame::set(size_t col, size_t row, std::optional<int> val) {
    _columns[col]->as_int()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, std::optional<bool> val) {
    _columns[col]->as_bool()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, std::optional<double> val) {
    _columns[col]->as_float()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, std::optional<std::string> val) {
    _columns[col]->as_string()->set(row, val);
}

void DataFrame::fill_row(size_t idx, Row& row) const {
    row.set_index(idx);
    for(size_t c = 0; c < _columns.size(); ++c){
        switch(_schema->col_type(c)){
            case 'I':
                row.set(c, this->get_int(c, idx));
                break;
            case 'F':
                row.set(c, this->get_double(c, idx));
                break;
            case 'B':
                row.set(c, this->get_bool(c, idx));
                break;
            case 'S':
                row.set(c, this->get_string(c, idx));
                break;
            default:
                p("Unexpected Column Type At Index ").p(c).p(": ").pln(_schema->col_type(c));
                assert(false); // unreachable
        }
    }
}

void DataFrame::add_row(Row& row) {
    for(size_t c = 0; c < _columns.size(); ++c){
        switch(_columns[c]->get_type()){
            case 'I':
                _columns[c]->push_back(row.get_int(c));
                break;
            case 'F':
                _columns[c]->push_back(row.get_double(c));
                break;
            case 'B':
                _columns[c]->push_back(row.get_bool(c));
                break;
            case 'S':
                _columns[c]->push_back(row.get_string(c));
                break;
            default:
                p("Unexpected Column Type At Index ").p(c).p(": ").pln(_schema->col_type(c));
                assert(false); // unreachable
        }
    }
}

size_t DataFrame::nrows() const {
    if(ncols() > 0){
        return _columns[0]->size();
    } else {
        return _schema->length();
    } 
}

size_t DataFrame::ncols() const {
    return _columns.size();
}

void DataFrame::map(Rower& r) const {
    Row row(*_schema);
    for(size_t i = 0; i < this->nrows(); ++i) {
        this->fill_row(i, row);
        r.accept(row);
    }
}

/** This method is used to execute a map in parallel. The row_start
 * is the starting row for the rower in this thread, the row_end is
 * the non-inclusive end row for the rower in this thread. This method
 * should be passed to a thread as the method it is executing.
 *
 * If the rower is mutating shared objects or data, it must 
 * do that in a thread safe manner internally, or reults are
 * undefined behavior.
 */
void DataFrame::_pmap_helper(size_t row_start, size_t row_end, Rower& rower) const {
    for(size_t r = row_start; r < row_end; ++r) {
        Row row(*_schema);
        this->fill_row(r, row);
        rower.accept(row);
    }
}

void DataFrame::pmap(Rower& rower) const {
    size_t row_cnt = this->nrows();
    // decide how many threads to use
    size_t thread_cnt = row_cnt / THREAD_ROWS;
    if(thread_cnt <= 1){
        // don't bother multi-threading, it will be faster to single thread
        this->map(rower);
        return;
    }
    size_t step_size = THREAD_ROWS;
    if(thread_cnt > MAX_THREADS) {
        thread_cnt = MAX_THREADS;
        step_size = row_cnt / thread_cnt;
    }

    std::vector<std::shared_ptr<Rower>> rower_clones;
    for(size_t i = 0; i < thread_cnt - 1; ++i){
        auto rc = std::dynamic_pointer_cast<Rower>(rower.clone());
        if(i == 1 && !rc){
            // clone failed - fallback to single threading
            map(rower);
            return;
        } else {
            assert(rc); // make sure clone suceeds
        }
        rower_clones.push_back(rc);
    }
    assert(thread_cnt - 1 == rower_clones.size());

    std::vector<std::thread> threads;

    // run multi-threaded
    size_t row_start = 0;
    for(size_t i = 0; i < thread_cnt; ++i) {
        // figure out bounds
        size_t row_end = (i == (thread_cnt - 1)) ? this->nrows() : row_start + step_size;
        assert(row_end <= row_cnt);
        // this function constructs the thread at the end of the array
        threads.emplace_back([this, row_start, row_end, i, &rower_clones, &rower]{
                this->_pmap_helper(row_start, row_end, (i > 0 ? *rower_clones[i - 1] : rower));
        });
        row_start = row_end;
    }
    assert(threads.size() == thread_cnt);

    // wait on threads
    for(size_t i = 0; i < threads.size(); ++i){
        threads[i].join();
    }
    
    // merge rower results and clean-up allocated memory
    for(size_t i = 0; i < rower_clones.size(); ++i) {
        rower.join(rower_clones[i]);
    }
}

std::shared_ptr<DataFrame> DataFrame::filter(Rower& r) const {
    auto df = std::make_shared<DataFrame>(*this);

    for(size_t i = 0; i < _schema->length(); ++i) {
        Row row(*_schema);
        this->fill_row(i, row);
        if(r.accept(row)) {
            df->add_row(row);
        }
    }
    return df;
}

void DataFrame::print() const {
    PrintRower pr;
    this->map(pr);
}

bool DataFrame::equals(const Object* other) const {
    const DataFrame *df_other = dynamic_cast<const DataFrame*>(other);
    if(df_other && _columns.size() == df_other->_columns.size() && _schema->equals(df_other->_schema.get())){
        for(size_t c = 0; c < _columns.size(); ++c) {
            if(!_columns[c]->equals(df_other->_columns[c].get())) return false;
        }
        return true;
    }
    return false;
}

std::shared_ptr<Object> DataFrame::clone() const {
    return std::make_shared<DataFrame>(*this);
}

size_t DataFrame::hash() const {
    size_t hash = _schema->hash();
    for(size_t i = 0; i < _columns.size(); ++i){
        hash += _columns[i]->hash();
    }
    return hash;
}

bool DataFrame::operator==(const DataFrame& other) const {
    return _columns == other._columns && _schema->equals(other._schema.get());
}

std::vector<uint8_t> DataFrame::serialize() const {
    std::vector<uint8_t> serialized = _schema->serialize();
    size_t col_cnt = _columns.size();
    auto temp =  Serializable::serialize<size_t>(col_cnt);
    serialized.insert(serialized.end(), temp.begin(), temp.end());
    for(size_t i = 0; i < col_cnt; ++i){
        temp = _columns[i]->serialize();
        serialized.insert(serialized.end(), temp.begin(), temp.end());
    }
    return serialized;
}


// Print Rower
bool DataFrame::PrintRower::accept(Row& r){
    r.visit(r.get_index(), this->pf);
    return true;
}

void DataFrame::PrintRower::join([[maybe_unused]]std::shared_ptr<Rower> other) {
    assert(false);
}

size_t DataFrame::PrintRower::hash() const {
    return 100;
}

bool DataFrame::PrintRower::equals(const Object *other) const {
    return dynamic_cast<const DataFrame::PrintRower*>(other);
}

std::shared_ptr<Object> DataFrame::PrintRower::clone() const {
    return nullptr;
}

// Print Fielder
/* void DataFrame::PrintRower::PrintFielder::start(size_t r){} */
/** Called for fields of the argument's type with the value of the field. */
void DataFrame::PrintRower::PrintFielder::accept(std::optional<bool> b){
    p('<');
    if(b) p(*b);
    p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(std::optional<double> f){
    p('<');
    if(f) p(*f);
    p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(std::optional<int> i){
    p('<');
    if(i) p(*i);
    p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(std::optional<std::string> s){
    p('<');
    if(s) {
        p(s->c_str());
    }
    p('>');
}

std::shared_ptr<Object> DataFrame::PrintRower::PrintFielder::clone() const {
    return nullptr;
}

/** Called when all fields have been seen. */
void DataFrame::PrintRower::PrintFielder::done(){
    pln();
}

size_t DataFrame::PrintRower::PrintFielder::hash() const {
    return 100;
}

bool DataFrame::PrintRower::PrintFielder::equals(const Object *other) const {
    return dynamic_cast<const DataFrame::PrintRower::PrintFielder *>(other);
}
