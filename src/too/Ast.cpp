//
//  Ast.cpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#include "Ast.hpp"
#include "Token.hpp"

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

template <typename T>
String delimited_list_to_string(const Vector<T>& values, const char* const delimiter) {
  String result;
  
  for (auto i = 0; i < values.size(); i++) {
    result += values[i].to_string();
    
    if (i < values.size()-1) {
      result += delimiter;
    }
  }
  
  return result;
}

template <typename T>
String delimited_boxed_list_to_string(const Vector<T>& values, const char* const delimiter) {
  String result;
  
  for (auto i = 0; i < values.size(); i++) {
    result += values[i]->to_string();
    
    if (i < values.size()-1) {
      result += delimiter;
    }
  }
  
  return result;
}

template <typename T>
String comma_separated_to_string(const Vector<T>& values) {
  return delimited_list_to_string(values, ", ");
}

template <typename T>
inline String wrap_comma_separated_to_string(const Vector<T>& values, const String& lhs, const String& rhs) {
  return lhs + comma_separated_to_string(values) + rhs;
}

template <typename T>
inline String wrap_delimited_list_to_string(const Vector<T>& values,
                                            const String& lhs,
                                            const String& rhs,
                                            const char* const delimiter) {
  return lhs + delimited_list_to_string(values, delimiter) + rhs;
}

String ast::Identifier::to_string() const {
  String result = too::to_string(name);
  
  if (type != too::NullOpt()) {
    result += (": " + type.value().to_string());
  }
  
  return result;
}

String ast::TypeParameter::to_string() const {
  String result = too::to_string(name);
  
  if (parameters != NullOpt{}) {
    result += wrap_comma_separated_to_string(parameters.value(), "<", ">");
  }
  
  return result;
}

String ast::TraitBoundedType::to_string() const {
  String result = too::to_string(type) + ": ";
  int64_t n_traits = traits.size();
  
  if (n_traits > 1) {
    result += "{";
  }
  
  result += comma_separated_to_string(traits);
  
  if (n_traits > 1) {
    result += "}";
  }
  
  return result;
}

String ast::WhereClause::to_string() const {
  return ("where " + comma_separated_to_string(types));
}

String ast::FunctionDefinition::to_string() const {
  String result = "fn ";
  
  result += too::to_string(name);
  
  if (type_parameters.size() > 0) {
    result += wrap_comma_separated_to_string(type_parameters, "<", ">");
  }
  
  result += "(";
  result += comma_separated_to_string(input_parameters);
  result += ") -> " + return_type.to_string();
  
  if (where_clause) {
    result += " ";
    result += where_clause.value().to_string();
  }
  
  if (body) {
    result += " ";
    
    if (body.value() == nullptr) {
      result += "<null>";
    } else {
      result += body.value()->to_string();
    }
  }
  
  return result;
}

String ast::StructDefinition::to_string() const {
  String result = "struct ";
  
  result += too::to_string(name);
  
  if (type_parameters.size() > 0) {
    result += wrap_comma_separated_to_string(type_parameters, "<", ">");
  }
  
  if (where_clause != too::NullOpt{}) {
    result += " ";
    result += where_clause.value().to_string();
    result += " ";
  }
  
  result += wrap_delimited_list_to_string(members, "{\n  ", "\n}", ",\n  ");
  
  return result;
}

String ast::TraitDefinition::to_string() const {
  String result = "trait ";
  
  result += too::to_string(name);
  
  if (type_parameters.size() > 0) {
    result += wrap_comma_separated_to_string(type_parameters, "<", ">");
  }
  
  if (where_clause != too::NullOpt{}) {
    result += " ";
    result += where_clause.value().to_string();
    result += " ";
  }
  
  result += wrap_delimited_list_to_string(functions, "{\n  ", "\n}", ",\n  ");
  
  return result;
}

String ast::LetStmt::to_string() const {
  String result = "let " + identifier.to_string();
  
  if (initializer && initializer.value() != nullptr) {
    result += " = " + initializer.value()->to_string();
  }
  
  result += ';';
  
  return result;
}

String ast::ExprStmt::to_string() const {
  return expression->to_string() + ";";
}

String ast::BlockStmt::to_string() const {
  String result = "{\n";
  
  for (auto i = 0; i < statements.size(); i++) {
    result += statements[i]->to_string();
    result += '\n';
  }
  
  result += "}\n";
  
  return result;
}

String ast::ReturnStmt::to_string() const {
  String result = "return";
  
  if (expression) {
    result += " ";
    result += expression.value() == nullptr ? "<null>" : expression.value()->to_string();
    result += ";";
  }
  
  return result;
}

String ast::UnaryExpr::to_string() const {
  String expr = expression == nullptr ? "<null>" : expression->to_string();
  return String(too::to_string_symbol(operator_token)) + "(" + expr + ")";
}

String ast::BinaryExpr::to_string() const {
  auto left_str = left == nullptr ? "<null>" : left->to_string();
  auto right_str = right == nullptr ? "<null>" : right->to_string();
  auto op = too::to_string_symbol(operator_token);
  
  auto left_wrapped = "(" + left_str + ")";
  auto right_wrapped = "(" + right_str + ")";
  
  return left_wrapped + op + right_wrapped;
}

String ast::IntLiteralExpr::to_string() const {
  return std::to_string(value);
}

String ast::FloatLiteralExpr::to_string() const {
  return std::to_string(value);
}

String ast::StringLiteralExpr::to_string() const {
  return "\"" + too::to_string(value) + "\"";
}

String ast::IdentifierLiteralExpr::to_string() const {
  return too::to_string(name);
}

String ast::FunctionCallExpr::to_string() const {
  String result = too::to_string(name) + "(";
  result += delimited_boxed_list_to_string(input_arguments, ", ");
  result += ")";
  
  return result;
}

String ast::AnonymousFunctionCallExpr::to_string() const {
  String result = function == nullptr ? "<null>" : function->to_string();
  
  result += "(";
  result += delimited_boxed_list_to_string(input_arguments, ", ");
  result += ")";
  
  return result;
}

String ast::ContentsReferenceExpr::to_string() const {
  String result = target_expression == nullptr ? "<null>" : target_expression->to_string();
  result += too::to_string_symbol(operator_token);
  result += (reference_expression == nullptr ? "<null>" : reference_expression->to_string());
  
  if (operator_token == TokenType::LEFT_BRACKET) {
    result += too::to_string_symbol(TokenType::RIGHT_BRACKET);
  }
  
  return result;
}

String ast::AssignmentExpr::to_string() const {
  String result = target_expression == nullptr ? "<null>" : target_expression->to_string();
  result += " = ";
  result += (assignment_expression == nullptr ? "<null>" : assignment_expression->to_string());
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
