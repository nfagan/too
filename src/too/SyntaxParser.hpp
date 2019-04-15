//
//  SyntaxParser.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include "Vector.hpp"
#include "Token.hpp"

namespace too {
  struct SyntaxError {
    std::string message;
  };
  
  struct SyntaxParseResult {
    too::Vector<SyntaxError> syntax_errors;
    bool had_error;
  };
  
  SyntaxParseResult parse(const too::Vector<too::Token>& tokens);
}
