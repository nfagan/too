//
//  Token.hpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#pragma once

#include "TokenType.hpp"
#include "String.hpp"
#include <cstdint>
#include <ostream>

namespace too {
  struct Token;
  class TokenIterator;
}

struct too::Token {
  TokenType type;
  StringView lexeme;
  
  static const Token& end();
};

class too::TokenIterator {
public:
  TokenIterator(const Token* tokens, int64_t n_tokens);
  ~TokenIterator() = default;
  
  const Token& peek() const;
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

std::ostream& operator<<(std::ostream& os, const too::Token& token);
