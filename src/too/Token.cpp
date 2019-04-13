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

std::ostream& operator<<(std::ostream& os, const too::TokenType& token_type) {
  switch (token_type) {
    case too::TokenType::COLON:
      os << "COLON";
      break;
      
    case too::TokenType::LEFT_BRACE:
      os << "LEFT_BRACE";
      break;
    
    case too::TokenType::RIGHT_BRACE:
      os << "RIGHT_BRACE";
      break;
      
    case too::TokenType::LEFT_PARENS:
      os << "LEFT_PARENS";
      break;
      
    case too::TokenType::RIGHT_PARENS:
      os << "RIGHT_PARENS";
      break;
      
    case too::TokenType::NUMBER_LITERAL:
      os << "NUMBER_LITERAL";
      break;
      
    case too::TokenType::STRING_LITERAL:
      os << "STRING_LITERAL";
      break;
      
    case too::TokenType::PLUS:
      os << "PLUS";
      break;
      
    case too::TokenType::MINUS:
      os << "MINUS";
      break;
      
    case too::TokenType::STAR:
      os << "STAR";
      break;
      
    case too::TokenType::FORWARD_SLASH:
      os << "FORWARD_SLASH";
      break;
      
    case too::TokenType::END:
      os << "END";
      break;
      
    case too::TokenType::LESS:
      os << "LESS";
      break;
      
    case too::TokenType::LESS_EQUAL:
      os << "LESS_EQUAL";
      break;
      
    case too::TokenType::GREATER:
      os << "GREATER";
      break;
      
    case too::TokenType::GREATER_EQUAL:
      os << "GREATER_EQUAL";
      break;
      
    case too::TokenType::BANG:
      os << "BANG";
      break;
      
    case too::TokenType::BANG_EQUAL:
      os << "BANG_EQUAL";
      break;
      
    case too::TokenType::WHERE:
      os << "WHERE";
      break;
      
    case too::TokenType::IF:
      os << "IF";
      break;
    
    case too::TokenType::ELSE:
      os << "ELSE";
      break;
      
    case too::TokenType::RETURN:
      os << "RETURN";
      break;
      
    case too::TokenType::FOR:
      os << "FOR";
      break;
      
    case too::TokenType::IMPL:
      os << "IMPL";
      break;
      
    case too::TokenType::FN:
      os << "FN";
      break;
      
    case too::TokenType::IDENTIFIER:
      os << "IDENTIFIER";
      break;
      
    case too::TokenType::TRAIT:
      os << "TRAIT";
      break;
      
    case too::TokenType::IN:
      os << "IN";
      break;
      
    case too::TokenType::EQUAL:
      os << "EQUAL";
      break;
      
    case too::TokenType::EQUAL_EQUAL:
      os << "EQUAL_EQUAL";
      break;
      
    case too::TokenType::STRUCT:
      os << "STRUCT";
      break;
  }
  
  return os;
}
