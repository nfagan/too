//
//  TokenType.hpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#pragma once

#include <ostream>

namespace too {
  enum class TokenType {
    COLON,
    COMMA,
    PERIOD,
    SEMICOLON,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PARENS,
    RIGHT_PARENS,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    PLUS,
    MINUS,
    STAR,
    FORWARD_SLASH,
    BANG,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    TOO_TRUE,
    TOO_FALSE,
    LOGICAL_AND,
    LOGICAL_OR,
    WHERE,
    IF,
    ELSE,
    RETURN,
    FOR,
    IMPL,
    FN,
    IDENTIFIER,
    TRAIT,
    IN,
    EQUAL,
    EQUAL_EQUAL,
    STRUCT,
    LET,
    RIGHT_ARROW,
    SELF,
    SELF_TYPE,
    META_GROUP_BEGIN,
    META_GROUP_END,
    META_OR,
    META_ZERO_OR_MORE,
    META_ONCE_OR_MORE,
    META_ONCE_OR_NEVER,
    END
  };
  
  bool is_meta(TokenType type);
  
  const char* const to_string(TokenType token_type);
  const char* const to_string_symbol(TokenType token_type);
}

std::ostream& operator<<(std::ostream& os, too::TokenType token_type);
