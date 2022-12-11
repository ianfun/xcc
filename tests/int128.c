__int128 a = 10 + 20;
unsigned __int128 b = 0xFFFFFFFFFFFFFFFFF;

__int128 int128_add(__int128 a, __int128 b) {
	return a + b;
}
__int128 int128_add(__int128 a, __int128 b) {
        return a + b;
}
__int128 int128_sub(__int128 a, __int128 b) {
        return a - b;
}
__int128 int128_mul(__int128 a, __int128 b) {
        return a * b;
}
__int128 int128_sdiv(__int128 a, __int128 b) {
        return a / b;
}
unsigned int128_udiv(unsigned __int128 a, unsigned __int128 b) {
	return a / b;
}
__int128 int128_neg(__int128 a) {
	return -a;
}
__int128 int128_not(__int128 a) {
        return !a;
}
__int128 int128_bitwise_not(__int128 a) {
        return ~a;
}
__int128 int128_or(__int128 a, __int128 b) {
        return a | b;
}
__int128 int128_and(__int128 a, __int128 b) {
        return a & b;
}
__int128 int128_xor(__int128 a, __int128 b) {
        return a ^ b;
}
void int128_switch(__int128 a, int *res) {
	switch (a) {
		case 0: *res = 1; break;
		case 1: *res = 2; break;
		case 2: *res = 3; break;
		default: *res = 0;
	}
}
_Bool int128_to_bool(__int128 a) {
	return a;
}
_Bool int128_eq(__int128 a, __int128 b) {
        return a == b;
}
_Bool int128_ne(__int128 a, __int128 b) {
        return a != b;
}
_Bool int128_gt(__int128 a, __int128 b) {
        return a > b;
}
_Bool int128_ge(__int128 a, __int128 b) {
        return a >= b;
}
_Bool int128_lt(__int128 a, __int128 b) {
        return a < b;
}
_Bool int128_le(__int128 a, __int128 b) {
        return a <= b;
}
_Bool uint128_gt(unsigned __int128 a, unsigned __int128 b) {
        return a > b;
}
_Bool uint128_ge(unsigned __int128 a, unsigned __int128 b) {
        return a >= b;
}
_Bool uint128_lt(unsigned __int128 a, unsigned __int128 b) {
        return a < b;
}
_Bool uint128_le(unsigned __int128 a, unsigned __int128 b) {
        return a <= b;
}
double int128_to_double(__int128 a) {
	return a;
}
double uint128_to_double(unsigned __int128 a) {
        return a;
}
__int128 int128_from_double(double a) {
	return a;
}
unsigned __int128 uint128_from_double(double a) {
        return a;
}
_Complex __int128 int128_complex_add(_Complex __int128 a, _Complex __int128 b) {
	return a + b;
}
_Complex __int128 int128_complex_sub(_Complex __int128 a, _Complex __int128 b) {
        return a - b;
}
_Complex __int128 int128_complex_mul(_Complex __int128 a, _Complex __int128 b) {
        return a * b;
}
_Complex __int128 int128_complex_div(_Complex __int128 a, _Complex __int128 b) {
        return a / b;
}

// expected output

/*
define dso_local i128 @int128_add(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = add nsw i128 %1, %0
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_sub(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = sub nsw i128 %0, %1
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_mul(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = mul nsw i128 %1, %0
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_sdiv(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = sdiv i128 %0, %1
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i32 @int128_udiv(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = udiv i128 %0, %1
  %4 = trunc i128 %3 to i32
  ret i32 %4
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_neg(i128 %0) local_unnamed_addr #0 {
  %2 = sub nsw i128 0, %0
  ret i128 %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_not(i128 %0) local_unnamed_addr #0 {
  %2 = icmp eq i128 %0, 0
  %3 = zext i1 %2 to i128
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_bitwise_not(i128 %0) local_unnamed_addr #0 {
  %2 = xor i128 %0, -1
  ret i128 %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_or(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = or i128 %1, %0
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_and(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = and i128 %1, %0
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_xor(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = xor i128 %1, %0
  ret i128 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write)
define dso_local void @int128_switch(i128 %0, ptr nocapture writeonly %1) local_unnamed_addr #1 {
  %3 = icmp ult i128 %0, 3
  %switch.idx.cast = trunc i128 %0 to i32
  %switch.offset = add i32 %switch.idx.cast, 1
  %.sink = select i1 %3, i32 %switch.offset, i32 0
  store i32 %.sink, ptr %1, align 4
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_to_bool(i128 %0) local_unnamed_addr #0 {
  %2 = icmp ne i128 %0, 0
  ret i1 %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_eq(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp eq i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_ne(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp ne i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_gt(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp sgt i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_ge(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp sge i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_lt(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp slt i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @int128_le(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp sle i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @uint128_gt(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp ugt i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @uint128_ge(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp uge i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @uint128_lt(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp ult i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i1 @uint128_le(i128 %0, i128 %1) local_unnamed_addr #0 {
  %3 = icmp ule i128 %0, %1
  ret i1 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local double @int128_to_double(i128 %0) local_unnamed_addr #0 {
  %2 = sitofp i128 %0 to double
  ret double %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local double @uint128_to_double(i128 %0) local_unnamed_addr #0 {
  %2 = uitofp i128 %0 to double
  ret double %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @int128_from_double(double %0) local_unnamed_addr #0 {
  %2 = fptosi double %0 to i128
  ret i128 %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local i128 @uint128_from_double(double %0) local_unnamed_addr #0 {
  %2 = fptoui double %0 to i128
  ret i128 %2
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local { i128, i128 } @int128_complex_add({ i128, i128 } %0, { i128, i128 } %1) local_unnamed_addr #0 {
  %.fca.0.extract3 = extractvalue { i128, i128 } %0, 0
  %.fca.1.extract4 = extractvalue { i128, i128 } %0, 1
  %.fca.0.extract = extractvalue { i128, i128 } %1, 0
  %.fca.1.extract = extractvalue { i128, i128 } %1, 1
  %3 = add i128 %.fca.0.extract, %.fca.0.extract3
  %4 = add i128 %.fca.1.extract, %.fca.1.extract4
  %5 = insertvalue { i128, i128 } poison, i128 %3, 0
  %6 = insertvalue { i128, i128 } %5, i128 %4, 1
  ret { i128, i128 } %6
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local { i128, i128 } @int128_complex_sub({ i128, i128 } %0, { i128, i128 } %1) local_unnamed_addr #0 {
  %.fca.0.extract3 = extractvalue { i128, i128 } %0, 0
  %.fca.1.extract4 = extractvalue { i128, i128 } %0, 1
  %.fca.0.extract = extractvalue { i128, i128 } %1, 0
  %.fca.1.extract = extractvalue { i128, i128 } %1, 1
  %3 = sub i128 %.fca.0.extract3, %.fca.0.extract
  %4 = sub i128 %.fca.1.extract4, %.fca.1.extract
  %5 = insertvalue { i128, i128 } poison, i128 %3, 0
  %6 = insertvalue { i128, i128 } %5, i128 %4, 1
  ret { i128, i128 } %6
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local { i128, i128 } @int128_complex_mul({ i128, i128 } %0, { i128, i128 } %1) local_unnamed_addr #0 {
  %.fca.0.extract3 = extractvalue { i128, i128 } %0, 0
  %.fca.1.extract4 = extractvalue { i128, i128 } %0, 1
  %.fca.0.extract = extractvalue { i128, i128 } %1, 0
  %.fca.1.extract = extractvalue { i128, i128 } %1, 1
  %3 = mul i128 %.fca.0.extract, %.fca.0.extract3
  %4 = mul i128 %.fca.1.extract, %.fca.1.extract4
  %5 = sub i128 %3, %4
  %6 = mul i128 %.fca.1.extract, %.fca.0.extract3
  %7 = mul i128 %.fca.0.extract, %.fca.1.extract4
  %8 = sub i128 %6, %7
  %9 = insertvalue { i128, i128 } poison, i128 %5, 0
  %10 = insertvalue { i128, i128 } %9, i128 %8, 1
  ret { i128, i128 } %10
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(none)
define dso_local { i128, i128 } @int128_complex_div({ i128, i128 } %0, { i128, i128 } %1) local_unnamed_addr #0 {
  %.fca.0.extract3 = extractvalue { i128, i128 } %0, 0
  %.fca.1.extract4 = extractvalue { i128, i128 } %0, 1
  %.fca.0.extract = extractvalue { i128, i128 } %1, 0
  %.fca.1.extract = extractvalue { i128, i128 } %1, 1
  %3 = mul i128 %.fca.0.extract, %.fca.0.extract
  %4 = mul i128 %.fca.1.extract, %.fca.1.extract
  %5 = add i128 %3, %4
  %6 = add i128 %.fca.0.extract, %.fca.1.extract4
  %7 = add i128 %6, %.fca.0.extract3
  %8 = add i128 %7, %.fca.1.extract
  %9 = sdiv i128 %8, %5
  %10 = add i128 %.fca.0.extract3, %.fca.1.extract
  %11 = sub i128 %6, %10
  %12 = sdiv i128 %11, %5
  %13 = insertvalue { i128, i128 } poison, i128 %9, 0
  %14 = insertvalue { i128, i128 } %13, i128 %12, 1
  ret { i128, i128 } %14
}
*/

