//
//  SyntaxParser.cpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#include "SyntaxParser.hpp"
#include "Ast.hpp"
#include "TokenNfa.hpp"
#include <functional>
#include <iostream>
#include <cstring>

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

template <typename T, typename... Args>
inline Optional<ast::BoxedExpr> make_optional_boxed_expr(Args&&... args) {
  return Optional<ast::BoxedExpr>(std::make_unique<T>(std::forward<Args>(args)...));
}

inline bool is_unary_token_type(const TokenType token_type) {
  return token_type == TokenType::BANG || token_type == TokenType::MINUS;
}

inline bool is_binary_token_type(const TokenType token_type) {
  switch (token_type) {
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::STAR:
    case TokenType::FORWARD_SLASH:
      return true;
    default:
      return false;
  }
}

inline bool is_literal_token_type(const TokenType token_type) {
  return (token_type == TokenType::INT_LITERAL ||
          token_type == TokenType::FLOAT_LITERAL ||
          token_type == TokenType::STRING_LITERAL);
}

template <typename T>
struct StringToNumber{};

template<>
struct StringToNumber<int32_t> {
  static inline int32_t convert(const std::string& str) {
    std::size_t idx;
    return std::stol(str, &idx);
  }
};

template<>
struct StringToNumber<int64_t> {
  static inline int32_t convert(const std::string& str) {
    std::size_t idx;
    return std::stoll(str, &idx);
  }
};

template<>
struct StringToNumber<float> {
  static inline float convert(const std::string& str) {
    std::size_t idx;
    return std::stof(str, &idx);
  }
};

template<>
struct StringToNumber<double> {
  static inline float convert(const std::string& str) {
    std::size_t idx;
    return std::stod(str, &idx);
  }
};

inline Optional<TooInteger> try_parse_int(const StringView& view) {
  if (view.empty()) {
    return NullOpt{};
  }
  
  auto str = too::to_string(view);
  
  try {
    return Optional<TooInteger>(StringToNumber<TooInteger>::convert(str));
  } catch (...) {
    return NullOpt{};
  }
}

inline Optional<TooFloat> try_parse_float(const StringView& view) {
  static_assert(std::is_same_v<TooFloat, double>, "Expected double as underlying float type.");
  
  if (view.empty()) {
    return NullOpt{};
  }
  
  auto str = too::to_string(view);
  
  try {
    return Optional<TooFloat>(StringToNumber<TooFloat>::convert(str));
  } catch (...) {
    return NullOpt{};
  }
}

inline bool consume_token(TokenIterator& iterator, SyntaxParseResult& result, const TokenType expected_type) {
  if (iterator.peek().type != expected_type) {
    //  @FIXME: Error handle
    return false;
  }
  
  iterator.advance(1);
  
  return true;
}

Optional<ast::TypeParameter> type_parameter(TokenIterator& iterator, SyntaxParseResult& result) {
  auto next = iterator.peek();
  
  if (next.type != TokenType::IDENTIFIER) {
    return NullOpt{};
  }
  
  ast::TypeParameter type_param;
  type_param.name = next.lexeme;
  iterator.advance(1);
  
  if (iterator.peek().type == TokenType::LESS) {
    iterator.advance(1);
    
    Vector<ast::TypeParameter> children;
    
    while (iterator.has_next()) {
      auto type_params_result = type_parameter(iterator, result);
      
      if (!type_params_result) {
        return NullOpt{};
      }
      
      children.push_back(type_params_result.rvalue());
      
      if (iterator.peek().type != TokenType::COMMA) {
        break;
      }
      
      iterator.advance(1);
    }
    
    if (!consume_token(iterator, result, TokenType::GREATER)) {
      return NullOpt{};
    }
    
    type_param.parameters = std::move(children);
  }
  
  return Optional<ast::TypeParameter>(std::move(type_param));
}

Optional<Vector<ast::TypeParameter>> type_parameters(TokenIterator& iterator, SyntaxParseResult& result) {
  Vector<ast::TypeParameter> res;
  
  while (iterator.has_next()) {
    auto type_param_result = type_parameter(iterator, result);
    
    if (!type_param_result) {
      return NullOpt{};
    }
    
    res.push_back(type_param_result.rvalue());
    
    if (iterator.peek().type != TokenType::COMMA) {
      break;
    }
    
    iterator.advance(1);
  }
  
  return Optional<Vector<ast::TypeParameter>>(std::move(res));
}


Optional<ast::Identifier> optionally_typed_identifier(TokenIterator& iterator,
                                                      SyntaxParseResult& result,
                                                      const bool allow_untyped) {
  if (iterator.peek().type != TokenType::IDENTIFIER) {
    return NullOpt{};
  }
  
  ast::Identifier identifier;
  identifier.name = iterator.peek().lexeme;
  iterator.advance(1);
  
  if (iterator.peek().type == TokenType::COLON) {
    iterator.advance(1);
    
    auto type_param_result = type_parameter(iterator, result);
    
    if (!type_param_result) {
      return NullOpt{};
    }
    
    identifier.type = type_param_result.rvalue();
  } else if (!allow_untyped) {
    //  Type must be specified
    return NullOpt{};
  }
  
  return Optional<ast::Identifier>(std::move(identifier));
}

inline Optional<ast::Identifier> explicitly_typed_identifier(TokenIterator& iterator,
                                                             SyntaxParseResult& result) {
  return optionally_typed_identifier(iterator, result, false);
}

Optional<Vector<ast::Identifier>> explicitly_typed_identifiers(TokenIterator& iterator,
                                                               SyntaxParseResult& result) {
  
  Vector<ast::Identifier> identifiers;
  
  while (iterator.has_next()) {
    auto identifier_result = explicitly_typed_identifier(iterator, result);
    
    if (!identifier_result) {
      return NullOpt{};
    }
    
    identifiers.push_back(identifier_result.rvalue());
    
    if (iterator.peek().type != TokenType::COMMA) {
      break;
    }
    
    iterator.advance(1);  // Consume ,
  }
  
  return Optional<decltype(identifiers)>(std::move(identifiers));
}

Optional<Vector<ast::Identifier>> explicitly_typed_identifier_sequence(TokenIterator& iterator,
                                                                       SyntaxParseResult& result,
                                                                       const TokenType stop_token) {
  if (iterator.peek().type == stop_token) {
    //  Empty sequence.
    iterator.advance(1);
    
    return Optional<Vector<ast::Identifier>>(Vector<ast::Identifier>());
  }
  
  auto identifiers = explicitly_typed_identifiers(iterator, result);
  
  if (!consume_token(iterator, result, stop_token)) {
    return NullOpt{};
  }
  
  return identifiers;
}

Optional<Vector<ast::Identifier>> function_arguments(TokenIterator& iterator, SyntaxParseResult& result) {
  return explicitly_typed_identifier_sequence(iterator, result, TokenType::RIGHT_PARENS);
}

Optional<Vector<ast::Identifier>> struct_members(TokenIterator& iterator, SyntaxParseResult& result) {
  return explicitly_typed_identifier_sequence(iterator, result, TokenType::RIGHT_BRACE);
}

//#define USE_TOKEN_NFA

Optional<ast::TraitBoundedType> trait_bounded_type(TokenIterator& iterator, SyntaxParseResult& result) {
#ifdef USE_TOKEN_NFA
  TokenType start_sequence[2] = {TokenType::IDENTIFIER, TokenType::COLON};
  TokenRegex pattern(&start_sequence[0], 2);
  
  if (!pattern.matches_iterator(iterator, 2)) {
    return NullOpt{};
  }
#else
  
  if (iterator.peek().type != TokenType::IDENTIFIER ||
      iterator.peek(1).type != TokenType::COLON) {
    return NullOpt{};
  }
  
#endif
  
  ast::TraitBoundedType bounded_type;
  bounded_type.type = iterator.next().lexeme;

  iterator.advance(1);  //  Skip colon.
  
  const auto& next = iterator.peek();
  
  if (next.type == TokenType::IDENTIFIER) {    
    bounded_type.traits.push_back({next.lexeme});
    iterator.advance(1);
    
  } else if (next.type != TokenType::LEFT_BRACE) {
    return NullOpt{};
    
  } else {
    //  Sequence of types; consume {
    iterator.advance(1);
    
    auto comma_result = type_parameters(iterator, result);
    
    if (!comma_result) {
      return NullOpt{};
    }
    
    if (!consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
      return NullOpt{};
    }
    
    bounded_type.traits = comma_result.rvalue();
  }
  
  return Optional<decltype(bounded_type)>(std::move(bounded_type));
}

Optional<ast::WhereClause> where_clause(TokenIterator& iterator, SyntaxParseResult& result) {
  ast::WhereClause clause;
  
  while (iterator.has_next()) {
    auto trait_bound_result = trait_bounded_type(iterator, result);
    
    if (!trait_bound_result) {
      return NullOpt{};
    }
    
    const auto& val = trait_bound_result.value();
    
    if (!val.traits.empty()) {
      clause.types.push_back(trait_bound_result.rvalue());
    }
    
    if (iterator.peek().type != TokenType::COMMA) {
      break;
    }
    
    iterator.advance(1);
  }
  
  if (clause.types.empty()) {
    return NullOpt{};
  }
  
  return Optional<decltype(clause)>(std::move(clause));
}

template <typename T>
inline bool check_assign_where(TokenIterator& iterator,
                               SyntaxParseResult& result,
                               T& assign_to) {
  
  auto next = iterator.peek();
  
  if (next.type == TokenType::WHERE) {
    iterator.advance(1);  //  consume where
    
    auto where_result = where_clause(iterator, result);
    
    if (!where_result) {
      return false;
    }
    
    assign_to.where_clause = std::move(where_result);
  }
  
  return true;
}

template <typename T>
inline bool check_assign_type_parameters(TokenIterator& iterator,
                                         SyntaxParseResult& result,
                                         T& assign_to) {
  auto next = iterator.peek();
  
  if (next.type == TokenType::LESS) {
    iterator.advance(1);  //  Consume <
    
    auto maybe_type_parameters = type_parameters(iterator, result);
    
    if (!maybe_type_parameters || !consume_token(iterator, result, TokenType::GREATER)) {
      return false;
    }
    
    assign_to.type_parameters = maybe_type_parameters.rvalue();
  }
  
  return true;
}

template <typename T>
inline bool check_assign_name_and_type_parameters(TokenIterator& iterator,
                                                  SyntaxParseResult& result,
                                                  T& assign_to) {
  
  auto next = iterator.peek();
  if (next.type != TokenType::IDENTIFIER) {
    //  @FIXME: Error, missing name
    return false;
  }
  
  assign_to.name = next.lexeme;
  iterator.advance(1);
  
  if (!check_assign_type_parameters(iterator, result, assign_to)) {
    return false;
  }
  
  return true;
}

Optional<ast::FunctionDefinition> function_definition(TokenIterator& iterator, SyntaxParseResult& result) {
  too::ast::FunctionDefinition function_def;
  
  if (!check_assign_name_and_type_parameters(iterator, result, function_def)) {
    return NullOpt{};
  }
  
  if (!consume_token(iterator, result, TokenType::LEFT_PARENS)) {
    //  @FIXME: Expected left parens
    return NullOpt{};
  }
  
  //  Arguments
  auto maybe_arguments = function_arguments(iterator, result);
  
  if (!maybe_arguments) {
    return NullOpt{};
  }
  
  function_def.input_parameters = std::move(maybe_arguments.rvalue());
  
  if (!consume_token(iterator, result, TokenType::RIGHT_ARROW)) {
    //  @FIXME: Expect right_arrow for return type.
    return NullOpt{};
  }
  
  auto next = iterator.peek();
  if (next.type != TokenType::IDENTIFIER) {
    //  @FIXME: Expect identifier. (return type).
    return NullOpt{};
  }
  
  function_def.return_type.name = next.lexeme;
  iterator.advance(1);
  
  //  (Optional) Where clause
  if (!check_assign_where(iterator, result, function_def)) {
    return NullOpt{};
  }
  
  if (iterator.peek().type != TokenType::SEMICOLON) {
    //  Expect function body.
    if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
      //  @FIXME: Expect left brace (opening to function body)
      return NullOpt{};
    }
  } else {
    iterator.advance(1);  //  Consume ;
  }
  
  return Optional<ast::FunctionDefinition>(std::move(function_def));
}

void struct_definition(TokenIterator& iterator, SyntaxParseResult& result) {
  too::ast::StructDefinition struct_def;
  
  if (!check_assign_name_and_type_parameters(iterator, result, struct_def)) {
    return;
  }
  
  //  Has where clause ?
  if (!check_assign_where(iterator, result, struct_def)) {
    return;
  }
  
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    //  @FIXME: Expected left brace
    return;
  }
  
  //  Arguments
  auto maybe_members = struct_members(iterator, result);
  
  if (!maybe_members) {
    //  Error should be marked in struct_members
    return;
  }
  
  struct_def.members = maybe_members.rvalue();
  
  result.structs.push_back(struct_def);
}

void trait_definition(TokenIterator& iterator, SyntaxParseResult& result) {
  too::ast::TraitDefinition trait_def;
  
  if (!check_assign_name_and_type_parameters(iterator, result, trait_def)) {
    return;
  }
  
  //  Has where clause ?
  if (!check_assign_where(iterator, result, trait_def)) {
    return;
  }
  
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    //  @FIXME: Expected left brace
    return;
  }
  
  while (iterator.has_next() && iterator.peek().type != TokenType::RIGHT_BRACE) {
    if (!consume_token(iterator, result, TokenType::FN)) {
      //  @FIXME: Expect function.
      return;
    }
    
    auto func_result = function_definition(iterator, result);
    
    if (!func_result) {
      return;
    }
    
    trait_def.functions.push_back(func_result.rvalue());
  }
  
  if (!consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
    return;
  }
  
  result.traits.push_back(std::move(trait_def));
}

Optional<ast::BoxedExpr> expression(TokenIterator& iterator, SyntaxParseResult& result) {
  const auto next = iterator.next();
  
  if (is_unary_token_type(next.type)) {
    auto operator_token = next.type;
    auto sub_expression = expression(iterator, result);
    
    if (!sub_expression) {
      return NullOpt{};
    }
    
    auto unary_expr = make_optional_boxed_expr<ast::UnaryExpr>(operator_token, sub_expression.rvalue());
    
    if (is_binary_token_type(iterator.peek().type)) {
      auto binary_operator = iterator.next().type;
      
      auto rhs = expression(iterator, result);
      
      if (!rhs) {
        return NullOpt{};
      }
      
      return make_optional_boxed_expr<ast::BinaryExpr>(binary_operator, unary_expr.rvalue(), rhs.rvalue());
    } else {
      return unary_expr;
    }
    
  } else if (is_literal_token_type(next.type)) {
    if (next.type == TokenType::INT_LITERAL) {
      auto parsed_int = try_parse_int(next.lexeme);
      
      if (!parsed_int) {
        return NullOpt{};
      }
      
      return make_optional_boxed_expr<ast::IntLiteralExpr>(parsed_int.value());
      
    } else if (next.type == TokenType::FLOAT_LITERAL) {
      auto parsed_float = try_parse_float(next.lexeme);
      
      if (!parsed_float) {
        return NullOpt{};
      }
      
      return make_optional_boxed_expr<ast::FloatLiteralExpr>(parsed_float.value());
    }
  }
  
  return NullOpt{};
}

void let_statement(TokenIterator& iterator, SyntaxParseResult& result) {
  ast::LetStatement let_stmt;
  
  //  true -> allow untyped.
  auto identifier = optionally_typed_identifier(iterator, result, true);
  
  if (!identifier) {
    return;
  }
  
  if (iterator.peek().type == TokenType::EQUAL) {
    iterator.advance(1);  //  consume =
    
    auto initializer = expression(iterator, result);
    
    if (!initializer) {
      return;
    }
    
    let_stmt.initializer = std::move(initializer);
  }
  
  let_stmt.identifier = identifier.rvalue();
  
  std::cout << let_stmt.to_string() << std::endl;
}

void block_statement(TokenIterator& iterator, SyntaxParseResult& result) {
  const auto& token = iterator.next();
  
  if (token.type == TokenType::FN) {
    auto func_res = function_definition(iterator, result);
    
    if (func_res) {
      result.functions.push_back(func_res.rvalue());
    }
    
  } else if (token.type == TokenType::STRUCT) {
    struct_definition(iterator, result);
    
  } else if (token.type == TokenType::TRAIT) {
    trait_definition(iterator, result);
    
  } else if (token.type == TokenType::LET) {
    let_statement(iterator, result);
  }
}

SyntaxParseResult parse_syntax(const too::Vector<too::Token>& tokens) {
  SyntaxParseResult result;
  
  too::TokenIterator iterator(tokens.data(), tokens.size());
  
  while (iterator.has_next()) {
    block_statement(iterator, result);
  }
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
