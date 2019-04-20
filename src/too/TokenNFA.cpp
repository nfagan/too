//
//  TokenNfa.cpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#include "TokenNfa.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <type_traits>

too::TokenRegex::TokenRegex(const too::TokenType* pattern_ptr, const int pattern_size) :
  pattern(pattern_ptr, pattern_ptr + pattern_size) {
    build_transition_graph();
}

void too::TokenRegex::build_transition_graph() {
  auto pattern_size = too::size(pattern);
  
  epsilon_transition_graph.resize(pattern_size + 1);
  
  Vector<int64_t> group_starts;
  int64_t group_start = -1;
  
  for (auto i = 0; i < pattern_size; i++) {
    const auto token_type = pattern[i];
    
    if (is_meta(token_type)) {
      epsilon_transition_graph[i].push_back(i + 1);
    }
    
    if (token_type == TokenType::META_GROUP_BEGIN) {
      group_starts.push_back(i);
      
    } else if (token_type == TokenType::META_GROUP_END) {
      //  @FIXME: Error if no META_GROUP_BEGIN
      assert(!group_starts.empty() && "Expect META_GROUP_BEGIN before META_GROUP_END.");
      
      group_start = group_starts.back();
      group_starts.pop_back();
      
    } else if (token_type == TokenType::META_OR) {
      //  @FIXME: Error if META_OR at 0
      assert(i > 0 && "Expect token literal before META_OR.");
      
      auto prev_index = i - 1;
      auto prev = pattern[prev_index];
      
      if (prev == TokenType::META_GROUP_END) {
        prev_index = group_start;
      }
      
      auto next_index = i + 1;
      
      epsilon_transition_graph[prev_index].push_back(i + 2);
      epsilon_transition_graph[next_index].push_back(i + 2);
      
    } else if (token_type == TokenType::META_ZERO_OR_MORE) {
      //  @FIXME: Handle empty group: ()
      //  @FIXME: Error if META_ZERO_OR_MORE at 0
      assert(i > 0 && "Expect token literal before META_ZERO_OR_MORE.");
      
      auto prev_index = i - 1;
      auto prev = pattern[prev_index];
      
      if (prev == TokenType::META_GROUP_END) {
        prev_index = group_start;
      }
      
      epsilon_transition_graph[prev_index].push_back(i);
      epsilon_transition_graph[i].push_back(prev_index);
      
    } else if (token_type == TokenType::META_ONCE_OR_MORE) {
      //  @FIXME: Error if META_ONCE_OR_MORE at 0
      assert(i > 0 && "Expect token literal before META_ONCE_OR_MORE.");
      
      auto prev_index = i - 1;
      auto prev = pattern[prev_index];
      
      if (prev == TokenType::META_GROUP_END) {
        prev_index = group_start;
      }
      
      epsilon_transition_graph[i].push_back(prev_index);
      
    } else if (token_type == TokenType::META_ONCE_OR_NEVER) {
      //  @FIXME: Error if META_ONCE_OR_NEVER at 0
      assert(i > 0 && "Expect token literal before META_ONCE_OR_NEVER.");
      
      auto prev_index = i - 1;
      auto prev = pattern[prev_index];
      
      if (prev == TokenType::META_GROUP_END) {
        prev_index = group_start;
      }
      
      epsilon_transition_graph[prev_index].push_back(i);
    }
  }
}

too::Vector<int> too::TokenRegex::dfs(const too::Vector<int>& starts) const {
  return dfs(starts.data(), starts.size());
}

too::Vector<int> too::TokenRegex::dfs(const int* starts, const int n_starts) const {
  Vector<bool> marked(epsilon_transition_graph.size());
  
#ifdef USE_TOO_VECTOR
  std::fill(marked.data(), marked.data() + marked.size(), false);
#else
  std::fill(marked.begin(), marked.end(), false);
#endif
  
  Vector<int> stack(starts, starts + n_starts);
  
  while (!stack.empty()) {
    const auto next = stack.back();
    stack.pop_back();
    
    marked[next] = true;
    
    const auto& adjacents = epsilon_transition_graph[next];
    
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

struct FromStream {
  template <typename T>
  static inline std::enable_if_t<std::is_pointer_v<T>, const too::TokenType&> get(const T data, const int64_t at) {
    return data[at].type;
  }
  
  template <typename T, std::enable_if_t<!std::is_pointer_v<T>, int> = 0>
  static inline const too::TokenType& get(const T& iterator, const int64_t at) {
    return iterator.peek(at).type;
  }
};

template <typename T>
bool too::TokenRegex::matches_stream_or_iterator(const T& token_stream, const int64_t stream_size) const {
  const auto pattern_size = too::size(pattern);
  
  if (pattern_size == 0) {
    return false;
  }
  
  const int state0 = 0;
  auto reachable_states = dfs(&state0, 1);
  
  for (auto i = 0; i < stream_size; i++) {
    const auto& token_type = FromStream::get<T>(token_stream, i);
    
    Vector<int> search_next;
    
    for (auto j = 0; j < reachable_states.size(); j++) {
      const auto state = reachable_states[j];
      
      if (state == pattern_size) {
        return true;
      }
      
      if (pattern[state] == token_type) {
        search_next.push_back(state + 1);
      }
    }
    
    reachable_states = dfs(search_next);
  }
  
  for (auto i = 0; i < reachable_states.size(); i++) {
    if (reachable_states[i] == pattern_size) {
      return true;
    }
  }
  
  return false;
}

bool too::TokenRegex::matches(const Token* token_stream, const int64_t stream_size) const {
  return matches_stream_or_iterator<const Token*>(token_stream, stream_size);
}

bool too::TokenRegex::matches_iterator(const too::TokenIterator& iterator, const int64_t stream_size) const {
  return matches_stream_or_iterator<TokenIterator>(iterator, stream_size);
}
