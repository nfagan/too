#include "too/Scanner.hpp"
#include <vector>

struct TestStruct {
  int a;
  int b;
};

int main(int argc, char* argv[]) {
//  std::string code = "1.0000002 + 1 + (2 * 2.02 / 2): \n \"a \\\"a;\" ";
  
  std::string code = "(11.0001 + 2) === -> < <= !!= fn bold() {} bold_a 1.0 trait impl if{ } else {} in \"éée\" trai";
  
  auto scan_result = too::scan(code.c_str(), code.length());
  
  std::cout << "BEGAN WITH: \n" << code << std::endl << std::endl;
  
  for (int64_t i = 0; i < scan_result.tokens.size(); i++) {
    std::cout << scan_result.tokens[i] << std::endl;
  }
  
  std::cout << "Had error ? " << std::boolalpha << scan_result.had_error << std::endl;
  
  for (int64_t i = 0; i < scan_result.errors.size(); i++) {
    std::cout << scan_result.errors[i].type << std::endl;
  }
  
  std::cout << sizeof(std::string) << std::endl;
  std::cout << sizeof(too::Character) << std::endl;
  
  std::string str2 = "é";
  std::cout << too::utf8::count_code_units(str2.c_str(), str2.size()) << std::endl;
  
  std::cout << "is valid? " << std::boolalpha << too::utf8::is_valid(str2.c_str(), str2.size()) << std::endl;
  std::cout << "is valid? " << std::boolalpha << too::utf8::is_valid(code.c_str(), code.size()) << std::endl;
  
  return EXIT_SUCCESS;
}
