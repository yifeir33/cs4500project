#include <iostream>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cassert>

#include "util/helper.h"

// Printing functions
const Sys& Sys::p(char* c) const { std::cout << c; return *this; }
const Sys& Sys::p(bool c) const { std::cout << c; return *this; }
const Sys& Sys::p(double c) const { std::cout << c; return *this; }  
const Sys& Sys::p(int i) const { std::cout << i;  return *this; }
const Sys& Sys::p(size_t i) const { std::cout << i;  return *this; }
const Sys& Sys::p(const char* c) const { std::cout << c;  return *this; }
const Sys& Sys::p(char c) const { (c == '\n' ? std::cout <<std::endl : std::cout << c);  return *this; }
const Sys& Sys::pln() const { std::cout << std::endl;  return *this; }
const Sys& Sys::pln(int i) const { std::cout << i << std::endl;  return *this; }
const Sys& Sys::pln(char* c) const { std::cout << c << std::endl;  return *this; }
const Sys& Sys::pln(bool c) const { std::cout << c << std::endl;  return *this; }  
const Sys& Sys::pln(char c) const { std::cout << c << std::endl;  return *this; }
const Sys& Sys::pln(double x) const { std::cout << x << std::endl;  return *this; }
const Sys& Sys::pln(size_t x) const { std::cout << x << std::endl;  return *this; }
const Sys& Sys::pln(const char* c) const { std::cout << c << std::endl;  return *this; }

// Copying strings
/* char* duplicate(const char* s) { */
/*     char* res = new char[strlen(s) + 1]; */
/*     strcpy(res, s); */
/*     return res; */
/* } */
/* char* duplicate(char* s) { */
/*     char* res = new char[strlen(s) + 1]; */
/*     strcpy(res, s); */
/*     return res; */
/* } */

// Function to terminate execution with a message
void Sys::exit_if_not(bool b, const char* c) const {
    if (b) return;
    p("Exit message: ").pln(c);
    assert(false);
}

// Definitely fail
//  void FAIL() {
void Sys::myfail() const{
    pln("Failing");
    assert(false);
}

// Some utilities for lightweight testing
void Sys::OK(const char* m) const { pln(m); }
void Sys::t_true(bool p) const { if (!p) myfail(); }
void Sys::t_false(bool p) const { if (p) myfail(); }
