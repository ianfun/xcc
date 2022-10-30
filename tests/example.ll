; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@token = common dso_local global i32 0

; Function Attrs: nounwind optsize
declare dso_local i32 @next(...) #0

; Function Attrs: nounwind optsize
declare dso_local i32 @global_declaration(...) #0

; Function Attrs: nounwind optsize
define dso_local void @program(...) #0 {
  %1 = call i32 (...) @next()
  br label %3

2:                                                ; preds = %3
  br label %3

3:                                                ; preds = %2, %0
  %4 = load i32, ptr @token, align 4
  %5 = icmp sgt i32 %4, 0
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %2, label %8

8:                                                ; preds = %3
  ret void
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"cc version alpha (https://github.com/ianfun/cc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
