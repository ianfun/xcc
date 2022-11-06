/*

this file is a similar implementation of
 clang/Basic/IdentifierTable.h.

we use unsigned to store a token(including keyword enums and some constant enums), or builtin-id

the IdentifierTable is like a string pool, or llvm::UniqueStringSaver

*/

struct IdentifierInfo {
    static constexpr Token tok_high = PPpragma, tok_low = TNul;
    unsigned tok;

    IdentifierInfo() : tok{TNul} { }
    IdentifierInfo(unsigned tok) : tok{tok} { }
    IdentifierInfo(Token tok) : tok{tok} { }
    Token getToken() { return static_cast<Token>(tok); }
    void setToken(Token tok) { this->tok = static_cast<Token>(tok); }
    unsigned getBuiltinID() { return tok - static_cast<Token>(tok_high); }
    void setBuiltiID(unsigned ID) { tok = static_cast<Token>(tok_high) + ID; }
    bool isBuiltinMacro() { return tok >= PP__LINE__ && tok <= PP_Pragma; }
};

typedef llvm::StringMapEntry<IdentifierInfo> *IdentRef;

struct IdentifierTable {
    static constexpr unsigned InitialSize = 8192;
    llvm::StringMap<IdentifierInfo, ArenaAllocator> HashTable;

    IdentifierTable() : HashTable{InitialSize} {
        HashTable.insert({
#include "IdentifierTableInit.inc"
        });
    }
    ArenaAllocator &getAllocator() { return HashTable.getAllocator(); }
    IdentRef get(StringRef Name, unsigned val = PPIdent) {
        return &*HashTable.insert(std::make_pair(Name, IdentifierInfo(val))).first;
    }
};
