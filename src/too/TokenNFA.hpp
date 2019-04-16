//
//  TokenNFA.hpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#pragma once

#include "Token.hpp"
#include "Array.hpp"

namespace too {
  bool matches(const too::PtrWithSize<const TokenType>& pattern,
               const too::PtrWithSize<const Token>& token_stream);
}
