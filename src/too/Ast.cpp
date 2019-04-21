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
  
  if (where_clause != too::NullOpt{}) {
    result += " ";
    result += where_clause.value().to_string();
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

String ast::LetStatement::to_string() const {
  String result = "let " + identifier.to_string();
  
  if (initializer && initializer.value() != nullptr) {
    result += " = " + initializer.value()->to_string();
  }
  
  result += ';';
  
  return result;
}

String ast::UnaryExpr::to_string() const {
  return String(too::to_string_symbol(operator_token)) + "(" + expression->to_string() + ")";
}

String ast::BinaryExpr::to_string() const {
  auto left_str = left->to_string();
  auto right_str = right->to_string();
  auto op = too::to_string_symbol(operator_token);
  
  auto left_wrapped = "(" + left_str + ")";
  auto right_wrapped = "(" + right_str + ")";
  
  return left_wrapped + op + right_wrapped;
}

String ast::IntLiteralExpr::to_string() const {
  return "<int>";
}

String ast::FloatLiteralExpr::to_string() const {
  return "<float>";
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
