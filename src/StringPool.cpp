// StringPool - this file implements a Java'JVM like string pool.
//
// C's strings are always constant, and to get the address to the string, we offen use a global private object to store
// the string data instead of stack allocated.

struct StringPool {
    using GV = llvm::GlobalVariable *;
    const IRGen &irgen;
    DenseMap<StringRef, GV> interns;
    DenseMap<ArrayRef<uint16_t>, GV> interns16;
    DenseMap<ArrayRef<uint32_t>, GV> interns32;
    StringPool(const IRGen &ig) : irgen{ig} { }
    GV getAsUTF8(xstring &s) {
        auto it = interns.insert(std::make_pair(s.str(), nullptr));
        if (it.second) {
            auto str = enc::getUTF8(s, irgen.ctx);
            auto GV =
                new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.integer_types[3]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;
    }
    GV getAsUTF16(xstring s, bool is32Bit) {
        if (is32Bit) {
            SmallVector<uint32_t> data;
            uint32_t state = 0, codepoint;
            for (auto c : s) {
                if (enc::decode(&state, &codepoint, (uint32_t)(unsigned char)c))
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
            auto it = interns32.insert(std::make_pair(array, nullptr));
            if (it.second) {
                auto str = llvm::ConstantDataArray::get(irgen.ctx, array);
                auto GV =
                    new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
                GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
                GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.integer_types[5]));
                GV->setConstant(true);
                it.first->second = GV;
            }
            return it.first->second;
        }
        SmallVector<uint16_t> data;
        uint32_t state = 0, codepoint;
        for (auto c : s) {
            if (enc::decode(&state, &codepoint, (uint32_t)(unsigned char)c))
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
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.integer_types[4]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;  
    }
    GV getAsUTF32(xstring s) {
        SmallVector<uint32_t> data;
        uint32_t state = 0, codepoint;
        for (const auto c : s)
            if (!enc::decode(&state, &codepoint, (uint32_t)(unsigned char)c))
                data.push_back(codepoint);
        data.push_back(0);
        auto array = makeArrayRef(data);
        auto it = interns32.insert(std::make_pair(array, nullptr));
        if (it.second) {
            auto str = llvm::ConstantDataArray::get(irgen.ctx, array);
            auto GV =
                new llvm::GlobalVariable(*irgen.module, str->getType(), true, IRGen::PrivateLinkage, str, ".cstr");
            GV->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            GV->setAlignment(irgen.layout->getPrefTypeAlign(irgen.integer_types[5]));
            GV->setConstant(true);
            it.first->second = GV;
        }
        return it.first->second;
    }
};
