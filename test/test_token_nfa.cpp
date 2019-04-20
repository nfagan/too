//
//  test_token_nfa.cpp
//  too
//
//  Created by Nick Fagan on 4/19/19.
//

#include "test_token_nfa.hpp"
#include "too/Scanner.hpp"
#include "too/TokenNfa.hpp"

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

bool check_literal() {
  String literal = "fn another();";
  auto res = scan(literal);
  
  Vector<TokenType> literal_pattern;
  literal_pattern.push_back(TokenType::FN);
  literal_pattern.push_back(TokenType::IDENTIFIER);
  literal_pattern.push_back(TokenType::LEFT_PARENS);
  literal_pattern.push_back(TokenType::RIGHT_PARENS);
  literal_pattern.push_back(TokenType::SEMICOLON);
  
  return matches(literal_pattern, res.tokens);
}

bool check_function() {
  Vector<TokenType> pattern;
  
  pattern.push_back(TokenType::FN);
  pattern.push_back(TokenType::IDENTIFIER);
  pattern.push_back(TokenType::META_GROUP_BEGIN);
  pattern.push_back(TokenType::LESS);
  pattern.push_back(TokenType::META_GROUP_BEGIN);
  pattern.push_back(TokenType::IDENTIFIER);
  pattern.push_back(TokenType::META_GROUP_END);
  pattern.push_back(TokenType::META_ONCE_OR_MORE);
  pattern.push_back(TokenType::META_GROUP_BEGIN);
  pattern.push_back(TokenType::COMMA);
  pattern.push_back(TokenType::IDENTIFIER);
  pattern.push_back(TokenType::META_GROUP_END);
  pattern.push_back(TokenType::META_ZERO_OR_MORE);
  pattern.push_back(TokenType::GREATER);
  pattern.push_back(TokenType::META_GROUP_END);
  
  auto scan_result = scan("fn another<T>");
  bool result = matches(pattern, scan_result.tokens);
  
  if (!result) {
    return result;
  }
  
  //  Shouldn't match.
  scan_result = scan("fn another<>");
  result = matches(pattern, scan_result.tokens);
  
  if (result) {
    return false;
  }
  
  return true;
}

bool check_meta() {
  Vector<TokenType> pattern;
  
  pattern.push_back(TokenType::IDENTIFIER);
  pattern.push_back(TokenType::META_ONCE_OR_MORE);
  pattern.push_back(TokenType::SEMICOLON);
  
  auto scan_result = scan("a a a a b ;");
  
  if (!matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  scan_result = scan(";");
  
  if (matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  pattern[1] = TokenType::META_ZERO_OR_MORE;
  
  if (!matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  pattern[1] = TokenType::META_ONCE_OR_NEVER;
  
  if (!matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  scan_result = scan("a ;");
  
  if (!matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  scan_result = scan("a a ;");
  
  if (matches(pattern, scan_result.tokens)) {
    return false;
  }

  return true;
}

bool check_meta_or() {
  auto scan_result = scan("fn");
  
  Vector<TokenType> pattern;
  pattern.push_back(TokenType::FN);
  pattern.push_back(TokenType::META_OR);
  pattern.push_back(TokenType::IDENTIFIER);
  
  if (!too::matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  decltype(pattern) pattern2;
  pattern2.push_back(TokenType::IDENTIFIER);
  pattern2.push_back(TokenType::META_OR);
  pattern2.push_back(TokenType::STRING_LITERAL);
  
  if (too::matches(pattern2, scan_result.tokens)) {
    return false;
  }
  
  return true;
}

bool check_empty() {
  auto scan_result = scan("");
  
  Vector<TokenType> pattern;
  pattern.push_back(TokenType::FN);
  pattern.push_back(TokenType::SEMICOLON);
  
  if (too::matches(pattern, scan_result.tokens)) {
    return false;
  }
  
  if (too::matches(Vector<TokenType>(), scan_result.tokens)) {
    return false;
  }
  
  return true;
}

TestCase make_check_meta() {
  return make_test_case(&check_meta, "Meta sequence matched.", "Meta sequence did not match.");
}

TestCase make_check_meta_or() {
  return make_test_case(&check_meta_or, "Meta or sequence matched.", "Meta or sequence did not match.");
}

TestCase make_check_literal() {
  return make_test_case(&check_literal, "Literal matched.", "Literal did not match.");
}

TestCase make_check_function() {
  return make_test_case(&check_function, "Function matched.", "Function did not match.");
}

TestCase make_check_empty() {
  return make_test_case(&check_empty, "Empty inputs succeeded.", "Empty inputs failed.");
}

TestSuite get_token_nfa_test_suite() {
  TestSuite test_suite;
  
  test_suite.start = DisplayMessage("====\nTest token nfa\n====");
  
  test_suite.add_test(make_check_literal());
  test_suite.add_test(make_check_function());
  test_suite.add_test(make_check_meta());
  test_suite.add_test(make_check_meta_or());
  test_suite.add_test(make_check_empty());
  
  return test_suite;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
