# Grammar


program: (**_stmt_**)*

---

stmt: **_fn_stmt_**, **_if_stmt_**, **_let_stmt_**, **_return_stmt_**, **_struct_stmt_**, **_assign_stmt_**

fn_stmt: `fn` **_identifier_** (**_generic_**)? `(` (**_maybe_typed_identifier_**)* `)` `->` **_maybe_typed_identifier_** **_block_stmt_**

if_stmt: `if` **_expr_** **_block_stmt_** (`else` **_block_stmt_**)?

block_stmt: `{` (**_stmt_**)* `}`

let_stmt: `let` **_identifier_** (**_type_provision_**)? `=` **_expr_**

assign_stmt: **_identifier_** `=` **_expr_**

return_stmt: `return` **_expr_**

struct_stmt: `struct` **_identifier_** (**_generic_**)? `{` (**_struct_member_**)* `}`

---

struct_member: **_identifier_** `:` **_maybe_typed_identifier_**

type_provision: `:` **_maybe_typed_identifier_**

generic: `<` **_maybe_typed_identifier_** `>`

maybe_typed_identifier: **_identifier_** (`<` (**_maybe_typed_identifier_**)+ `>`)?

---

expr: **_logical_or_**

logical_or: **_logical_and_** (`||` **_logical_and_**)*

logical_and: **_equality_** (`&&` **_equality_**)*

equality: **_relational_** ((`==`, `!=`) **_relational_**)*

relational: **_additive_** ((`<`, `>`, `<=`, `>=`) **_additive_**)*

additive: **_multiplicative_** ((`+`, `-`) **_multiplicative_**)*

multiplicative: **_function_call_** ((`**`, `/`) **_function_call_**)*

function_call: **_grouping_**, (**_identifier_**)? `(` (**_expr_**)* `)`

grouping: **_unary_**, **_terminal_**, `(` **_expr_** `)`

unary: (`!`, `-`) **_expr_**

terminal: **_identifier_**, **_literal_**