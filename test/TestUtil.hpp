//
//  TestUtil.hpp
//  too
//
//  Created by Nick Fagan on 4/19/19.
//

#pragma once

#include "too/String.hpp"
#include "too/Vector.hpp"
#include <functional>
#include <iostream>

namespace too {
  std::function<void(void)> make_void_func();
  std::function<bool(void)> make_bool_func();
  
  struct TestCase {
    TestCase() : condition(make_bool_func()), pass(make_void_func()), fail(make_void_func()) {
      //
    }
    
    bool run() const {
      if (condition()) {
        pass();
        
        return true;
      } else {
        fail();
        
        return false;
      }
    }
    
    std::function<bool(void)> condition;
    std::function<void(void)> pass;
    std::function<void(void)> fail;
  };
  
  template <typename T>
  struct AssertThrows {
    std::function<void(void)> test;
    
    bool operator()(void) const {
      try {
        test();
        return false;
      } catch (const T& e) {
        return true;
      } catch (...) {
        return false;
      }
    }
  };
  
  template <typename T>
  struct AssertNoThrows {
    std::function<void(void)> test;
    
    bool operator()(void) const {
      try {
        test();
        return true;
      } catch (const T& e) {
        return false;
      } catch (...) {
        return false;
      }
    }
  };
  
  struct DisplayMessage {
    String message;
    
    DisplayMessage(const String& msg) : message(msg) {
      //
    }
    
    void operator()(void) const {
      std::cout << message << std::endl;
    }
  };
  
  class TestSuite {
  public:
    TestSuite() : start(make_void_func()), n_passed(0) {
      //
    }
    
    void add_test(const TestCase& test_case) {
      test_cases.push_back(test_case);
    }
    
    void run() {
      start();
      
      for (auto i = 0; i < test_cases.size(); i++) {
        n_passed += test_cases[i].run();
      }
      
      summarize();
    }
    
    std::function<void(void)> start;
    
  private:
    void summarize() {
      std::cout << test_cases.size() << " tests run, ";
      std::cout << n_passed << " passed." << std::endl;
    }
    
  private:
    Vector<TestCase> test_cases;
    int n_passed;
  };
  
  TestCase make_test_case(const std::function<bool(void)>& condition,
                          const String& pass_message,
                          const String& fail_message);
}
