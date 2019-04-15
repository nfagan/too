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

std::ostream& operator<<(std::ostream& os, too::TokenType token_type);
std::ostream& operator<<(std::ostream& os, const too::Token& token);

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
    END
  };
  
  struct Token {
    TokenType type;
    StringView lexeme;
    
    static const Token& end();
  };
  
  class TokenIterator {
  public:
    TokenIterator(const Token* tokens, int64_t n_tokens);
    ~TokenIterator() = default;
    
    const Token& peek() const;
    const Token& peek_next() const;
    const Token& peek(int64_t ahead) const;
    
    const Token& next();
    void advance(int64_t n_places);
    
    bool has_next() const;
    int64_t next_index() const;
    
  private:
    const Token* tokens;
    int64_t n_tokens;
    
    int64_t next_ind;
  };
  
  const char* const to_string(TokenType token_type);
}
