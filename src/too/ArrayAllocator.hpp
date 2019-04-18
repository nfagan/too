//
//  ArrayAllocator.hpp
//  too
//
//  Created by Nick Fagan on 4/12/19.
//

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <utility>
#include <cassert>

namespace too {
  struct ArrayAllocator {
    
    template <typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static inline void deallocate(T* ptr) {
      if (ptr != nullptr) {
        free(ptr);
      }
    }
    
    template <typename T>
    static inline std::enable_if_t<!std::is_trivially_copyable_v<T>, void> deallocate(T* ptr) {
      if (ptr != nullptr) {
        delete[] ptr;
      }
    }
    
    template <typename T, std::enable_if_t<std::is_trivially_destructible_v<T>, int> = 0>
    static inline void destroy(T* ptr, int64_t count) {
      //
    }
    
    template <typename T>
    static inline std::enable_if_t<!std::is_trivially_destructible_v<T>, void> destroy(T* ptr, int64_t count) {
      for (int64_t i = 0; i < count; i++) {
        ptr[i].~T();
      }
    }
    
    template <typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static inline void copy(T* src, T* dest, int64_t count) {
      std::memcpy(dest, src, sizeof(T) * count);
    }
    
    template <typename T>
    static inline std::enable_if_t<!std::is_trivially_copyable_v<T>, void> copy(T* src, T* dest, int64_t count) {
      for (int64_t i = 0; i < count; i++) {
        dest[i] = src[i];
      }
    }
    
    template <typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static inline void construct(T* data, int64_t count) {
      std::memset(data, 0, sizeof(T) * count);
    }
    
    template <typename T>
    static inline std::enable_if_t<!std::is_trivially_constructible_v<T>, void> construct(T* data, int64_t count) {
      for (int64_t i = 0; i < count; i++) {
        new (&data[i]) T();
      }
    }
    
    template <typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static inline T* reallocate(T* src, int64_t src_count, int64_t dest_capacity) {
      T* dest = nullptr;
      
      if (dest_capacity > 0) {
        dest = (T*) std::malloc(sizeof(T) * dest_capacity);
        
        if (src != nullptr) {
          int64_t n_copied = dest_capacity > src_count ? src_count : dest_capacity;
          
          std::memcpy(dest, src, sizeof(T) * n_copied);
        }
      }
      
      if (src != nullptr) {
        free(src);
      }
      
      return dest;
    }
    
    template <typename T>
    static inline std::enable_if_t<!std::is_trivially_copyable_v<T>, T*> reallocate(T* src,
                                                                                    int64_t src_count,
                                                                                    int64_t dest_capacity) {
      static_assert(std::is_move_assignable_v<T>, "Non-trivially copyable class must be move assignable.");
      
      T* dest = nullptr;
      
      if (dest_capacity > 0) {
        dest = new T[dest_capacity];
        
        if (src != nullptr) {
          int64_t n_moved = dest_capacity > src_count ? src_count : dest_capacity;
          
          for (int64_t i = 0; i < n_moved; i++) {
            dest[i] = std::move(src[i]);
          }
        }
      }
      
      if (src != nullptr) {
        delete[] src;
      }
      
      return dest;
    }
    
  };
}
