//
//  SyntaxParser.cpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#include "SyntaxParser.hpp"
#include "Ast.hpp"
#include "TokenNFA.hpp"
#include <functional>

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

bool expect_sequence(const TokenIterator& iterator,
                     const TokenType* token_types,
                     int64_t n_tokens,
                     const std::function<void(const Token&)>& error_handler) {
  
  for (int64_t i = 0; i < n_tokens; i++) {
    const auto& token = iterator.peek(i);
    
    if (token.type != token_types[i]) {
      error_handler(token);
      
      return false;
    }
  }
  
  return true;
}

template <typename T>
Optional<Vector<T>> comma_separated(TokenIterator& iterator,
                                    const std::function<bool(TokenIterator&, const Token&)>& stop_condition,
                                    const std::function<bool(TokenIterator&, const Token&)>& next_condition,
                                    const std::function<void(TokenIterator&, const Token&, too::Vector<T>&)>& inserter) {
  Vector<T> result;
  bool expect_comma = false;
  
  while (iterator.has_next()) {
    const auto& next = iterator.peek();
    
    if (expect_comma && next.type != TokenType::COMMA) {
      if (stop_condition(iterator, next)) {
        //  Ok: closing.
        break;
      } else {
        //  @FIXME: Error, expect comma between type parameters
        return NullOpt{};
      }
    } else if (expect_comma && next.type == TokenType::COMMA) {
      iterator.advance(1);
      expect_comma = false;
      
      continue;
    }
    
    if (!expect_comma && !next_condition(iterator, next)) {
      if (stop_condition(iterator, next)) {
        //  Ok: closing.
        break;
      } else {
        //  @FIXME: Error, expect xx.
        return NullOpt{};
      }
    } else if (!expect_comma && next_condition(iterator, next)) {
      inserter(iterator, next, result);
      
      expect_comma = true;
    }
  }
  
  return Optional<decltype(result)>(std::move(result));
}

Optional<Vector<ast::TypeParameter>> type_parameters(TokenIterator& iterator, SyntaxParseResult& result) {
  const auto stop_condition = [](auto& iter, const auto& next) -> bool {
    return next.type == TokenType::GREATER;
  };
  
  const auto next_condition = [](auto& iter, const auto& next) -> bool {
    return next.type == TokenType::IDENTIFIER;
  };
  
  const auto inserter = [](auto& iter, const auto& next, auto& result) -> void {
    result.push_back({next.lexeme});
    iter.advance(1);
  };
  
  auto comma_result = comma_separated<ast::TypeParameter>(iterator, stop_condition, next_condition, inserter);
  
  if (!comma_result || comma_result.value().size() == 0) {
    return NullOpt{};
  }
  
  //  Consume >
  iterator.advance(1);
  
  return comma_result;
}

Optional<Vector<ast::Identifier>> function_arguments(TokenIterator& iterator, SyntaxParseResult& result) {
  const auto stop_condition = [](auto& iter, const auto& next) -> bool {
    return next.type == TokenType::RIGHT_PARENS;
  };
  
  const auto next_condition = [](auto& iter, const auto& next) -> bool {
    return next.type == TokenType::IDENTIFIER &&
      iter.peek(1).type == TokenType::COLON &&
      iter.peek(2).type == TokenType::IDENTIFIER;
  };
  
  const auto inserter = [](auto& iter, const auto& next, auto& result) -> void {
    const auto name = next.lexeme;
    const auto type = iter.peek(2).lexeme;
    
    result.push_back({name, Optional<StringView>(type)});
    
    iter.advance(3);
  };
  
  auto comma_result = comma_separated<ast::Identifier>(iterator, stop_condition, next_condition, inserter);
  
  if (!comma_result) {
    return NullOpt{};
  }
  
  //  Consume )
  iterator.advance(1);
  
  return comma_result;
}

Optional<ast::TraitBoundedType> trait_bounded_type(TokenIterator& iterator, SyntaxParseResult& result) {
  TokenType start_sequence[2] = {TokenType::IDENTIFIER, TokenType::COLON};
  
  const auto start_error_handler = [](const auto&) -> void {
    //  @FIXME:
  };
  
  if (!expect_sequence(iterator, start_sequence, 2, start_error_handler)) {
    return NullOpt{};
  }
  
  ast::TraitBoundedType bounded_type;
  bounded_type.type = iterator.next().lexeme;

  iterator.advance(1);  //  Skip colon.
  
  const auto& next = iterator.peek();
  
  if (next.type == TokenType::IDENTIFIER) {
    bounded_type.traits.push_back({next.lexeme});
    
  } else if (next.type != TokenType::LEFT_BRACE) {
    return NullOpt{};
    
  } else {
    //  Sequence of types
    iterator.advance(1);
    
    const auto stop_condition = [](auto& iterator, const auto& next) -> bool {
      return next.type == TokenType::RIGHT_BRACE;
    };
    
    const auto next_condition = [](auto& iterator, const auto& next) -> bool {
      return next.type == TokenType::IDENTIFIER;
    };
    
    const auto inserter = [](auto& iterator, const auto& next, auto& result) -> void {
      result.push_back({next.lexeme});
      iterator.advance(1);
    };
    
    auto comma_result = comma_separated<ast::TypeParameter>(iterator, stop_condition, next_condition, inserter);
    
    if (!comma_result) {
      return NullOpt{};
    }
    
    bounded_type.traits = comma_result.rvalue();
  }
  
  return Optional<decltype(bounded_type)>(std::move(bounded_type));
}

Optional<ast::WhereClause> where_clause(TokenIterator& iterator, SyntaxParseResult& result) {
  ast::WhereClause clause;
  
  Optional<ast::TraitBoundedType> trait_bounds = trait_bounded_type(iterator, result);
  
  while (trait_bounds != NullOpt{}) {
    clause.types.push_back(trait_bounds.value());
    trait_bounds = trait_bounded_type(iterator, result);
  }
  
  if (clause.types.size() == 0) {
    return NullOpt{};
  }
  
  iterator.advance(1);
  
  return Optional<decltype(clause)>(std::move(clause));
}

void function(TokenIterator& iterator, SyntaxParseResult& result) {
  too::ast::FunctionDefinition function_definition;
  
  auto next = iterator.peek();
  
  if (next.type != TokenType::IDENTIFIER) {
    //  @FIXME: Error, missing identifier
    return;
  }
  
  //
  //  Name
  function_definition.name = next.lexeme;
  iterator.advance(1);
  
  //
  //  Generic ?
  next = iterator.peek();

  if (next.type == TokenType::LESS) {
    //  Consume <
    iterator.advance(1);
    
    auto maybe_type_parameters = type_parameters(iterator, result);
    
    if (!maybe_type_parameters) {
      //  Error should be marked in type_parameters
      return;
    }
    
    function_definition.type_parameters = maybe_type_parameters.rvalue();
  }
  
  next = iterator.peek();
  
  if (next.type != TokenType::LEFT_PARENS) {
    //  Error should be marked in type_parameters
    return;
  }
  
  //  Consume (
  iterator.advance(1);
  
  //
  //  Arguments
  auto maybe_arguments = function_arguments(iterator, result);
  
  if (!maybe_arguments) {
    //  Error should be marked in function_arguments
    return;
  }
  
  function_definition.input_arguments = std::move(maybe_arguments.rvalue());
  
  //
  //  Return type
  next = iterator.peek();
  
  if (next.type != TokenType::RIGHT_ARROW) {
    //  @FIXME: Expect right_arrow for return type.
    return;
  }
  
  iterator.advance(1);
  next = iterator.peek();
  
  if (next.type != TokenType::IDENTIFIER) {
    //  @FIXME: Expect identifier. (return type).
    return;
  }
  
  function_definition.return_type = next.lexeme;
  
  iterator.advance(1);
  
  //
  //  (Optional) Where clause
  
  next = iterator.peek();
  
  if (next.type == TokenType::WHERE) {
    iterator.advance(1);  //  consume where
    
    auto where_result = where_clause(iterator, result);
    
    if (!where_result) {
      return;
    }
    
    function_definition.where_clause = std::move(where_result);
  }
  
  next = iterator.peek();
  
  if (next.type != TokenType::LEFT_BRACE) {
    //  @FIXME: Expect left brace (opening to function body)
    return;
  }
  
  std::cout << function_definition.to_string() << std::endl;
}

SyntaxParseResult parse_syntax(const too::Vector<too::Token>& tokens) {
  SyntaxParseResult result;
  
  too::TokenIterator iterator(tokens.data(), tokens.size());
  
  while (iterator.has_next()) {
    const auto& token = iterator.next();
    
    if (token.type == TokenType::FN) {
      function(iterator, result);
    }
  }
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
