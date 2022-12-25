/* IdentifierTable.h - symbol cache

   see clang/Basic/IdentifierTable.h

*/

typedef llvm::StringMapEntry<Token> *IdentRef;

struct IdentifierTable {
    static constexpr unsigned InitialSize = 8192;
    llvm::StringMap<Token, ArenaAllocator> HashTable;

    IdentifierTable() : HashTable{InitialSize} {
        HashTable.insert({
#include "IdentifierTableInit.inc"
        });
    }
    ArenaAllocator &getAllocator() { return HashTable.getAllocator(); }
    IdentRef get(StringRef Name, Token val = PPIdent) {
        return &*HashTable.insert(std::make_pair(Name, val)).first;
    }
    ~IdentifierTable() {
        statics("IdentifierTable statics\n");
        statics("  - Number of buckets: %u\n", HashTable.getNumBuckets());
        statics("  - Number of items: %u\n", HashTable.getNumItems());
        endStatics();
    }
};
