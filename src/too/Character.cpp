//
//  Character.cpp
//  too
//
//  Created by Nick Fagan on 4/13/19.
//

#include "Character.hpp"

//
//
//  CharacterIterator

too::CharacterIterator::CharacterIterator() : CharacterIterator(nullptr, 0) {
  //
}

too::CharacterIterator::CharacterIterator(const char* str, int64_t len) :
current_index(0), end(len), str(str) {
  //
}

int64_t too::CharacterIterator::next_index() const {
  return current_index;
}

const char* too::CharacterIterator::data() const {
  return str;
}

bool too::CharacterIterator::has_next() const {
  return current_index < end;
}

too::Character too::CharacterIterator::advance() {
  int n_next = utf8::count_code_units(str + current_index, end - current_index);
  
  //  Invalid character.
  if (n_next == 0) {
    current_index = end;
    
    return too::Character('\0');
  }
  
  too::Character result(str + current_index, n_next);
  current_index += n_next;
  
  return result;
}

too::Character too::CharacterIterator::peek() const {
  if (current_index >= end) {
    return too::Character('\0');
  }
  
  int n_next = utf8::count_code_units(str + current_index, end - current_index);
  
  if (n_next == 0) {
    return too::Character('\0');
  }
  
  return too::Character(str + current_index, n_next);
}
