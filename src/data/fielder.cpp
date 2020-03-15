#include "fielder.h"

/** Called before visiting a row, the argument is the row offset in the
dataframe. */
void Fielder::start(size_t r){}

/** Called for fields of the argument's type with the value of the field. */
void Fielder::accept(bool b){}
void Fielder::accept(float f){}
void Fielder::accept(int i){}
void Fielder::accept(std::string* s){}

/** Called when all fields have been seen. */
void Fielder::done(){}
