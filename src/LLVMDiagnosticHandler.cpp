struct XCCDiagnosticHandler : public llvm::DiagnosticHandler {
    bool handleDiagnostics(const llvm::DiagnosticInfo &DI) override {
        auto &OS = llvm::errs();
        if (DI.getKind() == llvm::DK_SrcMgr) {
            const auto &DISM = llvm::cast<llvm::DiagnosticInfoSrcMgr>(DI);
            const llvm::SMDiagnostic &SMD = DISM.getSMDiag();
            SMD.print(nullptr, OS);
            return true;
        } else {
            switch (DI.getSeverity()) {
            case llvm::DS_Error: llvm::WithColor::error(OS); break;
            case llvm::DS_Warning: llvm::WithColor::warning(OS); break;
            case llvm::DS_Remark: break;
            case llvm::DS_Note: llvm::WithColor::note(OS); break;
            }
            llvm::DiagnosticPrinterRawOStream DP(OS);
            OS << llvm::LLVMContext::getDiagnosticMessagePrefix(DI.getSeverity()) << ": ";
            DI.print(DP);
            OS << "\n";
            return true;
        }
    }
};
