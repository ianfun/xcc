; ModuleID = 'bitfield.c'
source_filename = "bitfield.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.anon = type { i8 }
%struct.anon.0 = type { i16 }
%struct.anon.1 = type { i32 }
%struct.anon.2 = type { i64 }

@A = dso_local global %struct.anon zeroinitializer, align 1
@B = dso_local global %struct.anon.0 zeroinitializer, align 2
@C = dso_local global %struct.anon.1 zeroinitializer, align 4
@D = dso_local global %struct.anon.2 zeroinitializer, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i8 @get_A() #0 {
  %1 = load i8, ptr @A, align 1
  %2 = lshr i8 %1, 1
  ret i8 %2
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i16 @get_B() #0 {
  %1 = load i16, ptr @B, align 2
  %2 = lshr i16 %1, 1
  %3 = and i16 %2, 1023
  ret i16 %3
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @get_C() #0 {
  %1 = load i32, ptr @C, align 4
  %2 = lshr i32 %1, 1
  %3 = and i32 %2, 1073741823
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @get_D() #0 {
  %1 = load i64, ptr @D, align 8
  %2 = lshr i64 %1, 1
  %3 = and i64 %2, 1152921504606846975
  ret i64 %3
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @set_A(i8 noundef zeroext %0) #0 {
  %2 = alloca i8, align 1
  store i8 %0, ptr %2, align 1
  %3 = load i8, ptr %2, align 1
  %4 = load i8, ptr @A, align 1
  %5 = and i8 %3, 127
  %6 = shl i8 %5, 1
  %7 = and i8 %4, 1
  %8 = or i8 %7, %6
  store i8 %8, ptr @A, align 1
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @set_B(i16 noundef zeroext %0) #0 {
  %2 = alloca i16, align 2
  store i16 %0, ptr %2, align 2
  %3 = load i16, ptr %2, align 2
  %4 = load i16, ptr @B, align 2
  %5 = and i16 %3, 1023
  %6 = shl i16 %5, 1
  %7 = and i16 %4, -2047
  %8 = or i16 %7, %6
  store i16 %8, ptr @B, align 2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @set_C(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = load i32, ptr @C, align 4
  %5 = and i32 %3, 1073741823
  %6 = shl i32 %5, 1
  %7 = and i32 %4, -2147483647
  %8 = or i32 %7, %6
  store i32 %8, ptr @C, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @set_D(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = load i64, ptr %2, align 8
  %4 = load i64, ptr @D, align 8
  %5 = and i64 %3, 1152921504606846975
  %6 = shl i64 %5, 1
  %7 = and i64 %4, -2305843009213693951
  %8 = or i64 %7, %6
  store i64 %8, ptr @D, align 8
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 15.0.4-++20221102053248+5c68a1cb1231-1~exp1~20221102053256.89"}
