//
//  Token.hpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#pragma once

#include "String.hpp"
#include <cstdint>
#include <iostream>

namespace too {
  enum class TokenType;
  struct Token;
}

std::ostream& operator<<(std::ostream& os, const too::TokenType& token_type);
std::ostream& operator<<(std::ostream& os, const too::Token& token);

namespace too {
  enum class TokenType {
    COLON,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_PARENS,
    RIGHT_PARENS,
    NUMBER_LITERAL,
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
    END
  };
  
  struct Token {
    TokenType type;
    StringView lexeme;
  };
}
