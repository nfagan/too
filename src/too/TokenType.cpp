//
//  TokenType.cpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#include "TokenType.hpp"
#include <map>
#include <cassert>

std::ostream& operator<<(std::ostream& os, too::TokenType token_type) {
  os << too::to_string(token_type);
  
  return os;
}

bool too::is_meta(too::TokenType type) {
  switch (type) {
    case TokenType::META_OR:
    case TokenType::META_GROUP_BEGIN:
    case TokenType::META_GROUP_END:
    case TokenType::META_ZERO_OR_MORE:
    case TokenType::META_ONCE_OR_MORE:
    case TokenType::META_ONCE_OR_NEVER:
      return true;
    default:
      return false;
  }
  
  assert(false);
}

const char* const too::to_string(too::TokenType token_type) {
  static const std::map<too::TokenType, const char* const> token_type_to_string{
    {TokenType::COLON, "COLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::PERIOD, "PERIOD"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::LEFT_BRACE, "LEFT_BRACE"},
    {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
    {TokenType::LEFT_PARENS, "LEFT_PARENS"},
    {TokenType::RIGHT_PARENS, "RIGHT_PARENS"},
    {TokenType::LEFT_BRACKET, "LEFT_BRACKET"},
    {TokenType::RIGHT_BRACKET, "RIGHT_BRACKET"},
    {TokenType::INT_LITERAL, "INT_LITERAL"},
    {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
    {TokenType::STRING_LITERAL, "STRING_LITERAL"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::STAR, "STAR"},
    {TokenType::FORWARD_SLASH, "FORWARD_SLASH"},
    {TokenType::END, "END"},
    {TokenType::LESS, "LESS"},
    {TokenType::LESS_EQUAL, "LESS_EQUAL"},
    {TokenType::GREATER, "GREATER"},
    {TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
    {TokenType::BANG, "BANG"},
    {TokenType::BANG_EQUAL, "BANG_EQUAL"},
    {TokenType::WHERE, "WHERE"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::RETURN, "RETURN"},
    {TokenType::FOR, "FOR"},
    {TokenType::IMPL, "IMPL"},
    {TokenType::FN, "FN"},
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::TRAIT, "TRAIT"},
    {TokenType::IN, "IN"},
    {TokenType::EQUAL, "EQUAL"},
    {TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
    {TokenType::STRUCT, "STRUCT"},
    {TokenType::LET, "LET"},
    {TokenType::RIGHT_ARROW, "RIGHT_ARROW"},
    {TokenType::SELF, "SELF"},
    {TokenType::SELF_TYPE, "SELF_TYPE"},
    {TokenType::META_ZERO_OR_MORE, "META_ZERO_OR_MORE"},
    {TokenType::META_ONCE_OR_MORE, "META_ONCE_OR_MORE"},
    {TokenType::META_ONCE_OR_NEVER, "META_ONCE_OR_NEVER"},
    {TokenType::META_GROUP_END, "META_GROUP_END"},
    {TokenType::META_GROUP_BEGIN, "META_GROUP_BEGIN"},
    {TokenType::META_OR, "META_OR"},
  };
  
  return token_type_to_string.at(token_type);
}
