struct Options {
    StringRef mainFileName;
    llvm::Triple triple;
    SmallString<256> CWD;
    const llvm::Target *theTarget;
    bool g : 1;
    bool trigraphs : 1;
    Options() { llvm::sys::fs::current_path(CWD); }
};
