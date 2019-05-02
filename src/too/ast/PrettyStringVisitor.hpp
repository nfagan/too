//
//  PrettyStringVisitor.hpp
//  too
//
//  Created by Nick Fagan on 5/1/19.
//

#pragma once

#include "../Ast.hpp"
#include "../String.hpp"

namespace too {
  namespace ast {
    class PrettyStringVisitor;
  }
}

class too::ast::PrettyStringVisitor {
public:
  String let_statement(const LetStmt& let_stmt) const;
  String expression_statement(const ExprStmt& expr_stmt) const;
  String block_statement(const BlockStmt& block_stmt) const;
  String return_statement(const ReturnStmt& return_stmt) const;
  
  String unary_expression(const UnaryExpr& unary_expr) const;
  String binary_expression(const BinaryExpr& binary_expr) const;
  String int_literal_expression(const IntLiteralExpr& int_literal_expr) const;
  String float_literal_expression(const FloatLiteralExpr& float_literal_expr) const;
  String string_literal_expression(const StringLiteralExpr& string_literal_expr) const;
  String identifier_literal_expression(const IdentifierLiteralExpr& identifier_literal_expr) const;
  String anonymous_function_literal_expression(const AnonymousFunctionLiteralExpr& anon_func_literal_expr) const;
  String function_call_expression(const FunctionCallExpr& func_call_expr) const;
  String anonymous_function_call_expression(const AnonymousFunctionCallExpr& anon_func_call_expr) const;
  String contents_reference_expression(const ContentsReferenceExpr& contents_ref_expr) const;
  String assignment_expression(const AssignmentExpr& assignment_expr) const;
  
  String type_parameter(const TypeParameter& type_param) const;
  String identifier(const Identifier& identifier) const;
  String trait_bounded_type(const TraitBoundedType& type) const;
  String where_clause(const WhereClause& clause) const;
  String definition_header(const DefinitionHeader& header) const;
  String definition_context(const DefinitionContext& context) const;
  String function_definition(const FunctionDefinition& func) const;
  String struct_definition(const StructDefinition& struct_def) const;
  String trait_definition(const TraitDefinition& trait_def) const;
  
  String using_declaration(const UsingDeclaration& decl) const;
};
