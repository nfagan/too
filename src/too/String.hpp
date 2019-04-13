//
//  String.hpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#pragma once

#include "Vector.hpp"
#include "Character.hpp"
#include <string>
#include <cstdint>

namespace too {
  using String = std::string;
  using StringView = std::string_view;
  
  StringView make_string_view(const char* str, int64_t offset, int64_t len);
  StringView make_string_view(const String& str, int64_t offset, int64_t len);
  
  bool is_ascii_digit(Character c);
  bool is_ascii_alpha(Character c);
  bool is_ascii_alpha_numeric(Character c);
  bool is_white_space(Character c);
}
