int next();
int global_declaration();
int token;

void program() {
    // get next token
    next();
    while (token > 0) {
        global_declaration();
    }
}
