// this is needed for forward declration of Lexer
// Parser requires Lexer, but Lexer requires Parser's constant_expression method, so we defined here

Expr Lexer::constant_expression() { return parser->constant_expression(); }

const char Lexer::months[12][4] = {
    "Jan", // January
    "Feb", // February
    "Mar", // March
    "Apr", // April
    "May", // May
    "Jun", // June
    "Jul", // July
    "Aug", // August
    "Sep", // September
    "Oct", // October
    "Nov", // November
    "Dec"  // December
};
