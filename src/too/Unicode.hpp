//
//  Unicode.hpp
//  too
//
//  Created by Nick Fagan on 4/13/19.
//

#pragma once

#include <cstdint>

namespace too {
  namespace utf8 {
    static constexpr uint8_t bytes_per_code_point = 4;
    
    int count_code_units(const char* str, int64_t len);
    bool is_valid(const char* str, int64_t len);
  }
}
