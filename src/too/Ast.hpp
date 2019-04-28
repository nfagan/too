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
    struct Stmt {
      virtual String to_string() const = 0;
      virtual ~Stmt() = default;
    };
    
    struct Expr {
      virtual String to_string() const = 0;
      virtual bool is_valid_assignment_target() const {
        return false;
      }
      
      virtual ~Expr() = default;
    };
    
    using BoxedExpr = std::unique_ptr<Expr>;
    using BoxedStmt = std::unique_ptr<Stmt>;
    
    struct TypeParameter {
      StringView name;
      Optional<Vector<TypeParameter>> parameters;
      uint32_t type_flags;
      
      TypeParameter() : type_flags(0) {
        //
      }
      
      void mark_function() {
        type_flags |= (uint32_t(1) << 1);
      }
      
      void mark_array() {
        type_flags |= (uint32_t(1) << 2);
      }
      
      bool is_function() const {
        return (type_flags & (uint32_t(1) << 1));
      }
      
      bool is_array() const {
        return (type_flags & (uint32_t(1) << 2));
      }
      
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
    
    struct LetStmt : public Stmt {
      Identifier identifier;
      Optional<BoxedExpr> initializer;
      
      LetStmt(LetStmt&& other) :
      identifier(std::move(other.identifier)),
      initializer(std::move(other.initializer)) {
        //
      }
      
      LetStmt& operator=(LetStmt&& other) {
        identifier = std::move(other.identifier);
        initializer = std::move(other.initializer);
        
        return *this;
      }
      
      LetStmt() = default;
      ~LetStmt() = default;
      
      String to_string() const override;
    };
    
    struct ExprStmt : public Stmt {
      BoxedExpr expression;
      
      ExprStmt() = default;
      ~ExprStmt() = default;
      
      ExprStmt(BoxedExpr&& expr) : expression(std::move(expr)) {
        //
      }
      
      ExprStmt(ExprStmt&& other) : expression(std::move(other.expression)) {
        //
      }
      
      ExprStmt& operator=(ExprStmt&& other) {
        expression = std::move(other.expression);
        return *this;
      }
      
      String to_string() const override;
    };
    
    struct BlockStmt : public Stmt {
      Vector<BoxedStmt> statements;
      
      BlockStmt() = default;
      ~BlockStmt() = default;
      
      BlockStmt(BlockStmt&& other) : statements(std::move(other.statements)) {
        //
      }
      
      BlockStmt& operator=(BlockStmt&& other) {
        statements = std::move(other.statements);
        return *this;
      }
      
      String to_string() const override;
    };
    
    struct ReturnStmt : public Stmt {
      Optional<BoxedExpr> expression;
      
      ReturnStmt() = default;
      ~ReturnStmt() = default;
      ReturnStmt(ReturnStmt&& other) : expression(std::move(other.expression)) {
        //
      }
      ReturnStmt(BoxedExpr&& expr) : expression(std::move(expr)) {
        //
      }
      
      ReturnStmt& operator=(ReturnStmt&& other) {
        expression = std::move(other.expression);
        return *this;
      }
      
      String to_string() const override;
    };
    
    struct DefinitionContext {
      int64_t enclosing_module;
      Optional<int64_t> parent_scope;
      
      DefinitionContext() : enclosing_module(0) {
        //
      }
      
      String to_string() const;
    };
    
    struct DefinitionHeader {
      StringView name;
      Vector<TypeParameter> type_parameters;
      
      String to_string() const;
    };
    
    struct FunctionDefinition {
      DefinitionHeader header;
      Optional<WhereClause> where_clause;
      Vector<Identifier> input_parameters;
      Optional<TypeParameter> return_type;
      Optional<BoxedStmt> body;
      
      DefinitionContext context;
      
      String to_string() const;
    };
    
    struct StructDefinition {
      DefinitionHeader header;
      Optional<WhereClause> where_clause;
      Vector<Identifier> members;
      
      DefinitionContext context;
      
      String to_string() const;
    };
    
    struct TraitDefinition {
      DefinitionHeader header;
      Optional<WhereClause> where_clause;
      Vector<FunctionDefinition> functions;
      
      DefinitionContext context;
      
      String to_string() const;
    };
    
    struct UnaryExpr : public Expr {
      TokenType operator_token;
      BoxedExpr expression;
      
      UnaryExpr() : expression(nullptr) {
        //
      }
      
      ~UnaryExpr() = default;
      
      UnaryExpr(UnaryExpr&& other) :
      operator_token(other.operator_token),
      expression(std::move(other.expression)) {
        //
      }
      
      UnaryExpr(TokenType op, BoxedExpr&& expr) : operator_token(op), expression(std::move(expr)) {
        //
      }
      
      UnaryExpr& operator=(UnaryExpr&& other) {
        operator_token = other.operator_token;
        expression = std::move(other.expression);
        
        return *this;
      }
      
      String to_string() const override;
    };
    
    struct BinaryExpr : public Expr {
      TokenType operator_token;
      BoxedExpr left;
      BoxedExpr right;
      
      BinaryExpr() = default;
      ~BinaryExpr() = default;
      
      BinaryExpr& operator=(BinaryExpr&& other) {
        operator_token = other.operator_token;
        left = std::move(other.left);
        right = std::move(other.right);
        
        return *this;
      }
      
      BinaryExpr(BinaryExpr&& other) :
      operator_token(other.operator_token),
      left(std::move(other.left)),
      right(std::move(other.right)) {
        //
      }
      
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
    
    struct StringLiteralExpr : public Expr {
      StringView value;
      
      StringLiteralExpr(const StringView& value) : value(value) {}
      
      StringLiteralExpr() = default;
      ~StringLiteralExpr() = default;
      
      String to_string() const override;
    };
    
    struct IdentifierLiteralExpr : public Expr {
      StringView name;
      
      IdentifierLiteralExpr() = default;
      IdentifierLiteralExpr(const StringView& name) : name(name) {
        //
      }
      
      ~IdentifierLiteralExpr() = default;
      
      String to_string() const override;
      bool is_valid_assignment_target() const override;
    };
    
    struct AnonymousFunctionLiteralExpr : public Expr {
      
      String to_string() const override;
      bool is_valid_assignment_target() const override;
    };
    
    struct ElseExpr : public Expr {
      BlockStmt else_block;
      BoxedExpr else_expression;
      
      String to_string() const override;
    };
    
    struct IfExpr : public Expr {
      BoxedExpr condition;
      BlockStmt if_block;
      BoxedExpr if_expression;
      
      Optional<Vector<IfExpr>> else_if_blocks;
      Optional<ElseExpr> else_expression;
      
      String to_string() const override;
    };
    
    struct FunctionCallExpr : public Expr {
      StringView name;
      Vector<BoxedExpr> input_arguments;
      
      FunctionCallExpr() = default;
      ~FunctionCallExpr() = default;
      
      FunctionCallExpr(const StringView& name, Vector<BoxedExpr>&& args) :
      name(name), input_arguments(std::move(args)) {
        //
      }
      
      String to_string() const override;
    };
    
    struct AnonymousFunctionCallExpr : public Expr {
      BoxedExpr function;
      Vector<BoxedExpr> input_arguments;
      
      String to_string() const override;
    };
    
    struct ContentsReferenceExpr : public Expr {
      BoxedExpr target_expression;
      BoxedExpr reference_expression;
      TokenType operator_token;
      
      ContentsReferenceExpr() = default;
      ~ContentsReferenceExpr() = default;
      
      String to_string() const override;
      bool is_valid_assignment_target() const override;
    };
    
    struct AssignmentExpr : public Expr {
      BoxedExpr target_expression;
      BoxedExpr assignment_expression;
      
      String to_string() const override;
    };
  }
}
