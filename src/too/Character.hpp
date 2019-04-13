//
//  Character.hpp
//  too
//
//  Created by Nick Fagan on 4/13/19.
//

#pragma once

#include "Unicode.hpp"
#include <cstring>
#include <cstdint>

namespace too {
  class Character;
  class CharacterIterator;
}

class too::CharacterIterator {
public:
  CharacterIterator();
  CharacterIterator(const char* str, int64_t len);
  ~CharacterIterator() = default;
  
  bool has_next() const;
  int64_t next_index() const;
  
  const char* data() const;
  
  Character advance();
  Character peek() const;
  
private:
  int64_t current_index;
  int64_t end;
  
  const char* str;
};

class too::Character {
public:
  Character() = default;
  ~Character() = default;
  Character(const Character& other) = default;
  
  explicit Character(char c) {
    zero_units();
    units[0] = c;
  }
  
  //  From (up-to-4) byte sequence.
  explicit Character(const char* str, int n_units) {
    zero_units();
    
    n_units = check_length(n_units);
    
    for (int i = 0; i < n_units; i++) {
      units[i] = str[i];
    }
  }
  
  explicit operator uint32_t() const {
    uint32_t result;
    memcpy(&result, units, size);
    
    return result;
  }
  
  char char_at(int index) const {
    return units[index];
  }
  
  bool char_equals(char c) const {
    return units[0] == c;
  }
  
  bool char_less(char c) const {
    return units[0] < c;
  }
  
  bool is_ascii() const {
    //  I.e., is everything 0 beyond the first byte.
    return units[1] == 0;
  }
  
  bool is_valid() const {
    return count_units() != 0;
  }
  
  int count_units() const {
    return utf8::count_code_units(units, size);
  }
  
  static constexpr int size = utf8::bytes_per_code_point;
  
private:
  void zero_units() {
    std::memset(units, 0, sizeof(units));
  }
  
  int check_length(int len) {
    return (len < 0) ? 0 : (len > size) ? size : len;
  }
  
private:
  char units[4];
};

//  Char ops
inline bool operator==(const too::Character& lhs, char rhs) {
  return lhs.char_equals(rhs);
}

inline bool operator!=(const too::Character& lhs, char rhs) {
  return !lhs.char_equals(rhs);
}

inline bool operator<(const too::Character& lhs, char rhs) {
  return lhs.char_less(rhs);
}

inline bool operator>(const too::Character& lhs, char rhs) {
  return !lhs.char_equals(rhs) && !lhs.char_less(rhs);
}

inline bool operator>=(const too::Character& lhs, char rhs) {
  return lhs.char_equals(rhs) || !lhs.char_less(rhs);
}

inline bool operator<=(const too::Character& lhs, char rhs) {
  return lhs.char_less(rhs) || lhs.char_equals(rhs);
}

//  Character ops
inline bool operator==(const too::Character& lhs, const too::Character& rhs) {
  return uint32_t(lhs) == uint32_t(rhs);
}

inline bool operator!=(const too::Character& lhs, const too::Character& rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const too::Character& lhs, const too::Character& rhs) {
  return uint32_t(lhs) < uint32_t(rhs);
}

inline bool operator>(const too::Character& lhs, const too::Character& rhs) {
  return uint32_t(lhs) > uint32_t(rhs);
}

inline bool operator>=(const too::Character& lhs, const too::Character& rhs) {
  return uint32_t(lhs) >= uint32_t(rhs);
}

inline bool operator<=(const too::Character& lhs, const too::Character& rhs) {
  return uint32_t(lhs) <= uint32_t(rhs);
}
