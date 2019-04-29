//
//  SyntaxParser.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include "Vector.hpp"
#include "Token.hpp"
#include "String.hpp"
#include "Ast.hpp"
#include <set>

namespace too {
  struct SyntaxError {
    String message;
  };
  
  struct DefinitionRegistry {
    std::set<ast::NamedDefinitionContext> contents;
    bool register_definition(const StringView& name, const ast::DefinitionContext& context);
  };
  
  struct SyntaxParseResult {
    Vector<SyntaxError> syntax_errors;
    
    Vector<ast::FunctionDefinition> functions;
    Vector<ast::StructDefinition> structs;
    Vector<ast::TraitDefinition> traits;
    Vector<ast::UsingDeclaration> external_symbols;
    
    DefinitionRegistry function_registry;
    DefinitionRegistry struct_registry;
    DefinitionRegistry trait_registry;
    DefinitionRegistry identifier_registry;
    DefinitionRegistry module_registry;
    
    bool had_error;
  };
  
  SyntaxParseResult parse_syntax(const too::Vector<too::Token>& tokens);
}
