//
//  Display.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include "too/Vector.hpp"
#include "too/Token.hpp"

namespace too {
  void print_tokens(const too::Token* tokens, int64_t size);
  void print_tokens(const too::Vector<too::Token>& tokens);
}
