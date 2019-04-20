//
//  TestUtil.cpp
//  too
//
//  Created by Nick Fagan on 4/19/19.
//

#include "TestUtil.hpp"

std::function<void(void)> too::make_void_func() {
  return []() -> void {};
}

std::function<bool(void)> too::make_bool_func() {
  return []() -> bool {
    return false;
  };
}

too::TestCase too::make_test_case(const std::function<bool(void)>& condition,
                                  const String& pass_message,
                                  const String& fail_message) {
  TestCase test_case;
  
  test_case.condition = condition;
  test_case.pass = DisplayMessage("Passed: " + pass_message);
  test_case.fail = DisplayMessage("* Failed: " + fail_message);
  
  return test_case;
}
