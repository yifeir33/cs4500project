#pragma once
#include <iostream>
#include <cstring>

#include "object.h"
#include "string.h"
#include "assert.h"

#define FLOAT_EQUALITY 0.001

/**
 * An Array class to which elements can be added to and removed from.
 * author: chasebish */
class ObjectArray : public Object {
public:

  /** VARIABLES */
  
  size_t capacity_; // the capacity_ of the Array
  size_t length_; // length of the Array
  Object** data_; // contents of the Array

  /** CONSTRUCTORS & DESTRUCTORS **/

  /** Creates an Array of desired size */
  ObjectArray(const size_t array_size) : capacity_(array_size), length_(0), data_(nullptr) {
      data_ = new Object*[capacity_];
      memset(data_, 0, sizeof(Object*) * capacity_);
  }

  /** Copies the contents of an already existing Array */
  ObjectArray(ObjectArray* const copy_array) : capacity_(copy_array->capacity_), length_(copy_array->length_), data_(nullptr) {
      data_= new Object*[capacity_];
      memcpy(data_, copy_array->data_, sizeof(Object*) * copy_array->length());
  }

  /** Clears Array from memory */
  virtual ~ObjectArray() {
      delete[] data_;
  }


  /** INHERITED METHODS **/

  /** Inherited from Object, generates a hash for an Array */
  size_t hash() {
      size_t hash = length_;
      for(size_t i = 0; i < length_; ++i){
          hash += data_[i]->hash() * i;
      }
      return hash;
  }

  /** Inherited from Object, checks equality between an Array and an Object */
  bool equals(Object* obj) {
      ObjectArray *arr_ptr = reinterpret_cast<ObjectArray *>(obj);
      if(arr_ptr){
          if(length_ == arr_ptr->length_){
              for(size_t i = 0; i < length_; ++i){
                  Object* other_obj = arr_ptr->get(i);
                  if(!data_[i]){
                      // this allows for nullptrs in the array
                      if(data_[i] != other_obj) {
                          return false;
                      }
                  } else if(!data_[i]->equals(other_obj)){
                      return false;
                  }
              }
              return true;
          }
      }
      return false;
  }

  /** ARRAY METHODS **/

  /** Removes all elements from the Array */
  void clear() {
      memset(data_, 0, sizeof(Object*) * capacity_);
      length_ = 0;
  }

  /** Adds an Array to existing contents */
  void concat(ObjectArray* toAdd) {
      ensure_size_(length_ + toAdd->length());
      memcpy(data_ + length_, toAdd->data_, sizeof(Object*) * toAdd->length());
      length_ += toAdd->length_;
  }

  /** Gets an Object at the given index */
  Object* get(size_t index) {
      assert(index >= 0 && index < length_);
      return data_[index];
  }

  /** Returns the index of the given Object, -1 if Object is not found */
  int index_of(Object* to_find) {
      for(size_t i = 0; i < length_; ++i){
          if(!data_[i]) {
              if(data_[i] == to_find) return i; // allows for nullptr
          } else if(data_[i]->equals(to_find)){
              return i;
          }
      }
      return -1;
  }

  /** Returns the current length of the contents in an Array */
  size_t length() {
      return length_;
  }

  /** Removes the last Object of the Array, returns the removed Object */
  Object* pop() {
      Object *out = data_[length_ - 1];
      data_[length_ - 1] = nullptr;
      length_--;
      return out;
  }

  /** Adds an Object to the end of the Array, returns the new length */
  size_t push(Object* to_add) {
      this->ensure_size_(length_ + 1);
      data_[length_] = to_add;
      return ++length_;
  }

  /** Removes an Object at the given index, returns removed Object */
  Object* remove(size_t index) {
      assert(index >= 0 && index < length_);
      Object *output = data_[index];
      memmove(data_ + index, data_ + index + 1, sizeof(Object*) * (length_ - index));
      length_--;
      return output;
  }

  /** Replaces an Object at the given index with the given Object, returns the replaced Object */
  Object* replace(size_t index, Object* new_value) {
      assert(index >= 0 && index < length_);
      Object *old = data_[index];
      data_[index] = new_value;
      return old;
  }

  /** This is a helper method ensures the size with capacity by double it everytime*/
  virtual void ensure_size_(size_t required){
    if(required >= this->capacity_){
      // grow
      size_t newCap = capacity_ * 2;
      while(required > newCap){
        newCap *= 2;
      }

      Object **new_array = new Object*[newCap];
      memcpy(new_array, data_, sizeof(Object*) * length_);

      delete[] data_;
      data_ = new_array;
      capacity_ = newCap;
    }
  }
};

/**
 * An Array class to which elements can be added to and removed from.
 * author: chasebish */
class BoolArray : public Object {
public:

  /** VARIABLES */
  size_t capacity_; // the capacity_ of the Array
  size_t length_; // length of the Array
  bool* data_; // contents of the Array

  /** CONSTRUCTORS & DESTRUCTORS **/

  /** Creates an Array of desired size */
  BoolArray(const size_t size) : capacity_(size), length_(0) {
      data_ = new bool[capacity_];
      memset(data_, false, sizeof(bool) * capacity_);
  }

  /** Copies the contents of an already existing Array */
  BoolArray(BoolArray* const arr) : capacity_(arr->capacity_), length_(arr->length_), data_(nullptr) {
      data_ = new bool[capacity_];
      memcpy(data_, arr->data_, sizeof(bool) * arr->length());
  }

  /** Clears Array from memory */
  ~BoolArray() {
      delete[] data_;
  }


  /** INHERITED METHODS **/

  /** Inherited from Object, generates a hash for an Array */
  size_t hash() {
      size_t hash = length_;
      for(size_t i = 0; i < length_; ++i){
          hash += data_[i] * i;
      }
      return hash;
  }

  /** Inherited from Object, checks equality between an Array and an Object */
  bool equals(Object* obj) {
      BoolArray *arr_ptr = reinterpret_cast<BoolArray *>(obj);
      if(arr_ptr){
          if(length_ == arr_ptr->length_){
              for(size_t i = 0; i < length_; ++i){
                  if(data_[i] != arr_ptr->get(i)){
                      return false;
                  }
              }
              return true;
          }
      }
      return false;
  }

  /** ARRAY METHODS **/

  /** Removes all elements from the Array */
  void clear() {
      memset(data_, false, sizeof(bool) * capacity_);
      length_ = 0;
  }

  /** Adds an Array to existing contents */
  void concat(BoolArray* toAdd) {
      ensure_size_(length_ + toAdd->length());
      memcpy(data_ + length_, toAdd->data_, sizeof(bool) * toAdd->length());
      length_ += toAdd->length_;
  }

  /** Gets an Object at the given index */
  bool get(size_t index) {
      assert(index >= 0 && index < this->length_);
      return data_[index];
  }

  /** Returns the index of the given Object, -1 if Object is not found */
  int index_of(bool to_find) {
      for(size_t i = 0; i < length_; ++i){
          if(data_[i] == to_find) return i;
      }
      return -1; // TODO
  }

  /** Returns the current length of the contents in an Array */
  size_t length() {
      return length_;
  }

  /** Removes the last Object of the Array, returns the removed Object */
  bool pop() {
      bool out = data_[length_ - 1];
      length_--;
      return out;
  }

  /** Adds an Object to the end of the Array, returns the new length */
  size_t push(bool to_add) {
      this->ensure_size_(length_ + 1);
      data_[length_] = to_add;
      return ++length_;
  }

  /** Removes an Object at the given index, returns removed Object */
  bool remove(size_t index) {
      assert(index >= 0 && index < length_);
      bool output = data_[index];
      memmove(data_ + index, data_ + index + 1, sizeof(bool) * (length_ - index));
      length_--;
      return output;
  }

  /** Replaces an Object at the given index with the given Object, returns the replaced Object */
  bool replace(size_t index, bool new_value) {
      assert(index >= 0 && index < this->length_);
      bool old = data_[index];
      data_[index] = new_value;
      return old;
  }

  virtual void ensure_size_(size_t required){
    if(required >= this->capacity_){
      // grow
      size_t newCap = capacity_ * 2;
      while(required > newCap){
        newCap *= 2;
      }

      bool *new_array = new bool[newCap];
      memcpy(new_array, data_, sizeof(bool) * length_);

      delete[] data_;
      data_ = new_array;
      capacity_ = newCap;
    }
  }
};

/**
 * An Array class to which elements can be added to and removed from.
 * author: chasebish */
class FloatArray : public Object {
public:

  /** VARIABLES */
  size_t capacity_; // the capacity_ of the Array
  size_t length_; // length of the Array
  float* data_; // contents of the Array

    /** CONSTRUCTORS & DESTRUCTORS **/

    /** Creates an Array of desired size */
    FloatArray(const size_t array_size) : capacity_(array_size), length_(0) {
        data_ = new float[capacity_];
        memset(data_, 0, sizeof(float) * capacity_);
    }

    /** Copies the contents of an already existing Array */
    FloatArray(FloatArray* const copy_array) : capacity_(copy_array->capacity_), length_(copy_array->length_), data_(nullptr) {
        data_= new float[capacity_];
        memcpy(data_, copy_array->data_, sizeof(float) * copy_array->length());
    }

    /** Clears Array from memory */
    ~FloatArray() {
        delete[] data_;
    }


    /** INHERITED METHODS **/

    /** Inherited from Object, generates a hash for an Array */
    size_t hash() {
        size_t hash = length_;
        for(size_t i = 0; i < length_; ++i){
            hash += data_[i] * i;
        }
        return hash;
    }

    /** Inherited from Object, checks equality between an Array and an Object */
    bool equals(Object* obj) {
      FloatArray *arr_ptr = reinterpret_cast<FloatArray *>(obj);
      if(arr_ptr){
          if(length_ == arr_ptr->length_){
              for(size_t i = 0; i < length_; ++i){
                  if(std::abs(data_[i] - arr_ptr->get(i)) > FLOAT_EQUALITY){
                      return false;
                  }
              }
              return true;
          }
      }
      return false;
    }

    /** ARRAY METHODS **/

    /** Removes all elements from the Array */
    void clear() {
        memset(data_, 0, sizeof(float) * capacity_);
        length_ = 0;
    }

    /** Adds an Array to existing contents */
    void concat(FloatArray* toAdd) {
      ensure_size_(length_ + toAdd->length());
      memcpy(data_ + length_, toAdd->data_, sizeof(float) * toAdd->length());
      length_ += toAdd->length_;
    }

    /** Gets an Object at the given index */
    float get(size_t index) {
        assert(index >= 0 && index < length_);
        return data_[index];
    }

    /** Returns the index of the given Object, -1 if Object is not found */
    int index_of(float to_find) {
        for(size_t i = 0; i < length_; ++i){
            if(std::abs(data_[i] - to_find) < FLOAT_EQUALITY) return i;
        }
        return -1;
    }

    /** Returns the current length of the contents in an Array */
    size_t length() {
        return length_;
    }

    /** Removes the last Object of the Array, returns the removed Object */
    float pop() {
        float out = data_[length_ - 1];
        length_--;
        return out;
    }

    /** Adds an Object to the end of the Array, returns the new length */
    size_t push(float to_add) {
        this->ensure_size_(length_ + 1);
        data_[length_] = to_add;
        return ++length_;
    }

    /** Removes an Object at the given index, returns removed Object */
    float remove(size_t index) {
        assert(index >= 0 && index < length_);
        float output = data_[index];
        memmove(data_ + index, data_ + index + 1, sizeof(float) * (length_ - index));
        length_--;
        return output;
    }

    /** Replaces an Object at the given index with the given Object, returns the replaced Object */
    float replace(size_t index, float new_value) {
        assert(index >= 0 && index < length_);
        float old = data_[index];
        data_[index] = new_value;
        return old;
    }

    virtual void ensure_size_(size_t required){
      if(required >= this->capacity_){
        // grow
        size_t newCap = capacity_ * 2;
        while(required > newCap){
          newCap *= 2;
        }

        float *new_array = new float[newCap];
        memcpy(new_array, data_, sizeof(float) * length_);

        delete[] data_;
        data_ = new_array;
        capacity_ = newCap;
      }
    }
};

/**
 * An Array class to which elements can be added to and removed from.
 * author: chasebish */
class IntArray : public Object {
public:

  /** VARIABLES */
  size_t capacity_; // the capacity_ of the Array
  size_t length_; // length of the Array
  int* data_; // contents of the Array

  /** CONSTRUCTORS & DESTRUCTORS **/

  /** Creates an Array of desired size */
  IntArray(const size_t array_size) : capacity_(array_size), length_(0) {
      data_ = new int[capacity_];
      memset(data_, 0, sizeof(int) * capacity_);
  }

  /** Copies the contents of an already existing Array */
  IntArray(IntArray* const copy_array) : capacity_(copy_array->capacity_), length_(copy_array->length_), data_(nullptr) {
      data_ = new int[capacity_];
      memcpy(data_, copy_array->data_, sizeof(int) * copy_array->length());
  }

  /** Clears Array from memory */
  ~IntArray() {
      delete[] data_;
  }


  /** INHERITED METHODS **/

  /** Inherited from int, generates a hash for an Array */
  size_t hash() {
      size_t hash = length_;
      for(size_t i = 0; i < length_; ++i){
          hash += data_[i] * i;
      }
      return hash;
  }

  /** Inherited from int, checks equality between an Array and an int */
  bool equals(Object *obj) {
      IntArray *arr_ptr = reinterpret_cast<IntArray *>(obj);
      if(arr_ptr){
          if(length_ == arr_ptr->length_){
              for(size_t i = 0; i < length_; ++i){
                  if(data_[i] != arr_ptr->get(i)){
                      return false;
                  }
              }
              return true;
          }
      }
      return false;
  }

  /** ARRAY METHODS **/

  /** Removes all elements from the Array */
  void clear() {
      memset(data_, 0, sizeof(int) * capacity_);
      length_ = 0;
  }

  /** Adds an Array to existing contents */
  void concat(IntArray* toAdd) {
      ensure_size_(length_ + toAdd->length());
      memcpy(data_ + length_, toAdd->data_, sizeof(int) * toAdd->length());
      length_ += toAdd->length_;
  }

  /** Gets an int at the given index */
  int get(size_t index) {
      assert(index >= 0 && index < length_);
      return data_[index];
  }

  /** Returns the index of the given int, -1 if int is not found */
  int index_of(int to_find) {
      for(size_t i = 0; i < length_; ++i){
          if(data_[i] == to_find) return i;
      }
      return -1;
  }

  /** Returns the current length of the contents in an Array */
  size_t length() {
      return length_;
  }

  /** Removes the last int of the Array, returns the removed int */
  int pop() {
      int out = data_[length_ - 1];
      /* data_[length_ - 1] = nullptr; */
      length_--;
      return out;
  }

  /** Adds an int to the end of the Array, returns the new length */
  size_t push(int to_add) {
      this->ensure_size_(length_ + 1);
      data_[length_] = to_add;
      return ++length_;
  }

  /** Removes an int at the given index, returns removed int */
  int remove(size_t index) {
      assert(index >= 0 && index < length_);
      int output = data_[index];
      memmove(data_ + index, data_ + index + 1, sizeof(int) * (length_ - index));
      length_--;
      return output;
  }

  /** Replaces an int at the given index with the given int, returns the replaced int */
  int replace(size_t index, int new_value) {
      assert(index >= 0 && index < length_);
      int old = data_[index];
      data_[index] = new_value;
      return old;
  }

  virtual void ensure_size_(size_t required){
    if(required >= this->capacity_){
      // grow
      size_t newCap = capacity_ * 2;
      while(required > newCap){
        newCap *= 2;
      }

      int *new_array = new int[newCap];
      memcpy(new_array, data_, sizeof(int) * length_);

      delete[] data_;
      data_ = new_array;
      capacity_ = newCap;
    }
  }
};

/**
 * An Array class to which elements can be added to and removed from.
 * author: chasebish */
class StringArray : public Object {
public:

  /** VARIABLES */
  ObjectArray _array;
  

  /** CONSTRUCTORS & DESTRUCTORS **/

  /** Creates an Array of desired size */
  StringArray(const size_t array_size) : _array(array_size) {}

  /** Copies the contents of an already existing Array */
  StringArray(StringArray* const copy_array) : _array(copy_array->_array) {}

  /** Clears Array from memory */
  /* ~StringArray() {} */


  /** INHERITED METHODS **/

  /** Inherited from String, generates a hash for an Array */
  size_t hash() {
      return _array.hash() + 5000;
  }

  /** Inherited from Object, checks equality between an Array and an String */
  bool equals(Object* obj) {
      return _array.equals(obj);
  }

  /** ARRAY METHODS **/

  /** Removes all elements from the Array */
  void clear() {
      _array.clear();
  }

  /** Adds an Array to existing contents */
  void concat(StringArray* toAdd) {
      _array.concat(&toAdd->_array);
  }

  /** Gets an String at the given index */
  String* get(size_t index) {
      return static_cast<String*>(_array.get(index));
  }

  /** Returns the index of the given String, -1 if String is not found */
  int index_of(String* to_find) {
      return _array.index_of(to_find);
  }

  /** Returns the current length of the contents in an Array */
  size_t length() {
      return _array.length();
  }

  /** Removes the last String of the Array, returns the removed String */
  String* pop() {
      return static_cast<String*>(_array.pop());
  }

  /** Adds an String to the end of the Array, returns the new length */
  size_t push(String* to_add) {
      return _array.push(to_add);
  }

  /** Removes an String at the given index, returns removed String */
  String* remove(size_t index) {
      return static_cast<String*>(_array.remove(index));
  }

  /** Replaces an String at the given index with the given String, returns the replaced String */
  String* replace(size_t index, String* new_value) {
      return static_cast<String*>(_array.replace(index, new_value));
  }

};
