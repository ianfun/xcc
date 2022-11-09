struct Options {
    std::string mainFileName;
    llvm::Triple triple;
    SmallString<256> CWD;
    const llvm::Target *theTarget;
    bool g: 1;
    Options() { llvm::sys::fs::current_path(CWD); }
};
