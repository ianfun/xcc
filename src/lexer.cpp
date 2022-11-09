// this is needed for forward declration of Lexer
// Parser requires Lexer, but Lexer requires Parser's constant_expression method, so we defined here

Expr Lexer::constant_expression() { return parser.constant_expression(); }
