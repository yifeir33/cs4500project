#pragma once

#include <cstdlib>

/** Helper class providing some C++ functionality and convenience
 *  functions. This class has no data, constructors, destructors or
 *  virtual functions. Inheriting from it is zero cost.
 */
class Sys {
public:

    // Printing functions
    const Sys& p(char* c) const;
    const Sys& p(bool c) const;
    const Sys& p(float c) const;
    const Sys& p(int i) const;
    const Sys& p(size_t i) const;
    const Sys& p(const char* c) const;
    const Sys& p(char c) const;
    const Sys& pln() const;
    const Sys& pln(int i) const;
    const Sys& pln(char* c) const;
    const Sys& pln(bool c) const;
    const Sys& pln(char c) const;
    const Sys& pln(float x) const;
    const Sys& pln(size_t x) const;
    const Sys& pln(const char* c) const;

    // Copying strings
    /* char* duplicate(const char* s); */
    /* char* duplicate(char* s); */

    // Function to terminate execution with a message
    void exit_if_not(bool b, const char* c) const;

    // Definitely fail
    //  void FAIL() {
    void myfail() const;

    // Some utilities for lightweight testing
    void OK(const char* m) const;
    void t_true(bool p) const;
    void t_false(bool p) const;
};
