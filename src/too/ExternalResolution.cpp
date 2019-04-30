//
//  ExternalResolution.cpp
//  too
//
//  Created by Nick Fagan on 4/28/19.
//

#include "ExternalResolution.hpp"
#include "Scanner.hpp"
#include <iostream>
#include <fstream>
#include <map>

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

String read_file(const char* filename) {
  std::ifstream infile{filename};
  
  if (!infile.good()) {
    return "";
  }
  
  return {std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
}

Optional<SyntaxParseResult> read_external_module_file(const String& path, String& module_file) {
  module_file = read_file(path.c_str());
  if (module_file.empty() || !utf8::is_valid(module_file.c_str(), module_file.length())) {
    std::cout << "Failed to read file: " << path << std::endl;
    return NullOpt{};
  }
  
  auto scan_result = scan(module_file);
  if (scan_result.had_error) {
    std::cout << "Failed to scan file: " << path << std::endl;
    return NullOpt{};
  }
  
  auto parse_result = parse_syntax(scan_result.tokens);
  if (parse_result.had_error) {
    std::cout << "Failed to parse file: " << path << std::endl;
    return NullOpt{};
  }
  
  return Optional<SyntaxParseResult>(std::move(parse_result));
}

bool parse_modules(const Vector<ast::UsingDeclaration>& external_modules,
                   std::map<StringView, SyntaxParseResult>& into,
                   std::set<String>& visited_files,
                   const String& parent_path,
                   const String& parent_full_path) {
  
  for (auto i = 0; i < external_modules.size(); i++) {
    StringView module_name = external_modules[i].in_module;
    String module_path = parent_path + too::to_string(module_name) + ".rs";
    
    if (into.count(module_name) > 0 || module_path == parent_full_path) {
      continue;
    }
    
    String read_file;
    auto parse_result = read_external_module_file(module_path, read_file);
    if (!parse_result) {
      return false;
    }
    
    auto nested_externals = parse_result.value().external_symbols;
    into.emplace(module_name, parse_result.rvalue());
    visited_files.emplace(std::move(read_file));
    
    if (!parse_modules(nested_externals, into, visited_files, parent_path, parent_full_path)) {
      return false;
    }
  }
  
  return true;
}

void resolve_external_modules(const SyntaxParseResult& result,
                              const String& parent_path,
                              const String& parent_name) {
  auto parent_full_path = parent_path + parent_name + ".rs";
  std::map<StringView, SyntaxParseResult> parse_results;
  std::set<String> visited_files;
  
  if (!parse_modules(result.external_symbols, parse_results, visited_files, parent_path, parent_full_path)) {
    return;
  }
  
  for (auto i = 0; i < result.external_symbols.size(); i++) {
    const auto& mod = result.external_symbols[i];
    if (mod.targets) {
      const auto& targets = mod.targets.value();
      const auto& referenced_module = parse_results.at(mod.in_module);
      const auto type = mod.type;
    }
  }
  
  std::cout << "OK: " << parse_results.size() << std::endl;
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
