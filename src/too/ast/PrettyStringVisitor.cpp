//
//  PrettyStringVisitor.cpp
//  too
//
//  Created by Nick Fagan on 5/1/19.
//

#include "PrettyStringVisitor.hpp"
#include "../Token.hpp"

#define BEGIN_NAMESPACE namespace too { namespace ast {
#define END_NAMESPACE }}

BEGIN_NAMESPACE

template <typename T>
String delimited_boxed_list_to_string(const PrettyStringVisitor& visitor,
                                      const Vector<T>& values,
                                      const char* const delimiter) {
  String result;
  
  for (auto i = 0; i < values.size(); i++) {
    result += values[i]->accept(visitor);
    
    if (i < values.size()-1) {
      result += delimiter;
    }
  }
  
  return result;
}

template <typename T>
String delimited_list_to_string(const PrettyStringVisitor& visitor,
                                const Vector<T>& values,
                                const char* const delimiter) {
  String result;
  
  for (auto i = 0; i < values.size(); i++) {
    result += values[i].accept(visitor);
    
    if (i < values.size()-1) {
      result += delimiter;
    }
  }
  
  return result;
}

template <typename T>
String comma_separated_to_string(const PrettyStringVisitor& visitor, const Vector<T>& values) {
  return delimited_list_to_string(visitor, values, ", ");
}

template <typename T>
inline String wrap_comma_separated_to_string(const PrettyStringVisitor& visitor,
                                             const Vector<T>& values,
                                             const String& lhs,
                                             const String& rhs) {
  return lhs + comma_separated_to_string(visitor, values) + rhs;
}

template <typename T>
inline String wrap_delimited_list_to_string(const PrettyStringVisitor& visitor,
                                            const Vector<T>& values,
                                            const String& lhs,
                                            const String& rhs,
                                            const char* const delimiter) {
  return lhs + delimited_list_to_string(visitor, values, delimiter) + rhs;
}

template <typename T>
inline String optional_number_to_string(const Optional<T>& v) {
  if (!v) {
    return "<null>";
  } else {
    return std::to_string(v.value());
  }
}

String function_type_parameter_to_string(const PrettyStringVisitor& visitor, const Vector<ast::TypeParameter>& params) {
  if (params.empty()) {
    return "";
  }
  
  String result = "(";
  
  for (int64_t i = 0; i < params.size()-1; i++) {
    result += params[i].accept(visitor);
    
    if (i < params.size()-2) {
      result += ", ";
    }
  }
  
  result += ") -> " + params[params.size()-1].accept(visitor);
  
  return result;
}


String PrettyStringVisitor::let_statement(const LetStmt& let_stmt) const {
  String result = "let " + let_stmt.identifier.accept(*this);
  
  if (let_stmt.initializer && let_stmt.initializer.value() != nullptr) {
    result += " = " + let_stmt.initializer.value()->accept(*this);
  }
  
  result += ';';
  
  return result;
}

String PrettyStringVisitor::expression_statement(const too::ast::ExprStmt& expr_stmt) const {
  return expr_stmt.expression->accept(*this) + ";";
}

String PrettyStringVisitor::block_statement(const too::ast::BlockStmt& block_stmt) const {
  String result = "{";
  
  if (!block_stmt.statements.empty()) {
    result += "\n";
  }
  
  for (auto i = 0; i < block_stmt.statements.size(); i++) {
    result += block_stmt.statements[i]->accept(*this);
    result += '\n';
  }
  
  result += "}";
  
  return result;
}

String PrettyStringVisitor::return_statement(const too::ast::ReturnStmt& return_stmt) const {
  String result = "return";
  
  if (return_stmt.expression) {
    result += " ";
    
    if (return_stmt.expression.value() == nullptr) {
      result += "<null>";
    } else {
      result += return_stmt.expression.value()->accept(*this);
    }
    
    result += ";";
  }
  
  return result;
}

String PrettyStringVisitor::unary_expression(const too::ast::UnaryExpr& unary_expr) const {
  String expr = unary_expr.expression == nullptr ? "<null>" : unary_expr.expression->accept(*this);
  return String(too::to_string_symbol(unary_expr.operator_token)) + "(" + expr + ")";
}

String PrettyStringVisitor::binary_expression(const too::ast::BinaryExpr& binary_expr) const {
  auto left_str = binary_expr.left == nullptr ? "<null>" : binary_expr.left->accept(*this);
  auto right_str = binary_expr.right == nullptr ? "<null>" : binary_expr.right->accept(*this);
  auto op = too::to_string_symbol(binary_expr.operator_token);
  
  auto left_wrapped = "(" + left_str + ")";
  auto right_wrapped = "(" + right_str + ")";
  
  return left_wrapped + op + right_wrapped;
}

String PrettyStringVisitor::int_literal_expression(const too::ast::IntLiteralExpr& int_literal_expr) const {
  return std::to_string(int_literal_expr.value);
}

String PrettyStringVisitor::float_literal_expression(const too::ast::FloatLiteralExpr& float_literal_expr) const {
  return std::to_string(float_literal_expr.value);
}

String PrettyStringVisitor::string_literal_expression(const too::ast::StringLiteralExpr& string_literal_expr) const {
  return "\"" + too::to_string(string_literal_expr.value) + "\"";
}

String PrettyStringVisitor::identifier_literal_expression(const IdentifierLiteralExpr& identifier_literal_expr) const {
  return too::to_string(identifier_literal_expr.name);
}

String PrettyStringVisitor::anonymous_function_literal_expression(const too::ast::AnonymousFunctionLiteralExpr& anon_func_literal_expr) const {
  return "<Fn>";
}

String PrettyStringVisitor::function_call_expression(const too::ast::FunctionCallExpr& func_call_expr) const {
  String result = too::to_string(func_call_expr.name) + "(";
  result += delimited_boxed_list_to_string(*this, func_call_expr.input_arguments, ", ");
  result += ")";
  
  return result;
}

String PrettyStringVisitor::anonymous_function_call_expression(const too::ast::AnonymousFunctionCallExpr& anon_func_call_expr) const {
  String result = anon_func_call_expr.function == nullptr ? "<null>" : anon_func_call_expr.function->accept(*this);
  
  result += "(";
  result += delimited_boxed_list_to_string(*this, anon_func_call_expr.input_arguments, ", ");
  result += ")";
  
  return result;
}

String PrettyStringVisitor::contents_reference_expression(const too::ast::ContentsReferenceExpr& expr) const {
  String result = expr.target_expression == nullptr ? "<null>" : expr.target_expression->accept(*this);
  result += too::to_string_symbol(expr.operator_token);
  result += (expr.reference_expression == nullptr ? "<null>" : expr.reference_expression->accept(*this));
  
  if (expr.operator_token == TokenType::LEFT_BRACKET) {
    result += too::to_string_symbol(TokenType::RIGHT_BRACKET);
  }
  
  return result;
}

String PrettyStringVisitor::assignment_expression(const too::ast::AssignmentExpr& expr) const {
  String result = expr.target_expression == nullptr ? "<null>" : expr.target_expression->accept(*this);
  result += " = ";
  result += (expr.assignment_expression == nullptr ? "<null>" : expr.assignment_expression->accept(*this));
  
  return result;
}

String PrettyStringVisitor::type_parameter(const too::ast::TypeParameter& type_param) const {
  if (type_param.is_function()) {
    return function_type_parameter_to_string(*this, type_param.parameters.value());
    
  } else if (type_param.is_array()) {
    String result = "[" + too::to_string(type_param.name);
    
    if (type_param.parameters != NullOpt{}) {
      result += comma_separated_to_string(*this, type_param.parameters.value());
    }
    
    result += "]";
    return result;
    
  } else {
    String result = too::to_string(type_param.name);
    
    if (type_param.parameters != NullOpt{}) {
      result += wrap_comma_separated_to_string(*this, type_param.parameters.value(), "<", ">");
    }
    
    return result;
  }
}

String PrettyStringVisitor::identifier(const too::ast::Identifier& identifier) const {
  String result = too::to_string(identifier.name);
  
  if (identifier.type != too::NullOpt()) {
    result += (": " + identifier.type.value().accept(*this));
  }
  
  return result;
}

String PrettyStringVisitor::trait_bounded_type(const too::ast::TraitBoundedType& type) const {
  String result = too::to_string(type.type) + ": ";
  int64_t n_traits = type.traits.size();
  
  if (n_traits > 1) {
    result += "{";
  }
  
  result += comma_separated_to_string(*this, type.traits);
  
  if (n_traits > 1) {
    result += "}";
  }
  
  return result;
}

String PrettyStringVisitor::where_clause(const too::ast::WhereClause& clause) const {
  return ("where " + comma_separated_to_string(*this, clause.types));
}

String PrettyStringVisitor::definition_header(const too::ast::DefinitionHeader& header) const {
  String result = too::to_string(header.name);
  
  if (header.type_parameters.size() > 0) {
    result += wrap_comma_separated_to_string(*this, header.type_parameters, "<", ">");
  }
  
  return result;
}

String PrettyStringVisitor::function_definition(const too::ast::FunctionDefinition& func) const {
  String result = func.header.accept(*this) + " :: ";
  
  result += "(";
  result += comma_separated_to_string(*this, func.input_parameters);
  result += ")";
  
  if (func.return_type) {
    result += " -> " + func.return_type.value().accept(*this);
  }
  
  if (func.where_clause) {
    result += " ";
    result += func.where_clause.value().accept(*this);
  }
  
  if (func.body) {
    result += " ";
    
    if (func.body.value() == nullptr) {
      result += "<null>";
    } else {
      result += func.body.value()->accept(*this);
    }
  }
  
  return result;
}

String PrettyStringVisitor::struct_definition(const too::ast::StructDefinition& struct_def) const {
  String result = struct_def.header.accept(*this) + " :: struct ";
  
  if (struct_def.where_clause != too::NullOpt{}) {
    result += " ";
    result += struct_def.where_clause.value().accept(*this);
    result += " ";
  }
  
  result += wrap_delimited_list_to_string(*this, struct_def.members, "{\n  ", "\n}", ",\n  ");
  
  return result;
}

String PrettyStringVisitor::trait_definition(const too::ast::TraitDefinition& trait_def) const {
  String result = trait_def.header.accept(*this) + " :: trait ";
  
  if (trait_def.where_clause != too::NullOpt{}) {
    result += " ";
    result += trait_def.where_clause.value().accept(*this);
    result += " ";
  }
  
  result += "{\n";
  
  for (auto i = 0; i < trait_def.functions.size(); i++) {
    result += "<fn>\n";
  }
  
  result += "}";
  
  return result;
}

String PrettyStringVisitor::definition_context(const too::ast::DefinitionContext& context) const {
  String result = "[module=" + optional_number_to_string(context.enclosing_module);
  result += ",function=" + optional_number_to_string(context.enclosing_function);
  result += ",trait=" + optional_number_to_string(context.enclosing_trait);
  result += ",scope=" + std::to_string(context.scope_depth);
  result += "]";
  
  return result;
}

String PrettyStringVisitor::using_declaration(const too::ast::UsingDeclaration& decl) const {
  String result = String("use ") + too::to_string_symbol(decl.type) + " ";
  
  if (decl.targets) {
    result += wrap_comma_separated_to_string(*this, decl.targets.value(), "{", "}");
    result += " in ";
  }
  
  result += too::to_string(decl.in_module);
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
