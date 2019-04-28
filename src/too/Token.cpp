//
//  Token.cpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#include "Token.hpp"

std::ostream& operator<<(std::ostream& os, const too::Token& token) {
  os << token.type << ": " << token.lexeme;
  
  return os;
}

//
//
//  Token

const too::Token& too::Token::end() {
  static const too::Token end_token{too::TokenType::END, too::StringView(nullptr, 0)};
  
  return end_token;
}

//
//
//  TokenIterator

too::TokenIterator::TokenIterator(const too::Token* tokens, int64_t n_tokens) :
  tokens(tokens), n_tokens(n_tokens), next_ind(0) {
  //
}

bool too::TokenIterator::has_next() const {
  return next_ind < n_tokens;
}

int64_t too::TokenIterator::next_index() const {
  return next_ind;
}

const too::Token& too::TokenIterator::next() {
  if (next_ind >= n_tokens) {
    return Token::end();
  }
  
  return tokens[next_ind++];
}

void too::TokenIterator::advance(int64_t n_places) {
  next_ind += n_places;
}

const too::Token& too::TokenIterator::peek() const {
  return peek(0);
}

const too::Token& too::TokenIterator::peek(int64_t ahead) const {
  auto ind = next_ind + ahead;
  
  if (ind < 0 || ind >= n_tokens) {
    return Token::end();
  }
  
  return tokens[next_ind + ahead];
}
