// StringPool - this file implements a Java'JVM like string pool.
//
// C's strings are always constant, and to get the address to the string, we offen use a global private object to store
// the string data instead of stack allocated.

struct StringPool {
    using GV = llvm::GlobalVariable *;
    const IRGen &irgen;
    DenseMap<StringRef, GV> interns;
#if CC_WCHAR32
    DenseMap<ArrayRef<uint16_t>, GV> interns16;
#else
    DenseMap<ArrayRef<uint32_t>, GV> interns16;
#endif
    DenseMap<ArrayRef<uint32_t>, GV> interns32;
    StringPool(const IRGen &ig) : irgen{ig} { }
    GV getAsUTF8(xstring &s) {
        s.make_eos();
        auto it = interns.insert(std::make_pair(s.str(), nullptr));
        if (it.second) {
            auto str = llvm::ConstantDataArray::getString(irgen.ctx, s.str(), false);
            auto GV =
                new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.types[x8]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;
    }
    GV getAsUTF16(xstring s) {
#if CC_WCHAR32
        SmallVector<uint16_t> data;
#else
        SmallVector<uint32_t> data;
#endif
        uint32_t state = 0, codepoint;
        for (auto c : s) {
            if (decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                continue;
            if (codepoint <= 0xFFFF) {
                data.push_back(codepoint);
                continue;
            }
            data.push_back(0xD7C0 + (codepoint >> 10));
            data.push_back(0xDC00 + (codepoint & 0x3FF));
        }
        data.push_back(0);
        auto array = makeArrayRef(data);
        auto it = interns16.insert(std::make_pair(array, nullptr));
        if (it.second) {
            auto str = llvm::ConstantDataArray::get(irgen.ctx, array);
            auto GV =
                new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.types[x16]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;
    }
    GV getAsUTF32(xstring s) {
        SmallVector<uint32_t> data;
        uint32_t state = 0, codepoint;
        for (const auto c : s)
            if (!decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                data.push_back(codepoint);
        data.push_back(0);
        auto array = makeArrayRef(data);
        auto it = interns32.insert(std::make_pair(array, nullptr));
        if (it.second) {
            auto str = llvm::ConstantDataArray::get(irgen.ctx, array);
            auto GV =
                new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.types[x32]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;
    }
};
