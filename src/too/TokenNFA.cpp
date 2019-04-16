//
//  TokenNFA.cpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#include "TokenNFA.hpp"
#include "Vector.hpp"

bool too::matches(const too::PtrWithSize<const too::TokenType>& pattern,
                  const too::PtrWithSize<const too::Token>& token_stream) {
  
  Vector<Vector<int>> nfa;
  
  for (auto i = 0; i < pattern.size; i++) {
    const auto token_type = pattern.data[i];
    
    nfa.push_back(Vector<int>());
    
    if (!is_meta(token_type)) {
      nfa[i].push_back(i + 1);
    } else {
      if (token_type == TokenType::META_GROUP_BEGIN) {
        
      }
    }
  }
  
  return false;
}
