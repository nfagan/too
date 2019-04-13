//
//  String.cpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#include "String.hpp"

too::StringView too::make_string_view(const char* str, int64_t offset, int64_t len) {
  return std::string_view(str + offset, len);
}

too::StringView too::make_string_view(const too::String& str, int64_t offset, int64_t len) {
  return std::string_view(str.c_str() + offset, len);
}

bool too::is_white_space(too::Character c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool too::is_ascii_digit(too::Character c) {
  return c.is_ascii() && c >= '0' && c <= '9';
}

bool too::is_ascii_alpha(Character c) {
  return c.is_ascii() && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}
