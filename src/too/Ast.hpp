//
//  Ast.hpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#pragma once

#include "String.hpp"
#include "Optional.hpp"
#include "Types.hpp"
#include <memory>
#include <iostream>

namespace too {
  enum class TokenType;
  
  namespace ast {
    struct Def {};
    struct Stmt {};
    struct Expr {
      virtual String to_string() const = 0;
      virtual ~Expr() = default;
    };
    
    using BoxedExpr = std::unique_ptr<Expr>;
    using BoxedStmt = std::unique_ptr<Stmt>;
    
    struct UnaryExpr : public Expr {
      TokenType operator_token;
      BoxedExpr expression;
      
      UnaryExpr() = default;
      ~UnaryExpr() = default;
      
      UnaryExpr(TokenType op, BoxedExpr&& expr) : operator_token(op), expression(std::move(expr)) {
        //
      }
      
      String to_string() const override;
    };
    
    struct BinaryExpr : public Expr {
      TokenType operator_token;
      BoxedExpr left;
      BoxedExpr right;
      
      BinaryExpr() = default;
      ~BinaryExpr() = default;
      
      BinaryExpr(TokenType op, BoxedExpr&& left, BoxedExpr&& right) :
        operator_token(op), left(std::move(left)), right(std::move(right)) {
          //
      }
      
      String to_string() const override;
    };
    
    struct IntLiteralExpr : public Expr {
      TooInteger value;
      
      IntLiteralExpr(TooInteger value) : value(value) {}
      
      IntLiteralExpr() = default;
      ~IntLiteralExpr() = default;
      String to_string() const override;
    };
    
    struct FloatLiteralExpr : public Expr {
      TooFloat value;
      
      FloatLiteralExpr(TooFloat value) : value(value) {}
      
      FloatLiteralExpr() = default;
      ~FloatLiteralExpr() = default;
      String to_string() const override;
    };
    
    struct TypeParameter {
      StringView name;
      Optional<Vector<TypeParameter>> parameters;
      
      String to_string() const;
    };
    
    struct Identifier {
      StringView name;
      Optional<TypeParameter> type;
      
      String to_string() const;
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
    
    struct FunctionDefinition : public Def {
      StringView name;
      Vector<TypeParameter> type_parameters;
      Optional<WhereClause> where_clause;
      Vector<Identifier> input_parameters;
      TypeParameter return_type;
      
      Optional<BoxedStmt> body;
      
      String to_string() const;
    };
    
    struct StructDefinition : public Def {
      StringView name;
      Vector<TypeParameter> type_parameters;
      Optional<WhereClause> where_clause;
      Vector<Identifier> members;
      
      String to_string() const;
    };
    
    struct TraitDefinition : public Def {
      StringView name;
      Vector<TypeParameter> type_parameters;
      Optional<WhereClause> where_clause;
      Vector<FunctionDefinition> functions;
      
      String to_string() const;
    };
    
    struct LetStatement : public Stmt {
      Identifier identifier;
      Optional<BoxedExpr> initializer;
      
      String to_string() const;
    };
  }
}
