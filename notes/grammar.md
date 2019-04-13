# Grammar


_program_: _expr_ , (_stmt_)*

---

_stmt_: _fn_stmt_, _if_stmt_, _let_stmt_

_fn_stmt_: 'fn' _identifier_ '(' _identifier_ ')' '->' _identifier_ _block_stmt_

_if_stmt_: 'if' _expr_ _block_stmt_ ('else' _block_stmt_)?

_block_stmt_: '{' (_stmt_)* '}'

_let_stmt_: 'let' _identifier_ '=' _expr_

---

_expr_: _logical_or_

_logical_or_: _logical_and_ ('||' _logical_and_)*

_logical_and_: _equality_ ('&&' _equality_)*

_equality_: _relational_ (('==', '!=') _relational_)*

_relational_: _additive_ (('<', '>', '<=', '>=') _additive)_*

_additive_: _multiplicative_ (('+', '-') _multiplicative_)*

_multiplicative_: _grouping_ (('**', '/') _grouping_)*

_grouping_: _unary_, (_expr_)

_unary_: ('!', '-') _expr_
