//
//  Vector.hpp
//  too
//
//  Created by Nick Fagan on 4/12/19.
//

#pragma once

#include "ArrayAllocator.hpp"
#include <cstdint>
#include <cstddef>
#include <iostream>

namespace too {
  template <typename T>
  class Vector;
}

template <typename T>
class too::Vector {
public:
  Vector() : contents(nullptr), capacity(0), count(0) {
    //
  }
  
  Vector(Vector&& other) : contents(other.contents), capacity(other.capacity), count(other.count) {
    other.zero_members();
  }
  
  Vector(const Vector& other) :
    contents(ArrayAllocator::reallocate<T>(nullptr, other.count, other.capacity)),
    capacity(other.capacity),
    count(other.count) {
    ArrayAllocator::copy<T>(other.contents, contents, count);
  }
  
  Vector& operator=(const Vector& other) {
    if (this != &other) {
      ~Vector<T>();
      
      capacity = other.capacity;
      count = other.count;
      
      contents = ArrayAllocator::reallocate<T>(nullptr, count, capacity);
      ArrayAllocator::copy<T>(other.contents, contents, count);
    }
    
    return *this;
  }
  
  Vector& operator=(Vector&& other) {
    if (this != &other) {
      ~Vector<T>();
      
      contents = other.contents;
      capacity = other.capacity;
      count = other.count;
      
      other.zero_members();
    }
    
    return *this;
  }
  
  ~Vector() {
    ArrayAllocator::destroy<T>(contents, count);
    ArrayAllocator::deallocate<T>(contents);
  }
  
  void push_back(const T& value) {
    if (count == capacity) {
      increase_capacity();
    }
    
    contents[count++] = value;
  }
  
  const T* data() const {
    return contents;
  }
  
  T* data() {
    return contents;
  }
  
  const T& operator[](int64_t at) const {
    return contents[at];
  }
  
  T& operator[](int64_t at) {
    return contents[at];
  }
  
  void reserve(int64_t to) {
    if (to < 0 || to < capacity) {
      return;
    }
    
    contents = ArrayAllocator::reallocate<T>(contents, count, to);
    capacity = to;
  }
  
  int64_t size() const {
    return count;
  }
  
private:
  void increase_capacity() {
    capacity = (capacity == 0) ? 8 : capacity * 2;
    contents = ArrayAllocator::reallocate<T>(contents, count, capacity);
  }
  
  void zero_members() {
    contents = nullptr;
    count = 0;
    capacity = 0;
  }
  
private:
  T* contents;
  
  int64_t capacity;
  int64_t count;
};
