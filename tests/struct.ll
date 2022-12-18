; ModuleID = 'struct.c'
source_filename = "struct.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Empty = type {}
%struct.Single = type { i32 }
%struct.Pair = type { i32, i32 }
%struct.Outer = type { i32, %struct.anon, i32 }
%struct.anon = type { i32, %struct.anon.0, i32 }
%struct.anon.0 = type { i32, [50 x %struct.anon.1] }
%struct.anon.1 = type { i32, i32 }

@a = dso_local global %struct.Empty undef, align 1
@b = dso_local global %struct.Single { i32 1 }, align 4
@c = dso_local global %struct.Single { i32 1 }, align 4
@d = dso_local global %struct.Pair { i32 1, i32 2 }, align 4
@e = dso_local global %struct.Pair { i32 1, i32 2 }, align 4
@f = dso_local global %struct.Pair { i32 1, i32 2 }, align 4
@g = dso_local global %struct.Outer zeroinitializer, align 4
@h = dso_local global { i32, { i32, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> }, i32 }, i32 } { i32 1, { i32, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> }, i32 } { i32 2, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> } { i32 3, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> <{ [31 x %struct.anon.1] [%struct.anon.1 { i32 4, i32 0 }, %struct.anon.1 { i32 5, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }], [19 x %struct.anon.1] zeroinitializer }> }, i32 8 }, i32 9 }, align 4
@__const.test.b = private unnamed_addr constant %struct.Single { i32 1 }, align 4
@__const.test.c = private unnamed_addr constant %struct.Single { i32 1 }, align 4
@__const.test.d = private unnamed_addr constant %struct.Pair { i32 1, i32 2 }, align 4
@__const.test.e = private unnamed_addr constant %struct.Pair { i32 1, i32 2 }, align 4
@__const.test.f = private unnamed_addr constant %struct.Pair { i32 1, i32 2 }, align 4
@__const.test.h = private unnamed_addr constant { i32, { i32, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> }, i32 }, i32 } { i32 1, { i32, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> }, i32 } { i32 2, { i32, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> } { i32 3, <{ [31 x %struct.anon.1], [19 x %struct.anon.1] }> <{ [31 x %struct.anon.1] [%struct.anon.1 { i32 4, i32 0 }, %struct.anon.1 { i32 5, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 0 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 6, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }, %struct.anon.1 { i32 0, i32 7 }], [19 x %struct.anon.1] zeroinitializer }> }, i32 8 }, i32 9 }, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test() #0 {
  %1 = alloca %struct.Empty, align 1
  %2 = alloca %struct.Single, align 4
  %3 = alloca %struct.Single, align 4
  %4 = alloca %struct.Pair, align 4
  %5 = alloca %struct.Pair, align 4
  %6 = alloca %struct.Pair, align 4
  %7 = alloca %struct.Outer, align 4
  %8 = alloca %struct.Outer, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.test.b, i64 4, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %3, ptr align 4 @__const.test.c, i64 4, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %4, ptr align 4 @__const.test.d, i64 8, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %5, ptr align 4 @__const.test.e, i64 8, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %6, ptr align 4 @__const.test.f, i64 8, i1 false)
  call void @llvm.memset.p0.i64(ptr align 4 %7, i8 0, i64 420, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %8, ptr align 4 @__const.test.h, i64 420, i1 false)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nocallback nofree nounwind willreturn }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn writeonly }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 15.0.4-++20221102053248+5c68a1cb1231-1~exp1~20221102053256.89"}
