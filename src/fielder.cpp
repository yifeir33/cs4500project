#include "fielder.h"

/** Called before visiting a row, the argument is the row offset in the
dataframe. */
void Fielder::start([[maybe_unused]]size_t r){}

/** Called for fields of the argument's type with the value of the field. */
void Fielder::accept([[maybe_unused]]bool b){}
void Fielder::accept([[maybe_unused]]float f){}
void Fielder::accept([[maybe_unused]]int i){}
void Fielder::accept([[maybe_unused]]std::weak_ptr<std::string> s){}

/** Called when all fields have been seen. */
void Fielder::done(){}
