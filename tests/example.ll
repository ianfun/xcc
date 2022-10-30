; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@0 = private unnamed_addr constant [14 x i8] c"Hello world!\0A\00", align 1

; Function Attrs: nounwind optsize
declare dso_local i32 @printf(ptr, ...) #0

; Function Attrs: nounwind optsize
declare dso_local void @exit(i32) #0

; Function Attrs: nounwind optsize
declare dso_local i32 @getchar(...) #0

; Function Attrs: nounwind optsize
define dso_local i32 @main(...) #0 {
  %1 = call i32 (...) @getchar()
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %3, label %5

3:                                                ; preds = %0
  %4 = call i32 (ptr, ...) @printf(ptr @0)
  br label %6

5:                                                ; preds = %0
  call void @exit(i32 1)
  br label %6

6:                                                ; preds = %5, %3
  ret i32 0
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"cc version alpha (https://github.com/ianfun/cc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
