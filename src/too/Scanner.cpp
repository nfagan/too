//
//  Scanner.cpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#include "Scanner.hpp"
#include <map>
#include <cassert>

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

std::ostream& operator<<(std::ostream& os, const too::ScanErrorType& type) {
  switch (type) {
    case too::ScanErrorType::UNTERMINATED_STRING:
      os << "UNTERMINATED_STRING";
      break;
    case too::ScanErrorType::UNRECOGNIZED_SYMBOL:
      os << "UNRECOGNIZED_SYMBOL";
      break;
  }
  
  return os;
}

//  Auto-indent gets funky when the entire file is wrapped in a namespace.
BEGIN_NAMESPACE

namespace {
  static std::map<char, TokenType> SINGLE_CHAR_TO_TOKEN_TYPE{
    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'!', TokenType::BANG},
    {'(', TokenType::LEFT_PARENS},
    {')', TokenType::RIGHT_PARENS},
    {'{', TokenType::LEFT_BRACE},
    {'}', TokenType::RIGHT_BRACE},
    {'[', TokenType::LEFT_BRACKET},
    {']', TokenType::RIGHT_BRACKET},
    {'<', TokenType::LESS},
    {'>', TokenType::GREATER},
    {'*', TokenType::STAR},
    {'/', TokenType::FORWARD_SLASH},
    {':', TokenType::COLON},
    {',', TokenType::COMMA},
    {'.', TokenType::PERIOD},
    {'=', TokenType::EQUAL},
    {';', TokenType::SEMICOLON}
  };
  
  static std::map<char, TokenType> COMPOUND_PUNCTUATION_START_TO_TOKEN_TYPE{
    {'!', TokenType::BANG_EQUAL},
    {'<', TokenType::LESS_EQUAL},
    {'>', TokenType::GREATER_EQUAL},
    {'=', TokenType::EQUAL_EQUAL},
    {'-', TokenType::RIGHT_ARROW}
  };
  
  static std::map<StringView, TokenType> KEYWORD_TO_TOKEN_TYPE{
    {"fn", TokenType::FN},
    {"return", TokenType::RETURN},
    {"where", TokenType::WHERE},
    {"for", TokenType::FOR},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"impl", TokenType::IMPL},
    {"trait", TokenType::TRAIT},
    {"in", TokenType::IN},
    {"struct", TokenType::STRUCT},
    {"let", TokenType::LET},
    {"self", TokenType::SELF},
    {"Self", TokenType::SELF_TYPE},
    {"true", TokenType::TOO_TRUE},
    {"false", TokenType::TOO_FALSE},
    {"type", TokenType::TYPE},
    {"use", TokenType::USE},
    {"mod", TokenType::MOD},
    {"lib", TokenType::LIB}
  };
}

inline bool is_recognized_single_character(const Character& c) {
  return c.is_ascii() && SINGLE_CHAR_TO_TOKEN_TYPE.count(char(c)) > 0;
}

inline bool is_recognized_compound_punctuation_start(const Character& c) {
  return c.is_ascii() && COMPOUND_PUNCTUATION_START_TO_TOKEN_TYPE.count(char(c)) > 0;
}

inline bool is_recognized_keyword(StringView view) {
  return KEYWORD_TO_TOKEN_TYPE.count(view) > 0;
}

inline TokenType get_single_character_token_type(const Character& c) {
  return SINGLE_CHAR_TO_TOKEN_TYPE.at(char(c));
}

inline TokenType get_single_character_with_equal_token_type(const Character& c) {
  return COMPOUND_PUNCTUATION_START_TO_TOKEN_TYPE.at(char(c));
}

inline TokenType get_keyword_token_type(StringView view) {
  return KEYWORD_TO_TOKEN_TYPE.at(view);
}

inline int64_t current_index(const CharacterIterator& iterator, const Character& last_char) {
  return iterator.next_index() - last_char.count_units();
}

inline Token make_token(const CharacterIterator& iterator, const Character& last_char, TokenType type, int64_t len) {
  return Token{type, make_string_view(iterator.data(), current_index(iterator, last_char), len)};
}

inline Token make_token(const CharacterIterator& iterator, TokenType type, int64_t start, int64_t len) {
  return Token{type, make_string_view(iterator.data(), start, len)};
}

inline ScanError make_unrecognized_symbol_error(CharacterIterator& iterator, const Character& last_char) {
  const int64_t begin = current_index(iterator, last_char);
  const int64_t end = begin + 1;
  
  return {ScanErrorType::UNRECOGNIZED_SYMBOL, begin, end};
}

inline int64_t consume_digits(CharacterIterator& iterator, bool* ended_on_decimal) {
  int64_t n_digits = 0;
  *ended_on_decimal = false;
  
  while (iterator.has_next()) {
    auto c = iterator.peek();
    
    if (!is_ascii_digit(c)) {
      if (c == '.') {
        *ended_on_decimal = true;
        //  consume '.'
        iterator.advance();
      }
      
      break;
    }
    
    n_digits++;
    iterator.advance();
  }
  
  return n_digits;
}

void number_literal(CharacterIterator& iterator, ScanResult& result, const Character& last_char) {
  int64_t number_str_len = 1;
  int64_t number_start = current_index(iterator, last_char);
  TokenType number_literal_type = TokenType::INT_LITERAL;
  
  bool ended_on_decimal;
  
  number_str_len += consume_digits(iterator, &ended_on_decimal);
  
  if (ended_on_decimal) {
    number_str_len += (consume_digits(iterator, &ended_on_decimal) + 1);
    number_literal_type = TokenType::FLOAT_LITERAL;
  }
  
  result.tokens.push_back(make_token(iterator, number_literal_type, number_start, number_str_len));
}

void string_literal(CharacterIterator& iterator, ScanResult& result, const Character& last_char) {
  int64_t n_bytes = 0;
  int64_t str_start = iterator.next_index();
  
  bool previous_escape = false;
  bool closed_string = false;
  
  while (iterator.has_next()) {
    auto c = iterator.peek();
    
    //  Allow escaped \" quotes
    if (c == '"' && !previous_escape) {
      iterator.advance();
      closed_string = true;
      break;
    }
    
    n_bytes += c.count_units();
    iterator.advance();
    previous_escape = c == '\\';
  }
  
  if (!closed_string) {
    //  Error: missing closing '"'
    result.errors.push_back({ScanErrorType::UNTERMINATED_STRING, str_start, iterator.next_index()});
    
  } else {
    result.tokens.push_back(make_token(iterator, TokenType::STRING_LITERAL, str_start, n_bytes));
  }
}

inline void identifier(CharacterIterator& iterator, ScanResult& result, const Character& last_char) {
  const int64_t begin = current_index(iterator, last_char);
  int64_t identifier_n_bytes = last_char.count_units();
  
  while (iterator.has_next()) {
    auto c = iterator.peek();
    
    if (!is_ascii_alpha_numeric(c) && c != '_') {
      break;
    }
    
    identifier_n_bytes += c.count_units();
    iterator.advance();
  }
  
  auto identifier_view = make_string_view(iterator.data(), begin, identifier_n_bytes);
  auto token_type = TokenType::IDENTIFIER;
  
  if (is_recognized_keyword(identifier_view)) {
    token_type = get_keyword_token_type(identifier_view);
  }
  
  result.tokens.push_back(make_token(iterator, token_type, begin, identifier_n_bytes));
}

bool is_compound_punctuation(const CharacterIterator& iterator, const Character& last_char) {
  auto next_char = iterator.peek();
  
  if (last_char == ':' && (next_char == ':' || next_char == '=')) {
    return true;
  }
  
  //  compound punctuation: !=, ==, ->, etc.
  if ((next_char != '=' && next_char != '>') || !is_recognized_compound_punctuation_start(last_char)) {
    return false;
  }
  
  if (next_char == '>' && last_char != '-') {
    return false;
  } else if (next_char == '=' && last_char == '-') {
    return false;
  } else {
    return true;
  }
}

inline TokenType get_compound_punctuation_token_type(const Character& current, const Character& next) {
  if (current != ':') {
    return get_single_character_with_equal_token_type(current);
  } else if (next == ':') {
    return TokenType::DOUBLE_COLON;
  } else if (next == '=') {
    return TokenType::COLON_EQUAL;
  } else {
    assert(false);
    return TokenType::END;
  }
}

void punctuation(CharacterIterator& iterator, ScanResult& result, const Character& c) {
  auto token_type = get_single_character_token_type(c);
  int64_t n_characters = 1;
  bool is_compound_punct = is_compound_punctuation(iterator, c);
  
  if (is_compound_punct) {
    token_type = get_compound_punctuation_token_type(c, iterator.peek());
    n_characters = 2;
  }
  
  result.tokens.push_back(make_token(iterator, c, token_type, n_characters));
  
  if (is_compound_punct) {
    iterator.advance();
  }
}

void comment(CharacterIterator& iterator, ScanResult& result, const Character& last_char) {
  while (iterator.has_next()) {
    auto c = iterator.peek();
    
    if (c == '\n') {
      break;
    }
    
    iterator.advance();
  }
}

inline void each_scan_iteration(CharacterIterator& iterator, ScanResult& result) {
  auto c = iterator.advance();
  
  if (is_white_space(c)) {
    //
  } else if (is_recognized_single_character(c)) {
    if (c == '/' && iterator.peek() == '/') {
      comment(iterator, result, c);
      
    } else {
      punctuation(iterator, result, c);
    }
    
  } else if (c == '"') {
    string_literal(iterator, result, c);
    
  } else if (is_ascii_digit(c)) {
    number_literal(iterator, result, c);
    
  } else if (is_ascii_alpha(c)) {
    identifier(iterator, result, c);
    
  } else {
    //  syntax error: Unrecognized token.
    result.errors.push_back(make_unrecognized_symbol_error(iterator, c));
  }
}

ScanResult scan(const String& code) {
  return scan(code.c_str(), code.size());
}

ScanResult scan(const char* code, int64_t len) {
  ScanResult result;
  
  CharacterIterator iterator(code, len);
  
  while (iterator.has_next()) {
    each_scan_iteration(iterator, result);
  }
  
  result.tokens.push_back(make_token(iterator, Character('\0'), TokenType::END, 0));
  result.had_error = result.errors.size() > 0;
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
