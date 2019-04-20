#include "test_token_nfa.hpp"

int main(int argc, char* argv[]) {
  
  auto token_suite = too::get_token_nfa_test_suite();
  token_suite.run();
  
  return 0;
}
