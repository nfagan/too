//
//  Array.hpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#pragma once

#include <cstdint>

namespace too {
  template <typename T, typename Size = int64_t>
  struct Span {
    T* data;
    const Size size;
    
    const T& operator[](int64_t at) const {
      return data[at];
    }
  };
  
  template <typename T>
  inline const T* data(const Span<T>& array) {
    return array.data;
  }
  
  template <typename T>
  inline int64_t size(const Span<T>& array) {
    return array.size;
  }
}
