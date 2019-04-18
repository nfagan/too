#include "too/Scanner.hpp"
#include <iterator>
#include <fstream>
#include <iostream>

namespace {
  void print_result(const too::ScanResult& scan_result) {
    for (int64_t i = 0; i < scan_result.tokens.size(); i++) {
      std::cout << scan_result.tokens[i] << std::endl;
    }
    
    std::cout << "Had error ? " << std::boolalpha << scan_result.had_error << std::endl;
    
    for (int64_t i = 0; i < scan_result.errors.size(); i++) {
      std::cout << scan_result.errors[i].type << std::endl;
    }
  }
  
  std::string read_file(const char* filename) {
    std::ifstream infile{filename};
    
    if (!infile.good()) {
      return "";
    }
    
    return {std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
  }
}

int main(int argc, char* argv[]) {
  std::string code = "let (11.0001 + 2) === -> < <= !!= fn bold() {} bold_a 1.0 trait impl if{ } else {} in \"éée\" trai";
  
  if (argc > 1) {
    code = read_file(argv[1]);
  }
  
  if (!too::utf8::is_valid(code.c_str(), code.size())) {
    std::cout << "Invalid UTF-8 string" << std::endl;
    return EXIT_SUCCESS;
  }
  
  auto scan_result = too::scan(code.c_str(), code.size());
  
  print_result(scan_result);
  
  return EXIT_SUCCESS;
}
