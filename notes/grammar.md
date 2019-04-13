# Grammar


_program_: **_expr_**, (**_stmt_**)*

---

_stmt_: **_fn_stmt_**, **_if_stmt_**, **_let_stmt_**, **_return_stmt_**, **_struct_stmt_**, **_assign_stmt_**

_fn_stmt_: 'fn' **_identifier_** (**_generic_**)? '(' (**_maybe_typed_identifier_**)* ')' '->' **_maybe_typed_identifier_** **_block_stmt_**

_if_stmt_: 'if' **_expr_** **_block_stmt_** ('else' **_block_stmt_**)?

_block_stmt_: '{' (**_stmt_**)* '}'

_let_stmt_: 'let' **_identifier_** (**_type_provision_**)? '=' **_expr_**

_assign_stmt_: **_identifier_** '=' **_expr_**

_return_stmt_: 'return' **_expr_**

_struct_stmt_: 'struct' **_identifier_** (**_generic_**)? '{' (**_struct_member_**)* '}'

---

_struct_member_: **_identifier_** ':' **_maybe_typed_identifier_**

_type_provision_: ':' **_identifier_**

_generic_: '<' **_maybe_typed_identifier_**'>'

_maybe_typed_identifier_: **_identifier_** ('<' (**_maybe_typed_identifier_**)+ '>')?

---

_expr_: **_logical_or_**

_logical_or_: **_logical_and_** ('||' **_logical_and_**)*

_logical_and_: **_equality_** ('&&' **_equality_**)*

_equality_: **_relational_** (('==', '!=') **_relational_**)*

_relational_: **_additive_** (('<', '>', '<=', '>=') **_additive_**)*

_additive_: **_multiplicative_** (('+', '-') **_multiplicative_**)*

_multiplicative_: **_grouping_** (('*', '/') **_grouping_**)*

_grouping_: **_unary_**, (**_expr_**)

_unary_: ('!', '-') **_term_**, **_expr_**

_term_: **_identifier_**, **_literal_**