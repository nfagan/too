//
//  StringView.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <ostream>

namespace too {
  class StringView;
}

class too::StringView {
public:
  StringView() : str(nullptr), len(0) {
    //
  }
  
  StringView(const char* str) : str(str), len(std::strlen(str)) {
    //
  }
  
  StringView(const char* str, int64_t len) : str(str), len(len) {
    //
  }
  
  ~StringView() = default;
  
  int64_t size() const {
    return len;
  }
  
  int64_t length() const {
    return len;
  }
  
  bool is_empty() const {
    return len == 0;
  }
  
  const char* data() const {
    return str;
  }
  
  char operator[](int64_t at) const {
    return str[at];
  }
  
private:
  const char* str;
  int64_t len;
};

inline std::ostream& operator<<(std::ostream& os, too::StringView view) {
  for (int64_t i = 0; i < view.size(); i++) {
    os << view[i];
  }
  
  return os;
}

inline bool operator==(const too::StringView& a, const too::StringView& b) {
  const auto sz = a.size();
  
  if (sz != b.size()) {
    return false;
  }
  
  return std::memcmp(a.data(), b.data(), sz) == 0;
}

inline bool operator!=(const too::StringView& a, const too::StringView& b) {
  return !(a == b);
}

inline bool operator<(const too::StringView& a, const too::StringView& b) {
  const auto sz_a = a.size();
  const auto sz_b = b.size();
  const auto sz = sz_a > sz_b ? sz_b : sz_a;
  
  int64_t i = 0;
  
  while (i < sz && (a[i] == b[i])) {
    i++;
  }
  
  if (i == sz) {
    return sz_a < sz_b;
  }
  
  return a[i] < b[i];
}

//
//
//  Comparator

namespace std {
  template<> struct less<too::StringView> {
    bool operator()(const too::StringView& a, const too::StringView& b) const {
      return ::operator<(a, b);
    }
  };
}
