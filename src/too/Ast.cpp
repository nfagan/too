//
//  Ast.cpp
//  too
//
//  Created by Nick Fagan on 4/14/19.
//

#include "Ast.hpp"
#include "Token.hpp"
#include "ast/PrettyStringVisitor.hpp"

#define BEGIN_NAMESPACE namespace too {
#define END_NAMESPACE }

BEGIN_NAMESPACE

String ast::Identifier::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.identifier(*this);
}

String ast::TypeParameter::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.type_parameter(*this);
}

String ast::TraitBoundedType::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.trait_bounded_type(*this);
}

String ast::WhereClause::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.where_clause(*this);
}

String ast::DefinitionHeader::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.definition_header(*this);
}

String ast::FunctionDefinition::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.function_definition(*this);
}

String ast::StructDefinition::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.struct_definition(*this);
}

String ast::TraitDefinition::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.trait_definition(*this);
}

String ast::DefinitionContext::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.definition_context(*this);
}

String ast::UsingDeclaration::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.using_declaration(*this);
}

String ast::LetStmt::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.let_statement(*this);
}

String ast::BlockStmt::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.block_statement(*this);
}

String ast::ExprStmt::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.expression_statement(*this);
}

String ast::ReturnStmt::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.return_statement(*this);
}

String ast::UnaryExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.unary_expression(*this);
}

String ast::BinaryExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.binary_expression(*this);
}

String ast::IntLiteralExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.int_literal_expression(*this);
}

String ast::FloatLiteralExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.float_literal_expression(*this);
}

String ast::StringLiteralExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.string_literal_expression(*this);
}

String ast::IdentifierLiteralExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.identifier_literal_expression(*this);
}

bool ast::IdentifierLiteralExpr::is_valid_assignment_target() const {
  return true;
}

String ast::AnonymousFunctionLiteralExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.anonymous_function_literal_expression(*this);
}

bool ast::AnonymousFunctionLiteralExpr::is_valid_assignment_target() const {
  return false;
}

String ast::FunctionCallExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.function_call_expression(*this);
}

String ast::AnonymousFunctionCallExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.anonymous_function_call_expression(*this);
}

String ast::ContentsReferenceExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.contents_reference_expression(*this);
}

bool ast::ContentsReferenceExpr::is_valid_assignment_target() const {
  return true;
}

String ast::AssignmentExpr::accept(const too::ast::PrettyStringVisitor& visitor) const {
  return visitor.assignment_expression(*this);
}

END_NAMESPACE

#undef BEGIN_NAMESPACE
#undef END_NAMESPACE
