//
//  Scanner.hpp
//  too
//
//  Created by Nick Fagan on 4/6/19.
//

#pragma once

#include "String.hpp"
#include "Vector.hpp"
#include "Token.hpp"
#include <cstdint>
#include <ostream>

namespace too {
  enum class ScanErrorType {
    UNTERMINATED_STRING,
    UNRECOGNIZED_SYMBOL
  };
  
  struct ScanError {
    ScanErrorType type;
    
    int64_t begin;
    int64_t end;
  };
  
  struct ScanResult {
    too::Vector<too::Token> tokens;
    too::Vector<too::ScanError> errors;
    bool had_error;
  };
  
  ScanResult scan(const String& code);
  ScanResult scan(const char* code, int64_t len);
}

std::ostream& operator<<(std::ostream& os, const too::ScanErrorType& type);
