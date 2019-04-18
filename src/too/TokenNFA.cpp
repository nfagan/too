//
//  TokenNFA.cpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#include "TokenNFA.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <iostream>

namespace {  
  too::Vector<int64_t> dfs(const too::Vector<too::Vector<int64_t>>& graph,
                           const too::Vector<int64_t>& starts) {
    
    using namespace too;
    
    Vector<bool> marked;
    marked.resize(graph.size());
    std::fill(marked.data(), marked.data() + marked.size(), false);
    
    Vector<int64_t> stack;
    
    for (auto i = 0; i < starts.size(); i++) {
      stack.push_back(starts[i]);
    }
    
    while (!stack.empty()) {
      auto next = stack.back();
      stack.pop_back();
      
      marked[next] = true;
      
      const auto& adjacents = graph[next];
      
      for (auto i = 0; i < adjacents.size(); i++) {
        auto adj = adjacents[i];
        
        if (!marked[adj]) {
          marked[adj] = true;
          stack.push_back(adj);
        }
      }
    }
    
    decltype(stack) result;
    
    for (auto i = 0; i < marked.size(); i++) {
      if (marked[i]) {
        result.push_back(i);
      }
    }
    
    return result;
  }
  
  too::Vector<too::Vector<int64_t>> build_nfa(const too::PtrWithSize<const too::TokenType>& pattern) {
    using namespace too;
    
    Vector<Vector<int64_t>> nfa;
    nfa.resize(pattern.size + 1);
    
    Vector<int64_t> group_starts;
    int64_t i = 0;
    
    while (i < pattern.size) {
      auto token_type = pattern[i];
      
      if (!is_meta(token_type)) {
        nfa[i].push_back(i + 1);
        
      } else {
        if (token_type == TokenType::META_GROUP_BEGIN) {
          group_starts.push_back(i);
          
        } else if (token_type == TokenType::META_GROUP_END) {
          auto group_start = group_starts.back();
          group_starts.pop_back();
          
          nfa[group_start].push_back(i);
        
        } else if (token_type == TokenType::META_ONCE_OR_MORE) {
          auto prev = pattern[i-1];
          
          
        }
        
      }
      
      i++;
    }
    
    return nfa;
  }
}

bool too::matches(const too::PtrWithSize<const too::TokenType>& pattern,
                  const too::PtrWithSize<const too::Token>& token_stream) {
  
  if (pattern.size == 0) {
    return false;
  }
  
  auto nfa = build_nfa(pattern);
  auto end_state = pattern.size + 1;
  
  Vector<TokenType> match;
  match.resize(pattern.size);
  
  Vector<int64_t> reachable_states;
  reachable_states.push_back(0);
  
  for (auto i = 0; i < token_stream.size; i++) {
    const auto& token_type = token_stream[i].type;
    
    Vector<int64_t> search_next;
    
    for (auto i = 0; i < reachable_states.size(); i++) {
      if (pattern[i] == token_type) {
        search_next.push_back(reachable_states[i] + 1);
      }
    }
    
    reachable_states = dfs(nfa, search_next);
  }
  
  for (auto i = 0; i < reachable_states.size(); i++) {
    if (i == end_state) {
      return true;
    }
  }
  
  return false;
}
