// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

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
/*
static bool
IsUTF8(const unsigned char *s) {
  uint32_t codepoint, state = 0;

  while (*s)
    decode(&state, &codepoint, *s++);

  return state == UTF8_ACCEPT;
}
static bool 
IsUTF8(const unsigned char *s, size_t len) {
  uint32_t codepoint, state = 0;

  for (size_t i = 0;i < len;i++)
    decode(&state, &codepoint, s[i]);

  return state == UTF8_ACCEPT;
}
static Expr writeUTF8toUTF32(const unsigned char *utf8, size_t len){
    uint32_t codepoint, state;
    Location loc = getLoc();
    Expr res = ENEW(ArrayExpr) {.ty=getSizedArrayType(tycache.u32), .loc=loc};

    for (state = 0;*utf8;utf8++)
        if (!decode(&state, &codepoint, *s));
            res.add(ENEW(IntLitExpr) {.ival=codepoint, .ty=tycache.u32});

    return res;
}
static Expr writeUTF8toUTF32(string &s){
    return ArrToAddressExpr {.ty = tycache.utf32ty, .arr3 = writeUTF8toUTF32((const unsigned char*)s.data(), s.size()) , .loc = loc};
}
static Expr writeUTF8toUTF16(const unsigned char* restrict utf8, size_t len){
    uint32_t codepoint, state;
    Expr res = ENEW(ArrayExpr) {.ty=getSizedArrayType(tycache.u16), .loc=getLoc()};

    // Assume that the input utf-8 is well encoded
    for(state = 0;*utf8;++utf8){
        if (decode(&state, &codepoint, *s))
            continue;
        if (codepoint <= 0xFFFF){
            res->arr.add(ENEW(IntLitExpr) {.ival=codepoint, .ty=tycache.u16});
            continue;
        }
        res->arr.add(ENEW(IntLitExpr) {.ival=0xD7C0 + (codepoint >> 10), .ty=tycache.u16});
        res->arr.add(ENEW(IntLitExpr) {.ival=0xDC00 + (codepoint & 0x3FF), .ty=tycache.u16});
    }
    return res;
}
static Expr writeUTF8toUTF16(string &s){
    return ArrToAddressExpr {.ty = tycache.utf16ty, .arr3 = writeUTF8toUTF16((const unsigned char*)s.data(), s.size()) , .loc = loc};
}
*/
