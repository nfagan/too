//
//  ExternalResolution.hpp
//  too
//
//  Created by Nick Fagan on 4/28/19.
//

#pragma once

#include "SyntaxParser.hpp"
#include "String.hpp"

namespace too {
  void resolve_external_modules(const SyntaxParseResult& result,
                                const String& parent_path,
                                const String& parent_name);
}
