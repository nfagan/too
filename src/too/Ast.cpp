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

String ast::Identifier::to_string() const {
  String result = too::to_string(name);
  
  if (type != too::NullOpt()) {
    result += (": " + too::to_string(type.value()));
  }
  
  return result;
}

String ast::TraitBoundedType::to_string() const {
  String result = too::to_string(type) + ": ";
  int64_t n_traits = traits.size();
  
  if (n_traits > 1) {
    result += "{";
  }
  
  for (int64_t i = 0; i < n_traits; i++) {
    result += too::to_string(traits[i].name);
    
    if (i < n_traits-1) {
      result += ", ";
    }
  }
  
  if (n_traits > 1) {
    result += "}";
  }
  
  return result;
}

String ast::WhereClause::to_string() const {
  String result = "where ";
  
  for (int64_t i = 0; i < types.size(); i++) {
    result += types[i].to_string();
    
    if (i < types.size()-1) {
      result += ", ";
    }
  }
  
  return result;
}

String ast::FunctionDefinition::to_string() const {
  String result = "fn ";
  
  result += (too::to_string(name));
  
  if (type_parameters.size() > 0) {
    result += "<";
    
    for (int64_t i = 0; i < type_parameters.size(); i++) {
      result += type_parameters[i].to_string();
      
      if (i < type_parameters.size()-1) {
        result += ", ";
      }
    }
    
    result += ">";
  }
  
  result += "(";
  
  for (int64_t i = 0; i < input_arguments.size(); i++) {
    result += input_arguments[i].to_string();
    
    if (i < input_arguments.size()-1) {
      result += ", ";
    }
  }
  
  result += ") -> " + too::to_string(return_type);
  
  if (where_clause != too::NullOpt()) {
    result += " ";
    result += where_clause.value().to_string();
  }
  
  return result;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
