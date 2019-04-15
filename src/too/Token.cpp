//
//  Token.cpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#include "Token.hpp"
#include <map>

std::ostream& operator<<(std::ostream& os, const too::Token& token) {
  os << token.type << ": " << token.lexeme;
  
  return os;
}

std::ostream& operator<<(std::ostream& os, too::TokenType token_type) {
  os << too::to_string(token_type);
  
  return os;
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
    {TokenType::SELF_TYPE, "SELF_TYPE"}
  };
  
  return token_type_to_string.at(token_type);
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

const too::Token& too::TokenIterator::peek_next() const {
  return peek(1);
}

const too::Token& too::TokenIterator::peek(int64_t ahead) const {
  if (next_ind + ahead >= n_tokens) {
    return Token::end();
  }
  
  return tokens[next_ind + ahead];
}
