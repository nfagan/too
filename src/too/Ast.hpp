//
//  Ast.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include "String.hpp"
#include "Optional.hpp"

namespace too {
  namespace ast {
    class Def {};
    class Stmt {};
    class Expr {};
    
    struct Identifier {
      StringView name;
      Optional<StringView> type;
      
      String to_string() const;
    };
    
    struct TypeParameter {
      StringView name;
      
      String to_string() const {
        return too::to_string(name);
      }
    };
    
    struct TraitBoundedType {
      StringView type;
      Vector<TypeParameter> traits;
      
      String to_string() const;
    };
    
    struct WhereClause {
      Vector<TraitBoundedType> types;
      
      String to_string() const;
    };
    
    class FunctionDefinition : public Def {
    public:
      StringView name;
      Vector<TypeParameter> type_parameters;
      Vector<Identifier> input_arguments;
      StringView return_type;
      Optional<WhereClause> where_clause;
      
      String to_string() const;
    };
  }
}
