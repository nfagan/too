#include "too/Scanner.hpp"
#include "too/Display.hpp"
#include "too/SyntaxParser.hpp"
#include "too/ExternalResolution.hpp"
#include "too/TokenNfa.hpp"
#include "too/Optional.hpp"
#include "too/ast/PrettyStringVisitor.hpp"
#include <iterator>
#include <fstream>
#include <iostream>
#include <chrono>

namespace {
  void print_functions(const too::ast::PrettyStringVisitor& visitor,
                       const too::Vector<too::ast::FunctionDefinition>& functions) {
    for (auto i = 0; i < functions.size(); i++) {
      std::cout << functions[i].context.accept(visitor);
      
      if (functions[i].context.enclosing_function) {
        const auto& parent_ind = functions[i].context.enclosing_function.value();
        std::cout << " - Parent:" << functions[parent_ind].header.accept(visitor);
      }
      
      std::cout << std::endl << functions[i].accept(visitor) << std::endl;
    }
  }
  
  void print_parse_result(const too::SyntaxParseResult& result) {
    std::cout << result.functions.size() << " functions." << std::endl;
    std::cout << result.structs.size() << " structs." << std::endl;
    std::cout << result.traits.size() << " traits." << std::endl;
    std::cout << result.external_symbols.size() << " external symbols." << std::endl;
    
    too::ast::PrettyStringVisitor visitor;
    print_functions(visitor, result.functions);
    
    for (auto i = 0; i < result.traits.size(); i++) {
      std::cout << result.traits[i].context.accept(visitor) << std::endl;
      std::cout << result.traits[i].accept(visitor) << std::endl;
    }
    
    for (auto i = 0; i < result.structs.size(); i++) {
      std::cout << result.structs[i].context.accept(visitor) << std::endl;
      std::cout << result.structs[i].accept(visitor) << std::endl;
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
  
  too::SyntaxParseResult parse_result;
  
  if (!scan_result.had_error) {
    t0 = std::chrono::high_resolution_clock::now();
    parse_result = too::parse_syntax(scan_result.tokens);
    t1 = std::chrono::high_resolution_clock::now();
    
    elapsed = t1 - t0;
    
    std::cout << "Parsed " << scan_result.tokens.size() << " tokens in " << (elapsed.count() * 1e3) << "ms" << std::endl;
    
//    print_parse_result(parse_result);
  }
  
  std::cout << "Had error ? " << parse_result.had_error << std::endl;
  
  const char* const parent_path = "/Users/Nick/repositories/lang/too1/data/scripts/";
  too::resolve_external_modules(parse_result, parent_path, "module");
  
  return 0;
}
