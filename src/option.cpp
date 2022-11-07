struct Options {
    std::string mainFileName;
    llvm::Triple triple;
    SmallString<256> CWD;
    bool g;
    Options() { llvm::sys::fs::current_path(CWD); }
};
