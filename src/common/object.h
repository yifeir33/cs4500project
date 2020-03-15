#pragma once
#include "helper.h"
// LANGUAGE: CwC

/** Base class for all objects in the system.
 *  author: vitekj@me.com */
class Object : public Sys {
public:
    /** Subclasses may have something to do on finalziation */
    virtual ~Object();

    /** Return the hash value of this object */
    virtual size_t hash() const = 0;

    /** Subclasses should redefine */
    virtual bool equals(const Object  * other) const = 0;

    /** Return a copy of the object; nullptr is considered an error */
    virtual Object* clone() const = 0;

    /** Returned c_str is owned by the object, don't modify nor delete. */
    virtual char* c_str() const = 0;
}; 
