# Grammar

`program`: `expr` | `stmt`

`stmt`: `fn_stmt`

`fn_stmt`:

`expr`: `logical_or`
`logical_or`: `logical_and` ("||" `logical_and`)*
`logical_and`: `equality` ("&&" `equality`)*
`equality`: `relational` (("==" | "!=") `relational`)*
`relational`: `additive` (("<" | ">" | "<=" | ">=") `additive`)*
`additive`: `multiplicative` (("+" | "-") `multiplicative`)*
`multiplicative`:   `grouping` (("**" | "/") `grouping`)*
`grouping`: `unary` | (`expr`)
`unary`: ("!" | "-") `expr`
