//
//  Stack.hpp
//  too
//
//  Created by Nick Fagan on 4/18/19.
//

#pragma once

namespace too {
  namespace stack {
    template <typename T, typename V>
    inline void push_back(const T& container, const V& value) {
      container.push_back(value);
    }
    
    template <typename T, typename V>
    inline const V& back(const T& container, const V& value) {
      return container.back();
    }
    
    template <typename T, typename V>
    inline void pop_back(const T& container, const V& value) {
      container.pop_back();
    }
    
    template <typename T>
    inline bool empty(const T& container) {
      return container.empty();
    }
  }
}
