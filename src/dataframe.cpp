#include "dataframe.h"

DataFrame::DataFrame(const DataFrame& df) : DataFrame(df._schema) {}

/** Create a data frame from a schema and columns. All columns are created
* empty. */
DataFrame::DataFrame(Schema& schema) : _schema(schema), _columns(schema.width()) {
    for(size_t i = 0; i < schema.width(); ++i){
        auto col = _get_col_from_type(schema.col_type(i));
        assert(col);
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
            return std::unique_ptr<Column>(nullptr);
    }
}

/** Returns the dataframe's schema. Modifying the schema after a dataframe
* has been created in undefined. */
const Schema& DataFrame::get_schema() const {
    return _schema;
}

/** Adds a column this dataframe, updates the schema, the new column
* is external, and appears as the last column of the dataframe, the
* name is optional and external. A nullptr colum is undefined. */
void DataFrame::add_column(std::unique_ptr<Column> col, std::shared_ptr<std::string> name){
    _schema.add_column(col->get_type(), name);
    _columns.push_back(std::move(col));
}

/** Return the value at the given column and row. Accessing rows or
*  columns out of bounds, or request the wrong type is undefined.*/
int DataFrame::get_int(size_t col, size_t row) const {
    return _columns[col]->as_int()->get(row);
}

bool DataFrame::get_bool(size_t col, size_t row) const {
    return _columns[col]->as_bool()->get(row);
}

float DataFrame::get_float(size_t col, size_t row) const {
    return _columns[col]->as_float()->get(row);
}

std::weak_ptr<std::string> DataFrame::get_string(size_t col, size_t row) const {
    return _columns[col]->as_string()->get(row);
}

/** Return the offset of the given column name or -1 if no such col. */
int DataFrame::get_col(std::string& col) const {
    return _schema.col_idx(col);
}

/** Return the offset of the given row name or -1 if no such row. */
int DataFrame::get_row(std::string& row) const {
    return _schema.row_idx(row);
}

/** Set the value at the given column and row to the given value.
* If the column is not  of the right type or the indices are out of
* bound, the result is undefined. */
void DataFrame::set(size_t col, size_t row, int val) {
    _columns[col]->as_int()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, bool val) {
    _columns[col]->as_bool()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, float val) {
    _columns[col]->as_float()->set(row, val);
}

void DataFrame::set(size_t col, size_t row, std::shared_ptr<std::string> val) {
    _columns[col]->as_string()->set(row, val);
}

/** Set the fields of the given row object with values from the columns at
* the given offset.  If the row is not form the same schema as the
* dataframe, results are undefined.
*/
void DataFrame::fill_row(size_t idx, Row& row) const {
    row.set_index(idx);
    for(size_t c = 0; c < _columns.size(); ++c){
        switch(_schema.col_type(c)){
            case 'I':
                row.set(c, this->get_int(c, idx));
                break;
            case 'F':
                row.set(c, this->get_float(c, idx));
                break;
            case 'B':
                row.set(c, this->get_bool(c, idx));
                break;
            case 'S':
                row.set(c, this->get_string(c, idx).lock());
                break;
            default:
                assert(false); // unreachable
        }
    }
}

/** Add a row at the end of this dataframe. The row is expected to have
*  the right schema and be filled with values, otherwise undedined.  */
void DataFrame::add_row(Row& row) {
    for(size_t c = 0; c < _columns.size(); ++c){
        switch(_columns[c]->get_type()){
            case 'I':
                _columns[c]->push_back(row.get_int(c));
                break;
            case 'F':
                _columns[c]->push_back(row.get_float(c));
                break;
            case 'B':
                _columns[c]->push_back(row.get_bool(c));
                break;
            case 'S':
                _columns[c]->push_back(row.get_string(c));
                break;
            default:
                assert(false); // unreachable
        }
    }
}

/** The number of rows in the dataframe. */
size_t DataFrame::nrows() const {
    if(ncols() > 0){
        return _columns[0]->size();
    } else {
        return _schema.length();
    } 
}

/** The number of columns in the dataframe.*/
size_t DataFrame::ncols() const {
    return _columns.size();
}

/** Visit rows in order */
void DataFrame::map(Rower& r) const {
    for(size_t i = 0; i < this->nrows(); ++i) {
        Row row(_schema);
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
void DataFrame::_pmap_helper(size_t row_start, size_t row_end, Rower *rower) const {
    for(size_t r = row_start; r < row_end; ++r) {
        Row row(_schema);
        this->fill_row(r, row);
        rower->accept(row);
    }
}

/** This method clones the Rower and executes the map in parallel. Join is
  * used at the end to merge the results. */
void DataFrame::pmap(Rower& r) const {
    size_t row_cnt = this->nrows();
    // decide how many threads to use
    size_t thread_cnt = row_cnt / THREAD_ROWS;
    if(thread_cnt <= 1){
        // don't bother multi-threading, it will be faster to single thread
        this->map(r);
        return;
    }
    size_t step_size = THREAD_ROWS;
    if(thread_cnt > MAX_THREADS) {
        thread_cnt = MAX_THREADS;
        step_size = row_cnt / thread_cnt;
    }

    Rower **rowers = new Rower*[thread_cnt];
    rowers[0] = &r;
    for(size_t i = 1; i < thread_cnt; ++i){
        rowers[i] = static_cast<Rower*>(r.clone());
        if(i == 1 && !rowers[i]){
            // clone failed - fallback to single threading
            delete[] rowers;
            map(r);
            return;
        } else {
            assert(rowers[i]); // make sure clone suceeds
        }
    }

    std::thread **threads = new std::thread*[thread_cnt];

    // run multi-threaded
    size_t row_start = 0;
    for(size_t i = 0; i < thread_cnt; ++i) {
        // figure out bounds
        size_t row_end = (i == (thread_cnt - 1)) ? this->nrows() : row_start + step_size;
        assert(row_end <= row_cnt);
        threads[i] = new std::thread(&DataFrame::_pmap_helper, this, row_start, row_end, rowers[i]);
        row_start = row_end;
    }

    // wait on threads
    for(size_t i = 0; i < thread_cnt; ++i){
        threads[i]->join();
    }
    
    // merge rower results and clean-up allocated memory
    for(size_t i = 1; i < thread_cnt; ++i) {
        rowers[0]->join_delete(rowers[i]);
    }

    // clean-up memory
    for(size_t i = 0; i < thread_cnt; ++i){
        delete threads[i];
    }
    delete[] threads;
    delete[] rowers;
}


/** Create a new dataframe, constructed from rows for which the given Rower
* returned true from its accept method. */
DataFrame* DataFrame::filter(Rower& r) const {
    DataFrame *df = new DataFrame(*this);

    for(size_t i = 0; i < _schema.length(); ++i) {
        Row row(_schema);
        this->fill_row(i, row);
        if(r.accept(row)) {
            df->add_row(row);
        }
    }
    return df;
}


/** Print the dataframe in SoR format to standard output. */
void DataFrame::print() const {
    PrintRower pr;
    this->map(pr);
}

bool DataFrame::equals(const Object* other) const {
    const DataFrame *df_other = dynamic_cast<const DataFrame*>(other);
    if(!df_other) return false;
    return _columns == df_other->_columns && _schema.equals(&df_other->_schema);
}

Object* DataFrame::clone() const {
    return new DataFrame(*this);
}

size_t DataFrame::hash() const {
    size_t hash = _schema.hash();
    for(size_t i = 0; i < _columns.size(); ++i){
        hash += _columns[i]->hash();
    }
    return hash;
}

// Print Rower
bool DataFrame::PrintRower::accept(Row& r){
    PrintFielder pf;
    r.visit(r.get_index(), pf);
    return true;
}

void DataFrame::PrintRower::join_delete([[maybe_unused]]Rower* other) {
    assert(false);
}

size_t DataFrame::PrintRower::hash() const {
    return Rower::hash() + 100;
}

bool DataFrame::PrintRower::equals(const Object *other) const {
    return dynamic_cast<const PrintRower*>(other);
}

Object* DataFrame::PrintRower::clone() const {
    return nullptr;
}

// Print Fielder
/* void DataFrame::PrintRower::PrintFielder::start(size_t r){} */
/** Called for fields of the argument's type with the value of the field. */
void DataFrame::PrintRower::PrintFielder::accept(bool b){
    _sys.p('<').p(b).p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(float f){
    _sys.p('<').p(f).p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(int i){
    _sys.p('<').p(i).p('>');
}

void DataFrame::PrintRower::PrintFielder::accept(std::weak_ptr<std::string> s){
    _sys.p('<').p(s.lock()->c_str()).p('>');
}

Object* DataFrame::PrintRower::PrintFielder::clone() const {
    return nullptr;
}

/** Called when all fields have been seen. */
void DataFrame::PrintRower::PrintFielder::done(){
    _sys.p('\n');
}

size_t DataFrame::PrintRower::PrintFielder::hash() const {
    return Fielder::hash() + 100;
}

bool DataFrame::PrintRower::PrintFielder::equals(const Object *other) const {
    return dynamic_cast<const PrintFielder*>(other);
}
