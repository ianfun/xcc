; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-w64-windows-gnu"

; Function Attrs: nounwind optsize
define dso_local i32 @main(...) #0 {
  br label %1

1:                                                ; preds = %1, %0
  br label %1

2:                                                ; No predecessors!
  br label %3

3:                                                ; preds = %3, %2
  br label %3

4:                                                ; No predecessors!
  ret i32 0
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"cc version alpha (https://github.com/ianfun/cc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
