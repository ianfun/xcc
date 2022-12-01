; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@a = dso_local constant { double, double } { double 2.000000e+00, double 4.000000e+00 }, align 8
@b = dso_local constant { double, double } { double 1.000000e+00, double 2.000000e+00 }, align 8
@toBool1 = dso_local global i1 true, align 1
@toBool2 = dso_local global i1 true, align 1
@toBool3 = dso_local global i1 true, align 1
@toBool4 = dso_local global i1 true, align 1
@toBool5 = dso_local global i1 false, align 1
@toBool6 = dso_local global i1 false, align 1
@A = dso_local global { double, double } { double 3.000000e+00, double 6.000000e+00 }, align 8
@B = dso_local global { double, double } { double 1.000000e+00, double 2.000000e+00 }, align 8
@C = dso_local global { double, double } { double -6.000000e+00, double 8.000000e+00 }, align 8
@D = dso_local global { double, double } { double 2.000000e+00, double 0.000000e+00 }, align 8
@eq = dso_local global i1 false, align 1
@ne = dso_local global i1 true, align 1
@toComplexInt = dso_local global { i32, i32 } { i32 2, i32 4 }, align 8
@toComplexUnsigned = dso_local global { i32, i32 } { i32 1, i32 2 }, align 8
@toReal = dso_local global double 2.000000e+00, align 8
@toInt = dso_local global i32 1, align 4
@toComplexFloat = dso_local global { float, float } { float 2.000000e+00, float 4.000000e+00 }, align 8
@toComplexLongDouble = dso_local global { double, double } { double 1.000000e+00, double 2.000000e+00 }, align 8
@unaryNeg = dso_local global { double, double } { double -2.000000e+00, double -4.000000e+00 }, align 8
@unaryConj = dso_local global { double, double } { double 1.000000e+00, double -2.000000e+00 }, align 8
@poison_pair = dso_local global { i32, i32 } zeroinitializer, align 8
@neg_nan_pair = dso_local global { double, double } { double -0.000000e+00, double 0xFFF8000000000000 }, align 8

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"XCC version alpha (https://github.com/ianfun/xcc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
