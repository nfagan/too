//
//  Display.cpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#include "Display.hpp"
#include <cstdint>
#include <string>
#include <iostream>

void too::print_tokens(const too::Vector<too::Token>& tokens) {
  print_tokens(tokens.data(), tokens.size());
}

void too::print_tokens(const too::Token* tokens, int64_t size) {
  int64_t max_token_type_size = -1;
  
  for (int64_t i = 0; i < size; i++) {
    int64_t token_type_size = std::strlen(too::to_string(tokens[i].type));
    
    if (token_type_size > max_token_type_size) {
      max_token_type_size = token_type_size;
    }
  }
  
  for (int64_t i = 0; i < size; i++) {
    const auto& token = tokens[i];
    const char* const token_type_str = too::to_string(token.type);
    int n_spaces = int(max_token_type_size) - int(std::strlen(token_type_str));
    
    for (int j = 0; j < n_spaces; j++) {
      std::cout << " ";
    }
    
    std::cout << token << std::endl;
  }
}
