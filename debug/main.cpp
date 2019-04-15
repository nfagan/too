#include "too/Scanner.hpp"
#include "too/Display.hpp"
#include "too/SyntaxParser.hpp"
#include "too/Optional.hpp"
#include <iterator>
#include <fstream>

namespace {
  void print_result(const too::ScanResult& scan_result) {
    too::print_tokens(scan_result.tokens);
    
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
    return 0;
  }
  
  auto scan_result = too::scan(code.c_str(), code.size());
  
//  print_result(scan_result);
  
  if (!scan_result.had_error) {
    auto parse_result = too::parse(scan_result.tokens);
  }
  
  return 0;
}
