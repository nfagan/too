# Grammar

_program_: _expr_ | _stmt_

_stmt_: _fn_stmt_

_fn_stmt_:

_expr_: _logical_or_
_logical_or_: _logical_and_ ("||" _logical_and_)*
_logical_and_: _equality_ ("&&" _equality_)*
_equality_: _relational_ (("==" | "!=") _relational_)*
_relational_: _additive_ (("<" | ">" | "<=" | ">=") _additive)_*
_additive_: _multiplicative_ (("+" | "-") _multiplicative_)*
_multiplicative_: _grouping_ (("**" | "/") _grouping_)*
_grouping_: _unary_ | (_expr_)
_unary_: ("!" | "-") _expr_