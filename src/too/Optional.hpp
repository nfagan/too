//
//  Optional.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include <utility>

namespace too {
  template <typename T>
  class Optional;
  
  class NullOpt {
    //
  };
}

template <typename T>
class too::Optional {
public:
  constexpr Optional() noexcept : val(), is_null(true) {
    //
  }
  
  constexpr Optional(Optional&& other) noexcept : val(std::move(other.val)), is_null(other.is_null) {
    //
  }
  
  template <typename ...Args>
  constexpr explicit Optional(Args&&... args) : val(std::forward<Args...>(args...)), is_null(false) {
    //
  }
  
  Optional(const NullOpt& other) noexcept : Optional() {
    //
  }
  
  Optional(const Optional& other) : val(other.val), is_null(other.is_null) {
    //
  }
  
  ~Optional() = default;
  
  Optional& operator=(const NullOpt& other) {
    val.~T();
    is_null = true;
    
    return *this;
  }
  
  Optional& operator=(const Optional& other) {
    is_null = other.is_null;
    val = other.val;
    
    return *this;
  }
  
  Optional& operator=(Optional&& other) {    
    is_null = other.is_null;
    val = std::move(other.val);
    
    return *this;
  }
  
  Optional& operator=(const T& value) {
    val = value;
    is_null = false;
    
    return *this;
  }
  
  Optional& operator=(T&& value) {
    val = std::move(value);
    is_null = false;
    
    return *this;
  }
  
  template <typename ...Args>
  void emplace(Args&&... args) {
    val.~T();
    new (&val) T(std::forward<Args...>(args...));
  }
  
  operator bool() const {
    return !is_null;
  }
  
  bool has_value() const {
    return !is_null;
  }
  
  const T& value() const {
    return val;
  }
  
  T&& rvalue() {
    return std::move(val);
  }
  
private:
  T val;
  bool is_null;
};

inline bool operator==(const too::NullOpt& lhs, const too::NullOpt& rhs) {
  return true;
}

inline bool operator!=(const too::NullOpt& lhs, const too::NullOpt& rhs) {
  return false;
}

template <typename T>
inline bool operator==(const too::Optional<T>& lhs, const too::NullOpt& rhs) {
  return !lhs.has_value();
}

template <typename T>
inline bool operator!=(const too::Optional<T>& lhs, const too::NullOpt& rhs) {
  return !(lhs == rhs);
}

template <typename T>
inline bool operator==(const too::NullOpt& lhs, const too::Optional<T>& rhs) {
  return !rhs.has_value();
}

template <typename T>
inline bool operator!=(const too::NullOpt& lhs, const too::Optional<T>& rhs) {
  return !(lhs == rhs);
}

template <typename T>
inline bool operator==(const too::Optional<T>& lhs, const too::Optional<T>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  
  if (!lhs.has_value()) {
    return true;
  }
  
  return lhs.value() == rhs.value();
}
