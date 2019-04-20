//
//  TokenNfa.hpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#pragma once

#include "Token.hpp"
#include "Array.hpp"
#include "Vector.hpp"

namespace too {
  class TokenRegex;
  
  template <typename T, typename U>
  bool matches(const T& pattern, const U& token_stream);
}

class too::TokenRegex {
public:
  TokenRegex(const TokenType* pattern, const int pattern_size);
  
  template <typename T>
  TokenRegex(const T& in_pattern) : TokenRegex(too::data(in_pattern), too::size(in_pattern)) {
    //
  } 
  
  bool matches(const Token* token_stream, const int64_t stream_size) const;
  bool matches_iterator(const TokenIterator& iterator, const int64_t stream_size) const;
  
  template <typename T>
  bool matches(const T& token_stream) const {
    return matches(too::data(token_stream), too::size(token_stream));
  }
  
private:
  void build_transition_graph();
  
  Vector<int> dfs(const Vector<int>& starts) const;
  Vector<int> dfs(const int* starts, const int n_starts) const;
  
  template <typename T>
  bool matches_stream_or_iterator(const T& token_stream, const int64_t stream_size) const;
  
private:
  Vector<TokenType> pattern;
  Vector<Vector<int>> epsilon_transition_graph;
};

template <typename T, typename U>
bool too::matches(const T& pattern, const U& token_stream) {
  TokenRegex regex(pattern);
  
  return regex.matches(token_stream);
}
