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
#include <cassert>
#include <set>

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

bool DefinitionRegistry::register_definition(const StringView& name, const ast::DefinitionContext& context) {
  ast::NamedDefinitionContext named_context(name, context);
  
  if (contents.count(named_context) > 0) {
    return false;
  }
  
  contents.emplace(named_context);
  return true;
}

struct EnclosingScope {
  Optional<int64_t> depth;
  
  void enter() {
    if (depth.has_value()) {
      depth = depth.value() + 1;
    } else {
      depth = 0;
    }
  }
  
  bool entered() const {
    return depth.has_value();
  }
  
  void exit() {
    assert(entered() && "Scope was not yet entered.");
    
    auto new_depth = depth.value() - 1;
    if (new_depth < 0) {
      depth = NullOpt{};
    } else {
      depth = new_depth;
    }
  }
};

struct UniqueEnclosingScope {
  UniqueEnclosingScope() : current_id(0) {
    //
  }
  
  Optional<int64_t> id() const {
    if (ids.empty()) {
      return NullOpt{};
    } else {
      return Optional<int64_t>(ids.back());
    }
  }
  
  void enter() {
    ids.push_back(current_id++);
  }
  
  bool entered() const {
    return !ids.empty();
  }
  
  void exit() {
    assert(entered() && "Scope was not yet entered.");
    ids.pop_back();
  }
private:
  Vector<int64_t> ids;
  int64_t current_id;
};

struct UniqueEnclosingScopeHelper {
  UniqueEnclosingScope& scope;
  
  UniqueEnclosingScopeHelper(UniqueEnclosingScope& scp) : scope(scp) {
    scope.enter();
  }
  
  ~UniqueEnclosingScopeHelper() {
    scope.exit();
  }
};

struct EnclosingScopeHelper {
  EnclosingScope& scope;
  
  EnclosingScopeHelper(EnclosingScope& scp) : scope(scp) {
    scope.enter();
  }
  
  ~EnclosingScopeHelper() {
    scope.exit();
  }
};

struct Context {
  UniqueEnclosingScope enclosing_scope;
  UniqueEnclosingScope enclosing_module;
  UniqueEnclosingScope enclosing_trait;
  EnclosingScope enclosing_function;
  int64_t scope_depth;
  
  DefinitionRegistry module_registry;
  DefinitionRegistry trait_registry;
  DefinitionRegistry function_registry;
  DefinitionRegistry struct_registry;
  DefinitionRegistry identifier_registry;
  
  Context() : scope_depth(-1) {
    //
  }
  
  void increment_scope() {
    scope_depth++;
  }
  
  void decrement_scope() {
    scope_depth--;
  }
};

struct ScopeHelper {
  Context& context;
  
  ScopeHelper(Context& ctx) : context(ctx) {
    context.increment_scope();
  }
  
  ~ScopeHelper() {
    context.decrement_scope();
  }
};

Optional<ast::BoxedExpr> expression(TokenIterator& iterator, SyntaxParseResult& result, Context& context);
Optional<ast::FunctionDefinition> anonymous_function_definition(TokenIterator& iterator,
                                                                SyntaxParseResult& result,
                                                                Context& context);

Optional<ast::BoxedStmt> block_statement(TokenIterator& iterator, SyntaxParseResult& result, Context& context);

ast::DefinitionContext definition_context(Context& context);
Optional<ast::TypeParameter> type_parameter(TokenIterator& iterator, SyntaxParseResult& result);
Optional<Vector<ast::TypeParameter>> type_parameters(TokenIterator& iterator, SyntaxParseResult& result);

template <typename T, typename... Args>
inline Optional<ast::BoxedExpr> make_optional_boxed_expr(Args&&... args) {
  return Optional<ast::BoxedExpr>(std::make_unique<T>(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
inline Optional<ast::BoxedStmt> make_optional_boxed_stmt(Args&&... args) {
  return Optional<ast::BoxedStmt>(std::make_unique<T>(std::forward<Args>(args)...));
}

inline bool is_unary_token_type(const TokenType token_type) {
  return token_type == TokenType::BANG || token_type == TokenType::MINUS;
}

inline bool matches_token_type_pair(const TokenIterator& iterator,
                                    const TokenType first,
                                    const TokenType second,
                                    const int64_t offset = 0) {
  return (iterator.peek(offset).type == first &&
          iterator.peek(offset + 1).type == second);
}

inline bool is_identifier_colon_sequence(const TokenIterator& iterator, int64_t offset = 0) {
  return matches_token_type_pair(iterator, TokenType::IDENTIFIER, TokenType::COLON, offset);
}

inline bool is_anonymous_function_definition_start(const TokenIterator& iterator) {
  if (iterator.peek().type != TokenType::LEFT_PARENS) {
    return false;
  } else if (is_identifier_colon_sequence(iterator, 1)) {
    return true;
  } else {
    return matches_token_type_pair(iterator, TokenType::RIGHT_PARENS, TokenType::RIGHT_ARROW, 1);
  }
}

inline bool is_unary_operable(const TokenType preceding_token) {
  switch (preceding_token) {
    case TokenType::IDENTIFIER:
    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::STRING_LITERAL:
    case TokenType::RIGHT_PARENS:
    case TokenType::RIGHT_BRACKET:
      return false;
    default:
      return true;
  }
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

inline bool is_useable_token_type(const TokenType token_type) {
  switch (token_type) {
    case TokenType::MOD:
    case TokenType::FN:
    case TokenType::TRAIT:
    case TokenType::STRUCT:
      return true;
    default:
      return false;
  }
}

inline int token_precedence(const TokenType token_type) {
  switch (token_type) {
    case TokenType::EQUAL:
      return 1;
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
      return 2;
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
      return 3;
    case TokenType::PLUS:
    case TokenType::MINUS:
      return 4;
    case TokenType::STAR:
    case TokenType::FORWARD_SLASH:
      return 5;
    default:
      return -1;
  }
}

inline bool is_literal_token_type(const TokenType token_type) {
  return (token_type == TokenType::INT_LITERAL ||
          token_type == TokenType::FLOAT_LITERAL ||
          token_type == TokenType::STRING_LITERAL);
}

void find_parent_functions(Vector<ast::FunctionDefinition>& functions) {
  //  @FIXME: This runs in quadratic time.
  int64_t start_search = -1;
  int64_t prev_depth = -1;
  const auto n_funcs = functions.size();
  
  for (auto i = 0; i < n_funcs; i++) {
    auto& func = functions[i];
    
    if (!func.context.enclosing_function) {
      continue;
    }
    
    int64_t j;
    auto depth = func.context.enclosing_function.value();
    const auto& enclosing_trait = func.context.enclosing_trait;
    
    if (start_search <= i || depth != prev_depth) {
      j = i + 1;
    } else {
      j = start_search;
    }
    
    prev_depth = depth;
    
    while (j < n_funcs) {
      const auto& maybe_parent_enclosing_func = functions[j].context.enclosing_function;
      const auto& maybe_parent_enclosing_trait = functions[j].context.enclosing_trait;
      
      const bool matches_depth = !maybe_parent_enclosing_func || maybe_parent_enclosing_func.value() == depth-1;
      const bool matches_trait = enclosing_trait == maybe_parent_enclosing_trait;
      
      if (matches_depth && matches_trait) {
        func.context.enclosing_function = j;
        start_search = j;
        break;
      }
      
      j++;
    }
  }
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

template <typename T>
inline Optional<T> try_parse_number(const StringView& view) {
  if (view.empty()) {
    return NullOpt{};
  }
  
  auto str = too::to_string(view);
  
  try {
    return Optional<T>(StringToNumber<T>::convert(str));
  } catch (...) {
    return NullOpt{};
  }
}

inline bool consume_token(TokenIterator& iterator, SyntaxParseResult& result, const TokenType expected_type) {
  if (iterator.peek().type != expected_type) {
    return false;
  }
  
  iterator.advance(1);
  return true;
}

inline Optional<Token> require_token(TokenIterator& iterator,
                                     SyntaxParseResult& result,
                                     const TokenType expected_type) {
  if (iterator.peek().type != expected_type) {
    return NullOpt{};
  }
  
  return Optional<Token>(iterator.next());
}

Optional<ast::TypeParameter> function_type_parameter(TokenIterator& iterator, SyntaxParseResult& result) {
  iterator.advance(1);  //  consume (
  
  Vector<ast::TypeParameter> params;
  
  //  Allow ()
  if (!consume_token(iterator, result, TokenType::RIGHT_PARENS)) {
    auto input_params_result = type_parameters(iterator, result);
    if (!input_params_result || !consume_token(iterator, result, TokenType::RIGHT_PARENS)) {
      return NullOpt{};
    }
    
    params = input_params_result.rvalue();
  }
  
  if (!consume_token(iterator, result, TokenType::RIGHT_ARROW)) {
    return NullOpt{};
  }
  
  auto return_type_result = type_parameter(iterator, result);
  if (!return_type_result) {
    return NullOpt{};
  }
  
  params.push_back(return_type_result.rvalue());
  
  ast::TypeParameter type_param;
  type_param.parameters = std::move(params);
  type_param.mark_function();
  
  return Optional<ast::TypeParameter>(std::move(type_param));
}

Optional<ast::TypeParameter> array_type_parameter(TokenIterator& iterator, SyntaxParseResult& result) {
  iterator.advance(1);  //  Consume [
  
  ast::TypeParameter type_param;
  Vector<ast::TypeParameter> children;
  
  auto inner_type_result = type_parameter(iterator, result);
  if (!inner_type_result || !consume_token(iterator, result, TokenType::RIGHT_BRACKET)) {
    return NullOpt{};
  }
  
  children.push_back(inner_type_result.rvalue());
  
  type_param.parameters = std::move(children);
  type_param.mark_array();
  
  return Optional<ast::TypeParameter>(std::move(type_param));
}

Optional<ast::TypeParameter> type_parameter(TokenIterator& iterator, SyntaxParseResult& result) {
  if (iterator.peek().type == TokenType::LEFT_PARENS) {
    return function_type_parameter(iterator, result);
  } else if (iterator.peek().type == TokenType::LEFT_BRACKET) {
    return array_type_parameter(iterator, result);
  }
  
  const auto next = require_token(iterator, result, TokenType::IDENTIFIER);
  if (!next) {
    return NullOpt{};
  }
  
  ast::TypeParameter type_param;
  type_param.name = next.value().lexeme;
  
  if (consume_token(iterator, result, TokenType::LESS)) {
    auto type_params_result = type_parameters(iterator, result);
    if (!type_params_result || !consume_token(iterator, result, TokenType::GREATER)) {
      return NullOpt{};
    }
    
    type_param.parameters = type_params_result.rvalue();
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
    
    if (!consume_token(iterator, result, TokenType::COMMA)) {
      break;
    }
  }
  
  return Optional<decltype(res)>(std::move(res));
}


Optional<ast::Identifier> optionally_typed_identifier(TokenIterator& iterator,
                                                      SyntaxParseResult& result,
                                                      const bool allow_untyped) {
  auto identifier_token = require_token(iterator, result, TokenType::IDENTIFIER);
  if (!identifier_token) {
    return NullOpt{};
  }
  
  ast::Identifier identifier;
  identifier.name = identifier_token.value().lexeme;
  
  if (consume_token(iterator, result, TokenType::COLON)) {
    auto type_param_result = type_parameter(iterator, result);
    if (!type_param_result) {
      return NullOpt{};
    }
    
    identifier.type = type_param_result.rvalue();
  } else if (!allow_untyped) {
    return NullOpt{};
  }
  
  return Optional<decltype(identifier)>(std::move(identifier));
}

inline Optional<ast::Identifier> maybe_typed_identifier(TokenIterator& iterator,
                                                        SyntaxParseResult& result) {
  return optionally_typed_identifier(iterator, result, true);
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
    
    if (!consume_token(iterator, result, TokenType::COMMA)) {
      break;
    }
  }
  
  return Optional<decltype(identifiers)>(std::move(identifiers));
}

Optional<Vector<ast::Identifier>> explicitly_typed_identifier_sequence(TokenIterator& iterator,
                                                                       SyntaxParseResult& result,
                                                                       const TokenType stop_token) {
  if (consume_token(iterator, result, stop_token)) {
    //  Empty sequence.
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

Optional<Vector<ast::Identifier>> untyped_identifier_sequence(TokenIterator& iterator,
                                                              SyntaxParseResult& result,
                                                              const TokenType stop_token) {
  Vector<ast::Identifier> identifiers;
  
  while (iterator.has_next()) {
    const auto& next = iterator.peek();
    
    if (next.type == TokenType::IDENTIFIER) {
      iterator.advance(1);
      ast::Identifier identifier;
      identifier.name = next.lexeme;
      identifiers.push_back(identifier);
    } else if (next.type != stop_token) {
      return NullOpt{};
    }
    
    if (!consume_token(iterator, result, TokenType::COMMA)) {
      break;
    }
  }
  
  return Optional<decltype(identifiers)>(std::move(identifiers));
}

Optional<ast::TraitBoundedType> trait_bounded_type(TokenIterator& iterator, SyntaxParseResult& result) {
  if (iterator.peek().type != TokenType::IDENTIFIER || iterator.peek(1).type != TokenType::COLON) {
    return NullOpt{};
  }
  
  ast::TraitBoundedType bounded_type;
  bounded_type.type = iterator.next().lexeme;
  iterator.advance(1);  //  Skip colon.
  const auto& next = iterator.peek();
  
  if (next.type == TokenType::IDENTIFIER) {
    auto type_param_result = type_parameter(iterator, result);
    if (!type_param_result) {
      return NullOpt{};
    }
    
    bounded_type.traits.push_back(type_param_result.rvalue());
    
  } else if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    return NullOpt{};

  } else {
    auto comma_result = type_parameters(iterator, result);
    if (!comma_result || !consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
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
    
    if (!consume_token(iterator, result, TokenType::COMMA)) {
      break;
    }
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
  if (consume_token(iterator, result, TokenType::WHERE)) {    
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
  if (consume_token(iterator, result, TokenType::LESS)) {
    auto maybe_type_parameters = type_parameters(iterator, result);
    if (!maybe_type_parameters || !consume_token(iterator, result, TokenType::GREATER)) {
      return false;
    }
    
    assign_to.header.type_parameters = maybe_type_parameters.rvalue();
  }
  
  return true;
}

template <typename T>
inline bool check_assign_name_and_type_parameters(TokenIterator& iterator,
                                                  SyntaxParseResult& result,
                                                  T& assign_to) {
  const auto& next = iterator.peek();
  if (next.type != TokenType::IDENTIFIER) {
    //  @FIXME: Error, missing name
    return false;
  }
  
  assign_to.header.name = next.lexeme;
  iterator.advance(1);
  
  return check_assign_type_parameters(iterator, result, assign_to);
}

Optional<ast::DefinitionHeader> definition_header(TokenIterator& iterator,
                                                  SyntaxParseResult& result,
                                                  const Token& identifier_token) {
  ast::DefinitionHeader header;
  header.name = identifier_token.lexeme;
  
  if (consume_token(iterator, result, TokenType::LESS)) {
    auto type_param_result = type_parameters(iterator, result);
    if (!type_param_result || !consume_token(iterator, result, TokenType::GREATER)) {
      return NullOpt{};
    }
    
    header.type_parameters = type_param_result.rvalue();
  }
  
  return Optional<ast::DefinitionHeader>(std::move(header));
}

inline bool check_assign_function_arguments_and_return_type(TokenIterator& iterator,
                                                            SyntaxParseResult& result,
                                                            Context& context,
                                                            ast::FunctionDefinition& function_def) {
  if (!consume_token(iterator, result, TokenType::LEFT_PARENS)) {
    //  @FIXME: Expected left parens
    return false;
  }
  
  //  Arguments
  auto maybe_arguments = function_arguments(iterator, result);
  if (!maybe_arguments) {
    return false;
  }
  
  function_def.input_parameters = maybe_arguments.rvalue();
  
  if (consume_token(iterator, result, TokenType::RIGHT_ARROW)) {
    auto return_type_res = type_parameter(iterator, result);
    if (!return_type_res) {
      return false;
    }
    
    function_def.return_type = return_type_res.rvalue();
  }
  
  return true;
}

inline bool check_assign_function_body(TokenIterator& iterator,
                                       SyntaxParseResult& result,
                                       Context& context,
                                       ast::FunctionDefinition& function_def) {
  //  Expect function body.
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    //  @FIXME: Expect left brace (opening to function body)
    return false;
  }
  
  auto body = block_statement(iterator, result, context);
  if (!body) {
    return false;
  }
  
  function_def.body = body.rvalue();
  return true;
}

inline bool register_module(Context& context, const StringView& name) {
  ast::DefinitionContext def_ctx = definition_context(context);
  return context.module_registry.register_definition(name, def_ctx);
}

inline bool register_trait(Context& context, const StringView& name) {
  ast::DefinitionContext def_ctx = definition_context(context);
  return context.trait_registry.register_definition(name, def_ctx);
}

inline bool register_function(Context& context, const StringView& name) {
  ast::DefinitionContext def_ctx = definition_context(context);
  return context.function_registry.register_definition(name, def_ctx);
}

inline bool register_struct(Context& context, const StringView& name) {
  ast::DefinitionContext def_ctx = definition_context(context);
  return context.struct_registry.register_definition(name, def_ctx);
}

inline bool register_identifier(Context& context, const StringView& name) {
  ast::DefinitionContext def_ctx = definition_context(context);
  return context.identifier_registry.register_definition(name, def_ctx);
}

ast::DefinitionContext definition_context(Context& context) {
  ast::DefinitionContext def_context;
  
  def_context.scope_depth = context.scope_depth;
  def_context.enclosing_function = context.enclosing_function.depth;
  def_context.enclosing_module = context.enclosing_module.id();
  def_context.enclosing_trait = context.enclosing_trait.id();
  def_context.enclosing_scope = context.enclosing_scope.id();
  
  return def_context;
}

Optional<ast::FunctionDefinition> function_definition(TokenIterator& iterator,
                                                      SyntaxParseResult& result,
                                                      Context& context,
                                                      ast::DefinitionHeader& header,
                                                      const bool require_implementation) {
  too::ast::FunctionDefinition function_def;
  function_def.header = std::move(header);
  
  if (!check_assign_function_arguments_and_return_type(iterator, result, context, function_def)) {
    return NullOpt{};
  }

  //  (Optional) Where clause.
  if (!check_assign_where(iterator, result, function_def)) {
    return NullOpt{};
  }
  
  //  Ensure non-duplicate.
  if (!register_function(context, header.name)) {
    return NullOpt{};
  }
  
  function_def.context = definition_context(context);
  EnclosingScopeHelper scope_helper(context.enclosing_function);
  
  if (consume_token(iterator, result, TokenType::SEMICOLON)) {
    if (require_implementation) {
      return NullOpt{};
    }
  } else if (!check_assign_function_body(iterator, result, context, function_def)) {
    return NullOpt{};
  }
  
  return Optional<ast::FunctionDefinition>(std::move(function_def));
}

Optional<ast::FunctionDefinition> anonymous_function_definition(TokenIterator& iterator,
                                                                SyntaxParseResult& result,
                                                                Context& context) {
  too::ast::FunctionDefinition function_def;
  
  if (!check_assign_function_arguments_and_return_type(iterator, result, context, function_def)) {
    return NullOpt{};
  }
  
  function_def.context = definition_context(context);
  EnclosingScopeHelper scope_helper(context.enclosing_function);
  
  if (!check_assign_function_body(iterator, result, context, function_def)) {
    return NullOpt{};
  }
  
  return Optional<ast::FunctionDefinition>(std::move(function_def));
}

Optional<ast::StructDefinition> struct_definition(TokenIterator& iterator,
                                                  SyntaxParseResult& result,
                                                  Context& context,
                                                  ast::DefinitionHeader& header) {
  
  iterator.advance(1);  //  consume struct.
  
  if (context.enclosing_function.entered() || context.enclosing_trait.entered()) {
    //  Disallow struct definitions, except in module scope.
    return NullOpt{};
  }
  
  if (!register_struct(context, header.name)) {
    return NullOpt{};
  }
  
  too::ast::StructDefinition struct_def;
  struct_def.header = std::move(header);
  struct_def.context = definition_context(context);
  
  //  Has where clause ?
  if (!check_assign_where(iterator, result, struct_def)) {
    return NullOpt{};
  }
  
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    //  @FIXME: Expected left brace
    return NullOpt{};
  }
  
  //  Arguments
  auto maybe_members = struct_members(iterator, result);
  if (!maybe_members) {
    //  Error should be marked in struct_members
    return NullOpt{};
  }
  
  struct_def.members = maybe_members.rvalue();
  
  return Optional<ast::StructDefinition>(std::move(struct_def));
}

Optional<ast::TraitDefinition> trait_definition(TokenIterator& iterator,
                                                SyntaxParseResult& result,
                                                Context& context,
                                                ast::DefinitionHeader& header) {
  iterator.advance(1);  //  consume trait.
  
  if (context.enclosing_function.entered()) {
    //  Disallow trait definitions, except in module scope.
    return NullOpt{};
  }
  
  too::ast::TraitDefinition trait_def;
  trait_def.header = std::move(header);
  trait_def.context = definition_context(context);
  
  if (!register_trait(context, trait_def.header.name)) {
    //  Duplicate trait.
    return NullOpt{};
  }
  
  //  Has where clause ?
  if (!check_assign_where(iterator, result, trait_def)) {
    return NullOpt{};
  }
  
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    //  @FIXME: Expected left brace
    return NullOpt{};
  }
  
  UniqueEnclosingScopeHelper scope_helper(context.enclosing_trait);
  
  while (iterator.has_next() && iterator.peek().type != TokenType::RIGHT_BRACE) {
    const auto next = require_token(iterator, result, TokenType::IDENTIFIER);
    if (!next) {
      return NullOpt{};
    }
    
    auto header_result = definition_header(iterator, result, next.value());
    if (!header_result || !consume_token(iterator, result, TokenType::DOUBLE_COLON)) {
      return NullOpt{};
    }
    
    auto&& header = header_result.rvalue();
    const bool require_impl = false;
    auto func_result = function_definition(iterator, result, context, header, require_impl);
    if (!func_result) {
      return NullOpt{};
    }
    
    result.functions.push_back(func_result.rvalue());
    trait_def.functions.push_back(result.functions.size()-1);
  }
  
  if (!consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
    return NullOpt{};
  }
  
  return Optional<ast::TraitDefinition>(std::move(trait_def));
}

Optional<ast::UsingDeclaration> using_declaration(TokenIterator& iterator,
                                                  SyntaxParseResult& result,
                                                  Context& context) {
  
  const auto& using_type = iterator.next();
  if (!is_useable_token_type(using_type.type)) {
    return NullOpt{};
  }
  
  Vector<ast::Identifier> used_targets;
  bool require_in_keyword = false;
  
  if (consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    auto identifier_sequence_res = untyped_identifier_sequence(iterator, result, TokenType::RIGHT_BRACE);
    if (!identifier_sequence_res || !consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
      return NullOpt{};
    }
    
    used_targets = identifier_sequence_res.rvalue();
    require_in_keyword = true;
  } else {
    auto token_res = require_token(iterator, result, TokenType::IDENTIFIER);
    if (!token_res) {
      return NullOpt{};
    }
    
    require_in_keyword = using_type.type != TokenType::MOD;
    ast::Identifier id;
    id.name = token_res.value().lexeme;
    used_targets.push_back(std::move(id));
  }
  
  ast::UsingDeclaration using_decl;
  
  if (require_in_keyword || iterator.peek().type == TokenType::IN) {
    if (!consume_token(iterator, result, TokenType::IN)) {
      return NullOpt{};
    }
    
    auto in_module_res = require_token(iterator, result, TokenType::IDENTIFIER);
    if (!in_module_res) {
      return NullOpt{};
    }
    
    using_decl.targets = std::move(used_targets);
    using_decl.type = using_type.type;
    using_decl.in_module = in_module_res.value().lexeme;
  } else {
    assert(used_targets.size() == 1 && "Expected 1 used target.");
    
    using_decl.in_module = used_targets[0].name;
    using_decl.type = TokenType::MOD;
  }
  
  using_decl.context = definition_context(context);
  
  if (!consume_token(iterator, result, TokenType::SEMICOLON)) {
    return NullOpt{};
  }
  
  return Optional<ast::UsingDeclaration>(std::move(using_decl));
}

Optional<Vector<ast::BoxedExpr>> function_call(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  Vector<ast::BoxedExpr> input_arguments;
  
  while (iterator.peek().type != TokenType::RIGHT_PARENS) {
    auto arg_result = expression(iterator, result, context);
    
    if (!arg_result) {
      return NullOpt{};
    }
    
    input_arguments.push_back(arg_result.rvalue());
    
    if (!consume_token(iterator, result, TokenType::COMMA)) {
      break;
    }
  }
  
  if (!consume_token(iterator, result, TokenType::RIGHT_PARENS)) {
    return NullOpt{};
  }
  
  return Optional<Vector<ast::BoxedExpr>>(std::move(input_arguments));
}

Optional<ast::BoxedExpr> named_function_call(TokenIterator& iterator,
                                             SyntaxParseResult& result,
                                             Context& context,
                                             const Token& identifier_token) {
  auto input_args_result = function_call(iterator, result, context);
  if (!input_args_result) {
    return NullOpt{};
  }
  
  return make_optional_boxed_expr<ast::FunctionCallExpr>(identifier_token.lexeme, input_args_result.rvalue());
}

inline bool identifier_expression(TokenIterator& iterator,
                                  SyntaxParseResult& result,
                                  Context& context,
                                  Vector<ast::BoxedExpr>& completed,
                                  const Token& current) {
  iterator.advance(1);  //  consume identifier.
  
  if (consume_token(iterator, result, TokenType::LEFT_PARENS)) {
    auto func_call = named_function_call(iterator, result, context, current);
    if (!func_call) {
      return false;
    }
    
    completed.push_back(func_call.rvalue());
    
  } else if (current.type == TokenType::IDENTIFIER) {
    completed.push_back(std::make_unique<ast::IdentifierLiteralExpr>(current.lexeme));
  } else {
    return false;
  }
  
  return true;
}

inline bool bracket_reference_expression(TokenIterator& iterator,
                                         SyntaxParseResult& result,
                                         Context& context,
                                         Vector<ast::BoxedExpr>& completed) {
  iterator.advance(1);  //  consume [
  
  if (completed.empty()) {
    return false;
  }
  
  auto reference_expr = expression(iterator, result, context);
  if (!reference_expr || !consume_token(iterator, result, TokenType::RIGHT_BRACKET)) {
    return false;
  }
  
  auto bracket_expr = std::make_unique<ast::ContentsReferenceExpr>();
  bracket_expr->reference_expression = reference_expr.rvalue();
  bracket_expr->target_expression = std::move(completed.back());
  bracket_expr->operator_token = TokenType::LEFT_BRACKET;
  
  completed.back() = std::move(bracket_expr);
  
  return true;
}

inline bool member_reference_expression(TokenIterator& iterator,
                                        SyntaxParseResult& result,
                                        Context& context,
                                        Vector<ast::BoxedExpr>& completed) {
  iterator.advance(1);  //  consume .
  
  if (completed.empty()) {
    return false;
  }
  
  auto reference_expression = std::move(completed.back());
  completed.pop_back();
  
  if (!identifier_expression(iterator, result, context, completed, iterator.peek())) {
    return false;
  }
  
  auto member_expr = std::make_unique<ast::ContentsReferenceExpr>();
  member_expr->reference_expression = std::move(completed.back());
  member_expr->target_expression = std::move(reference_expression);
  member_expr->operator_token = TokenType::PERIOD;
  
  completed.back() = std::move(member_expr);
  
  return true;
}

inline bool anonymous_function_call_expression(TokenIterator& iterator,
                                               SyntaxParseResult& result,
                                               Context& context,
                                               Vector<ast::BoxedExpr>& completed) {
  iterator.advance(1);  //  consume (
  
  if (completed.empty()) {
    return false;
  }
  
  auto func_call_res = function_call(iterator, result, context);
  if (!func_call_res) {
    return false;
  }
  
  auto anon_expr = std::make_unique<ast::AnonymousFunctionCallExpr>();
  anon_expr->function = std::move(completed.back());
  anon_expr->input_arguments = func_call_res.rvalue();
  completed.back() = std::move(anon_expr);
  
  return true;
}

inline bool assignment_expression(TokenIterator& iterator,
                                  SyntaxParseResult& result,
                                  Context& context,
                                  Vector<ast::BoxedExpr>& completed) {
  iterator.advance(1);  //  consume =.
  
  if (completed.empty() || !completed.back()->is_valid_assignment_target()) {
    return false;
  }
  
  auto assign_expr = expression(iterator, result, context);
  if (!assign_expr) {
    return false;
  }
  
  auto assignment_expr = std::make_unique<ast::AssignmentExpr>();
  assignment_expr->assignment_expression = assign_expr.rvalue();
  assignment_expr->target_expression = std::move(completed.back());
  completed.back() = std::move(assignment_expr);
  
  return true;
}

template <typename Number, typename Expr>
inline bool make_numeric_literal_expression(Vector<ast::BoxedExpr>& completed, const Token& current) {
  auto parse_result = try_parse_number<Number>(current.lexeme);
  if (!parse_result) {
    return false;
  }
  
  completed.push_back(std::make_unique<Expr>(parse_result.value()));
  return true;
}

inline bool literal_expression(TokenIterator& iterator,
                               SyntaxParseResult& result,
                               Vector<ast::BoxedExpr>& completed,
                               const Token& current) {
  iterator.advance(1);  //  consume literal.
  
  if (current.type == TokenType::INT_LITERAL) {
    return make_numeric_literal_expression<TooInteger, ast::IntLiteralExpr>(completed, current);
    
  } else if (current.type == TokenType::FLOAT_LITERAL) {
    return make_numeric_literal_expression<TooFloat, ast::FloatLiteralExpr>(completed, current);
    
  } else if (current.type == TokenType::STRING_LITERAL) {
    completed.push_back(std::make_unique<ast::StringLiteralExpr>(current.lexeme));
    return true;
  }
  
  return false;
}

inline bool anonymous_function_definition_expression(TokenIterator& iterator,
                                                     SyntaxParseResult& result,
                                                     Context& context,
                                                     Vector<ast::BoxedExpr>& completed) {
  auto anon_result = anonymous_function_definition(iterator, result, context);
  if (!anon_result) {
    return false;
  }
  
  auto reference_expr = std::make_unique<ast::AnonymousFunctionLiteralExpr>();
  completed.push_back(std::move(reference_expr));
  
  return true;
}

inline bool grouping_expression(TokenIterator& iterator,
                                SyntaxParseResult& result,
                                Context& context,
                                Vector<ast::BoxedExpr>& completed) {
  iterator.advance(1);  //  Consume (
  
  auto expr = expression(iterator, result, context);
  if (!expr || !consume_token(iterator, result, TokenType::RIGHT_PARENS)) {
    return false;
  }
  
  completed.push_back(expr.rvalue());
  return true;
}

inline bool reference_or_anonymous_function_call_expression(TokenIterator& iterator,
                                                            SyntaxParseResult& result,
                                                            Context& context,
                                                            Vector<ast::BoxedExpr>& completed) {
  while (iterator.has_next()) {
    //  Member contents reference.
    if (iterator.peek().type == TokenType::PERIOD) {
      if (!member_reference_expression(iterator, result, context, completed)) {
        return false;
      }
      continue;
    }
    
    //  Bracket reference.
    if (iterator.peek().type == TokenType::LEFT_BRACKET) {
      if (!bracket_reference_expression(iterator, result, context, completed)) {
        return false;
      }
      continue;
    }
    
    //  Anonymous function call.
    if (iterator.peek().type == TokenType::LEFT_PARENS) {
      if (!anonymous_function_call_expression(iterator, result, context, completed)) {
        return false;
      }
      continue;
    }
    
    break;
  }
  
  return true;
}

inline void update_binary_expressions(const TokenIterator& iterator,
                                      Vector<std::unique_ptr<ast::BinaryExpr>>& binaries,
                                      Vector<std::unique_ptr<ast::BinaryExpr>>& pending,
                                      Vector<int>& pending_precedence,
                                      Vector<ast::BoxedExpr>& completed) {
  if (!binaries.empty()) {
    auto& bin = binaries.back();
    assert(bin.get() && "Binary operator was null");
    
    int prec_curr = token_precedence(bin->operator_token);
    int prec_next = token_precedence(iterator.peek().type);
    
    if (prec_curr < prec_next) {
      pending.push_back(std::move(bin));
      pending_precedence.push_back(prec_next);
      
    } else {
      auto complete = std::move(completed.back());
      bin->right = std::move(complete);
      completed.back() = std::move(bin);
      
      while (!pending.empty() && prec_next < prec_curr) {
        auto pend = std::move(pending.back());
        prec_curr = pending_precedence.back();
        
        pend->right = std::move(completed.back());
        completed.back() = std::move(pend);
        
        pending.pop_back();
        pending_precedence.pop_back();
      }
    }
  }
}

inline void update_unary_expressions(Vector<std::unique_ptr<ast::UnaryExpr>>& unaries,
                                     Vector<ast::BoxedExpr>& completed) {
  while (!unaries.empty()) {
    unaries.back()->expression = std::move(completed.back());
    completed.back() = std::move(unaries.back());
    
    unaries.pop_back();
  }
}

Optional<ast::BoxedExpr> expression(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  Vector<ast::BoxedExpr> completed;
  Vector<std::unique_ptr<ast::UnaryExpr>> unaries;
  Vector<std::unique_ptr<ast::BinaryExpr>> binaries;
  Vector<std::unique_ptr<ast::BinaryExpr>> pending;
  Vector<int> pending_precedence;
  
  while (iterator.has_next()) {
    const auto& next = iterator.peek();
    
    if (next.type == TokenType::LEFT_PARENS) {
      if (is_anonymous_function_definition_start(iterator)) {
        if (!anonymous_function_definition_expression(iterator, result, context, completed)) {
          return NullOpt{};
        }
      } else if (!grouping_expression(iterator, result, context, completed)) {
        return NullOpt{};
      }
      
    } else if (next.type == TokenType::IDENTIFIER) {
      if (!identifier_expression(iterator, result, context, completed, next)) {
        return NullOpt{};
      }
      
    } else if (next.type == TokenType::EQUAL) {
      if (!assignment_expression(iterator, result, context, completed)) {
        return NullOpt{};
      }
      
    } else if (is_literal_token_type(next.type)) {
      if (!literal_expression(iterator, result, completed, next)) {
        return NullOpt{};
      }
      
    } else if (is_unary_token_type(next.type) && is_unary_operable(iterator.peek(-1).type)) {
      iterator.advance(1);
      unaries.push_back(std::make_unique<ast::UnaryExpr>(next.type, nullptr));
      
    } else if (is_binary_token_type(next.type)) {
      iterator.advance(1);
      
      if (completed.empty()) {
        //  Expect lhs.
        return NullOpt{};
      }
      
      auto left = std::move(completed.back());
      binaries.push_back(std::make_unique<ast::BinaryExpr>(next.type, std::move(left), nullptr));
      completed.pop_back();
      
    } else if (next.type == TokenType::RIGHT_PARENS ||
               next.type == TokenType::RIGHT_BRACKET ||
               next.type == TokenType::COMMA ||
               next.type == TokenType::SEMICOLON) {
      break;
      
    } else {
      return NullOpt{};
    }
    
    if (completed.empty()) {
      continue;
    }
    
    if (!reference_or_anonymous_function_call_expression(iterator, result, context, completed)) {
      return NullOpt{};
    }
    
    update_unary_expressions(unaries, completed);
    update_binary_expressions(iterator, binaries, pending, pending_precedence, completed);
  }
  
  if (completed.size() == 1) {
    return Optional<ast::BoxedExpr>(std::move(completed.back()));
  } else {
    return NullOpt{};
  }
}

Optional<ast::BoxedStmt> let_statement(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  iterator.advance(1);  //  consume let.
  
  ast::LetStmt let_stmt;
  //  true -> allow untyped.
  auto identifier = maybe_typed_identifier(iterator, result);
  if (!identifier) {
    return NullOpt{};
  }
  
  if (!register_identifier(context, identifier.value().name)) {
    //  Duplicate identifier.
    return NullOpt{};
  }
  
  if (consume_token(iterator, result, TokenType::EQUAL)) {
    auto initializer = expression(iterator, result, context);
    if (!initializer) {
      return NullOpt{};
    }
    
    let_stmt.initializer = std::move(initializer);
  }
  
  if (!consume_token(iterator, result, TokenType::SEMICOLON)) {
    return NullOpt{};
  }
  
  let_stmt.identifier = identifier.rvalue();
  return make_optional_boxed_stmt<ast::LetStmt>(std::move(let_stmt));
}

Optional<ast::BoxedStmt> return_statement(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  iterator.advance(1);  //  consume return.
  
  if (consume_token(iterator, result, TokenType::SEMICOLON)) {
    return make_optional_boxed_stmt<ast::ReturnStmt>();
  }
  
  auto expr = expression(iterator, result, context);
  if (!expr || !consume_token(iterator, result, TokenType::SEMICOLON)) {
    return NullOpt{};
  }
  
  return make_optional_boxed_stmt<ast::ReturnStmt>(expr.rvalue());
}

Optional<ast::BoxedStmt> expression_statement(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  auto expression_res = expression(iterator, result, context);
  
  if (!expression_res || !consume_token(iterator, result, TokenType::SEMICOLON)) {
    //  Expect semicolon
    return NullOpt{};
  }
    
  return make_optional_boxed_stmt<ast::ExprStmt>(expression_res.rvalue());
}

bool check_assign_let_statement(TokenIterator& iterator,
                                SyntaxParseResult& result,
                                Context& context,
                                ast::BlockStmt& assign_to) {
  auto statement_res = let_statement(iterator, result, context);
  if (statement_res) {
    assign_to.statements.push_back(statement_res.rvalue());
    return true;
  }
  
  return false;
}

bool check_assign_block_statement(TokenIterator& iterator,
                                  SyntaxParseResult& result,
                                  Context& context,
                                  ast::BlockStmt& assign_to) {
  iterator.advance(1);
  
  auto block_res = block_statement(iterator, result, context);
  if (block_res) {
    assign_to.statements.push_back(block_res.rvalue());
    return true;
  }
  
  return false;
}

bool check_assign_return_statement(TokenIterator& iterator,
                                   SyntaxParseResult& result,
                                   Context& context,
                                   ast::BlockStmt& assign_to) {
  auto return_res = return_statement(iterator, result, context);
  if (return_res) {
    assign_to.statements.push_back(return_res.rvalue());
    return true;
  }
  
  return false;
}

bool check_assign_expression_statement(TokenIterator& iterator,
                                       SyntaxParseResult& result,
                                       Context& context,
                                       ast::BlockStmt& assign_to) {
  auto statement_res = expression_statement(iterator, result, context);
  if (statement_res) {
    assign_to.statements.push_back(statement_res.rvalue());
    return true;
  }
  
  return false;
}

inline bool check_assign_function_definition(TokenIterator& iterator,
                                             SyntaxParseResult& result,
                                             Context& context,
                                             ast::DefinitionHeader& header) {
  const bool require_impl = true;
  auto function_def_result = function_definition(iterator, result, context, header, require_impl);
  if (!function_def_result) {
    return false;
  }
  
  result.functions.push_back(function_def_result.rvalue());
  
  return true;
}

inline bool check_assign_trait_definition(TokenIterator& iterator,
                                          SyntaxParseResult& result,
                                          Context& context,
                                          ast::DefinitionHeader& header) {
  auto trait_def_result = trait_definition(iterator, result, context, header);
  if (!trait_def_result) {
    return false;
  }
  
  result.traits.push_back(trait_def_result.rvalue());
  
  return true;
}

inline bool check_assign_struct_definition(TokenIterator& iterator,
                                           SyntaxParseResult& result,
                                           Context& context,
                                           ast::DefinitionHeader& header) {
  auto struct_def_result = struct_definition(iterator, result, context, header);
  if (!struct_def_result) {
    return false;
  }
  
  result.structs.push_back(struct_def_result.rvalue());
  return true;
}

inline bool check_assign_module_definition(TokenIterator& iterator,
                                           SyntaxParseResult& result,
                                           Context& context,
                                           ast::DefinitionHeader& header,
                                           ast::BlockStmt& assign_to) {
  iterator.advance(1);  //  consume mod.
  
  if (!consume_token(iterator, result, TokenType::LEFT_BRACE)) {
    return false;
  }
  
  if (context.enclosing_function.entered() || context.enclosing_trait.entered()) {
    //  Cannot define a module within a function or trait.
    return false;
  }
  
  if (!register_module(context, header.name)) {
    return false;
  }
  
  UniqueEnclosingScopeHelper scope_helper(context.enclosing_module);
  
  auto block_result = block_statement(iterator, result, context);
  if (!block_result) {
    return false;
  }
  
  assign_to.statements.push_back(block_result.rvalue());
  return true;
}

inline bool dispatch_from_identifier(TokenIterator& iterator,
                                     SyntaxParseResult& result,
                                     Context& context,
                                     ast::BlockStmt& assign_to) {
  const auto& identifier_token = iterator.next();
  
  auto header_result = definition_header(iterator, result, identifier_token);
  if (!header_result || !consume_token(iterator, result, TokenType::DOUBLE_COLON)) {
    return false;
  }
  
  const auto& next = iterator.peek();
  auto&& header = header_result.rvalue();
  
  if (next.type == TokenType::LEFT_PARENS) {
    return check_assign_function_definition(iterator, result, context, header);
    
  } else if (next.type == TokenType::TRAIT) {
    return check_assign_trait_definition(iterator, result, context, header);
    
  } else if (next.type == TokenType::STRUCT) {
    return check_assign_struct_definition(iterator, result, context, header);
    
  } else if (next.type == TokenType::MOD) {
    return check_assign_module_definition(iterator, result, context, header, assign_to);
    
  } else {
    return false;
  }
}

inline bool check_assign_variable_declaration(TokenIterator& iterator,
                                              SyntaxParseResult& result,
                                              Context& context,
                                              const Token& identifier_token,
                                              const Token& next_token,
                                              ast::BlockStmt& assign_to) {
  
  iterator.advance(2);  //  Consume identifier, :=
  
  auto initializer_result = expression(iterator, result, context);
  if (!initializer_result || !consume_token(iterator, result, TokenType::SEMICOLON)) {
    return false;
  }
  
  if (!register_identifier(context, identifier_token.lexeme)) {
    //  Duplicate identifier.
    return false;
  }
  
  ast::LetStmt let_stmt;
  let_stmt.identifier.name = identifier_token.lexeme;
  let_stmt.initializer = initializer_result.rvalue();
  
  auto fulfilled_initializer = std::make_unique<ast::LetStmt>(std::move(let_stmt));
  assign_to.statements.push_back(std::move(fulfilled_initializer));
  
  return true;
}

inline bool check_assign_identifier_statement_or_expression(TokenIterator& iterator,
                                                            SyntaxParseResult& result,
                                                            Context& context,
                                                            ast::BlockStmt& block_stmt,
                                                            const Token& token) {
  const auto& next = iterator.peek(1);
  
  if (next.type == TokenType::LESS || next.type == TokenType::DOUBLE_COLON) {
    return dispatch_from_identifier(iterator, result, context, block_stmt);
    
  } else if (next.type == TokenType::COLON_EQUAL) {
    return check_assign_variable_declaration(iterator, result, context, token, next, block_stmt);
    
  } else if (!check_assign_expression_statement(iterator, result, context, block_stmt)) {
    iterator.advance(1);
    return false;
  }
  
  return true;
}

inline bool check_assign_use_statement(TokenIterator& iterator,
                                       SyntaxParseResult& result,
                                       Context& context) {
  iterator.advance(1);  //  consume use;
  
  auto using_res = using_declaration(iterator, result, context);
  if (!using_res) {
    return false;
  }
  
  const auto& using_decl = using_res.value();
  if (using_decl.targets) {
    const auto& kind = using_decl.type;
    const auto& targets = using_decl.targets.value();
    
    for (auto i = 0; i < targets.size(); i++) {
      const auto& name = targets[i].name;
      
      if (kind == TokenType::MOD && !register_module(context, name)) {
        return false;
      } else if (kind == TokenType::TRAIT && !register_trait(context, name)) {
        return false;
      } else if (kind == TokenType::STRUCT && !register_struct(context, name)) {
        return false;
      } else if (kind == TokenType::FN && !register_function(context, name)) {
        return false;
      }
    }
  } else if (!register_module(context, using_decl.in_module)) {
    return false;
  }
  
  result.external_symbols.push_back(using_res.rvalue());
  
  return true;
}

Optional<ast::BoxedStmt> block_statement(TokenIterator& iterator, SyntaxParseResult& result, Context& context) {
  ScopeHelper scope_helper(context);
  UniqueEnclosingScopeHelper enclosing_scope_helper(context.enclosing_scope);
  ast::BlockStmt block_stmt;
  
  while (iterator.has_next()) {
    const auto& token = iterator.peek();
    
    if (token.type == TokenType::IDENTIFIER) {
      if (!check_assign_identifier_statement_or_expression(iterator, result, context, block_stmt, token)) {
        return NullOpt{};
      }
      
    } else if (token.type == TokenType::LET) {
      if (!check_assign_let_statement(iterator, result, context, block_stmt)) {
        return NullOpt{};
      }
      
    } else if (token.type == TokenType::USE) {
      if (!check_assign_use_statement(iterator, result, context)) {
        return NullOpt{};
      }
      
    } else if (token.type == TokenType::LEFT_BRACE) {
      if (!check_assign_block_statement(iterator, result, context, block_stmt)) {
        return NullOpt{};
      }
      
    } else if (token.type == TokenType::RETURN) {
      if (!check_assign_return_statement(iterator, result, context, block_stmt)) {
        return NullOpt{};
      }
      
    } else if (token.type == TokenType::RIGHT_BRACE || token.type == TokenType::END) {
      break;
      
    } else if (token.type == TokenType::SEMICOLON) {
      iterator.advance(1);
      
    } else {
      if (!check_assign_expression_statement(iterator, result, context, block_stmt)) {
        iterator.advance(1);
        return NullOpt{};
      }
    }
  }
  
  if (iterator.peek().type != TokenType::END) {
    if (!consume_token(iterator, result, TokenType::RIGHT_BRACE)) {
      return NullOpt{};
    }
  }
  
  return make_optional_boxed_stmt<ast::BlockStmt>(std::move(block_stmt));
}

SyntaxParseResult parse_syntax(const too::Vector<too::Token>& tokens) {
  SyntaxParseResult result;
  result.had_error = false;
  
  too::TokenIterator iterator(tokens.data(), tokens.size());
  Context context;
  ast::BlockStmt root;
  
  while (!consume_token(iterator, result, TokenType::END)) {
    auto block_res = block_statement(iterator, result, context);
    if (block_res) {
      root.statements.push_back(block_res.rvalue());
    } else {
      result.had_error = true;
    }
  }
  
  find_parent_functions(result.functions);
  
  result.function_registry = std::move(context.function_registry);
  result.struct_registry = std::move(context.struct_registry);
  result.trait_registry = std::move(context.trait_registry);
  result.identifier_registry = std::move(context.identifier_registry);
  result.module_registry = std::move(context.module_registry);
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
