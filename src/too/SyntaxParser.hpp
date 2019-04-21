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

namespace too {
  struct SyntaxError {
    String message;
  };
  
  struct SyntaxParseResult {
    Vector<SyntaxError> syntax_errors;
    
    Vector<ast::FunctionDefinition> functions;
    Vector<ast::StructDefinition> structs;
    Vector<ast::TraitDefinition> traits;
    
    bool had_error;
  };
  
  SyntaxParseResult parse_syntax(const too::Vector<too::Token>& tokens);
}
