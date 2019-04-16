//
//  Array.hpp
//  too
//
//  Created by Nick Fagan on 4/15/19.
//

#pragma once

#include <cstdint>

namespace too {
  template <typename T, typename Size = int64_t>
  struct PtrWithSize {
    T* data;
    const Size size;
  };
}
