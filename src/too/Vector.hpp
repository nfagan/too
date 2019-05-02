//
//  Vector.hpp
//  too
//
//  Created by Nick Fagan on 4/12/19.
//

#pragma once

#include "Config.hpp"
#include <cstdint>
#include <cstddef>
#include <cassert>

#ifdef USE_TOO_VECTOR

#include "ArrayAllocator.hpp"
#include <utility>

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

namespace too {
  template <typename T>
  inline const T* data(const Vector<T>& vec) {
    return vec.data();
  }
  
  template <typename T>
  inline int64_t size(const Vector<T>& vec) {
    return vec.size();
  }
}

#ifdef USE_TOO_VECTOR

template <typename T>
class too::Vector {
public:
  Vector() : contents(nullptr), capacity(0), count(0) {
    //
  }
  
  explicit Vector(int64_t use_count) {
    if (use_count < 0) {
      use_count = 0;
    }
    
    count = use_count;
    capacity = use_count;
    contents = ArrayAllocator::reallocate<T>(nullptr, count, capacity);
  }
  
  Vector(const T* begin, const T* end) {
    count = end - begin;
    capacity = count;
    contents = ArrayAllocator::reallocate<T>(nullptr, count, capacity);
    
    ArrayAllocator::copy<T>(begin, contents, count);
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
    ArrayAllocator::deallocate<T>(contents);
    zero_members();
  }
  
  void swap(Vector<T>& other) {
    std::swap(contents, other.contents);
    std::swap(capacity, other.capacity);
    std::swap(count, other.count);
  }
  
  void push_back(const T& value) {
    if (count == capacity) {
      increase_capacity();
    }
    
    contents[count++] = value;
  }
  
  void push_back(T&& value) {
    if (count == capacity) {
      increase_capacity();
    }
    
    contents[count++] = std::move(value);
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
  
  T& back() {
    return contents[count-1];
  }
  
  bool empty() const {
    return count == 0;
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
  
  void resize(int64_t to) {
    if (to <= count) {
      return;
    }
    
    if (to > capacity) {      
      contents = ArrayAllocator::reallocate<T>(contents, count, to);
      capacity = to;
    }
    
    ArrayAllocator::construct<T>(contents + count, to - count);
    count = to;
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

template <typename T>
void swap(too::Vector<T>& a, too::Vector<T>& b) {
  a.swap(b);
}

#endif
