//
//  Vector.hpp
//  too
//
//  Created by Nick Fagan on 4/12/19.
//

#pragma once

#include "Config.hpp"
#include "ArrayAllocator.hpp"
#include <cstdint>
#include <cstddef>
#include <iostream>

#ifdef USE_TOO_VECTOR

namespace too {
  template <typename T>
  class Vector;
}

#else

#include <vector>

namespace too {
  template <typename T>
  using Vector = std::vector<T>;
}

#endif

#ifdef USE_TOO_VECTOR

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
      if (capacity < other.capacity) {
        contents = ArrayAllocator::reallocate<T>(contents, count, other.capacity);
      }
      
      capacity = other.capacity;
      count = other.count;
      
      ArrayAllocator::copy<T>(other.contents, contents, count);
    }
    
    return *this;
  }
  
  Vector& operator=(Vector&& other) {
    if (this != &other) {
      this->~Vector<T>();
      
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
    
    zero_members();
  }
  
  void push_back(const T& value) {
    if (count == capacity) {
      increase_capacity();
    }
    
    contents[count++] = value;
  }
  
  void pop_back() {
    ArrayAllocator::destroy<T>(contents+count-1, 1);
    count--;
  }
  
  const T* data() const {
    return contents;
  }
  
  T* data() {
    return contents;
  }
  
  const T& back() const {
    return contents[count-1];
  }
  
  const T& operator[](int64_t at) const {
    return contents[at];
  }
  
  T& operator[](int64_t at) {
    return contents[at];
  }
  
  void reserve(int64_t to) {
    if (to <= capacity) {
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
    capacity = 0;
    count = 0;
  }
  
private:
  T* contents;
  
  int64_t capacity;
  int64_t count;
};

#endif
