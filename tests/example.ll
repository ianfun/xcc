; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone
define dso_local ptr @xxx(...) #0 {
  ret ptr inttoptr (i32 1234 to ptr)
}

attributes #0 = { noinline nounwind optnone }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"XCC version alpha (https://github.com/ianfun/xcc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
