#include "too/Scanner.hpp"
#include "too/Display.hpp"
#include "too/SyntaxParser.hpp"
#include "too/TokenNfa.hpp"
#include "too/Optional.hpp"
#include <iterator>
#include <fstream>
#include <iostream>
#include <chrono>

namespace {
  void print_parse_result(const too::SyntaxParseResult& result) {
    std::cout << result.functions.size() << " functions." << std::endl;
    std::cout << result.structs.size() << " structs." << std::endl;
    std::cout << result.traits.size() << " traits." << std::endl;
    
//    for (auto i = 0; i < result.functions.size(); i++) {
//      const auto& func = result.functions[i];
//      const auto& ctx = func.context;
//
//      if (ctx.parent_scope) {
//        auto enclosing_function_depth = ctx.parent_scope.value();
//
//        std::cout << "--- " << func.header.to_string() << " ";
//        std::cout << "Parent: " << result.functions[enclosing_function_depth].header.to_string() << std::endl;
//      }
//    }
    
    for (auto i = 0; i < result.functions.size(); i++) {
      std::cout << result.functions[i].to_string() << std::endl;
    }
    
    for (auto i = 0; i < result.traits.size(); i++) {
      std::cout << result.traits[i].to_string() << std::endl;
    }
    
    for (auto i = 0; i < result.structs.size(); i++) {
      std::cout << result.structs[i].to_string() << std::endl;
    }
  }
  
  void print_scan_result(const too::ScanResult& scan_result) {
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
  std::string code;
  
  if (argc > 1) {
    code = read_file(argv[1]);
  } else {
    code = read_file("/Users/Nick/repositories/lang/too1/data/scripts/block.rs");
  }
  
  if (!too::utf8::is_valid(code.c_str(), code.size())) {
    std::cout << "Invalid UTF-8 string" << std::endl;
    return 0;
  }
  
  auto t0 = std::chrono::high_resolution_clock::now();
  auto scan_result = too::scan(code.c_str(), code.size());
  auto t1 = std::chrono::high_resolution_clock::now();
  
  auto elapsed = std::chrono::duration<double>(t1 - t0);
  
  std::cout << "Scanned " << code.size() << " bytes in " << (elapsed.count() * 1e3) << "ms" << std::endl;
  
  if (!scan_result.had_error) {
    t0 = std::chrono::high_resolution_clock::now();
    auto parse_result = too::parse_syntax(scan_result.tokens);
    t1 = std::chrono::high_resolution_clock::now();
    
    elapsed = t1 - t0;
    
    std::cout << "Parsed " << scan_result.tokens.size() << " tokens in " << (elapsed.count() * 1e3) << "ms" << std::endl;
    
    print_parse_result(parse_result);
  }
  
  return 0;
}
