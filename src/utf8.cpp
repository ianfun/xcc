// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
namespace enc {
// clang-format off

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

uint32_t inline
decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state*16 + type];
  return *state;
}

static bool 
IsUTF8(StringRef str) {
  uint32_t codepoint, state = 0;

  for (size_t i = 0;i < str.size();i++)
    decode(&state, &codepoint, (uint64_t)(unsigned char)str[i]);

  return state == UTF8_ACCEPT;
}

// clang-format on

llvm::Constant *getUTF8(const StringRef &s, LLVMContext &ctx) {
    if (s.empty()) {
        auto Ty = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 1);
        return llvm::ConstantAggregateZero::get(Ty);
    }
    return llvm::ConstantDataArray::getString(ctx, s, false);
}

template <typename T> llvm::Constant *getUTF16AsNBit(const StringRef &s, LLVMContext &ctx) {
    if (s.empty()) {
        auto Ty = llvm::ArrayType::get(llvm::Type::getIntNTy(ctx, sizeof(T)), 1);
        return llvm::ConstantAggregateZero::get(Ty);
    }
    SmallVector<T> data;
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
    return llvm::ConstantDataArray::get(ctx, data);
}
// in Linux, wchar_t are 32 bit(int)
llvm::Constant *getUTF16As32Bit(const StringRef &s, LLVMContext &ctx) { return getUTF16AsNBit<uint32_t>(s, ctx); }
// in Windows, wchar_t are 16 bit(unsigned short)
llvm::Constant *getUTF16As16Bit(const StringRef &s, LLVMContext &ctx) { return getUTF16AsNBit<uint16_t>(s, ctx); }
llvm::Constant *getUTF32(const StringRef &s, LLVMContext &ctx) {
    if (s.empty()) {
        auto Ty = llvm::ArrayType::get(llvm::Type::getInt32Ty(ctx), 1);
        return llvm::ConstantAggregateZero::get(Ty);
    }
    uint32_t state = 0, codepoint;
    llvm::SmallVector<uint32_t> data;
    for (const auto c : s)
        if (!decode(&state, &codepoint, (uint32_t)(unsigned char)c))
            data.push_back(codepoint);
    return llvm::ConstantDataArray::get(ctx, data);
}

} // namespace enc
