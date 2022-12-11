; ModuleID = 'vla/basic.c'
source_filename = "vla/basic.c"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-w64-windows-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @vla(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store i32 %0, ptr %2, align 4
  %5 = load i32, ptr %2, align 4
  %6 = zext i32 %5 to i64
  %7 = call ptr @llvm.stacksave()
  store ptr %7, ptr %3, align 8
  %8 = alloca i32, i64 %6, align 16
  store i64 %6, ptr %4, align 8
  %9 = getelementptr inbounds i32, ptr %8, i64 1
  %10 = load ptr, ptr %3, align 8
  call void @llvm.stackrestore(ptr %10)
  ret ptr %9
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @vla2(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store i32 %0, ptr %2, align 4
  %5 = load i32, ptr %2, align 4
  %6 = zext i32 %5 to i64
  %7 = call ptr @llvm.stacksave()
  store ptr %7, ptr %3, align 8
  %8 = alloca i32, i64 %6, align 16
  store i64 %6, ptr %4, align 8
  %9 = mul nsw i64 1, %6
  %10 = getelementptr inbounds i32, ptr %8, i64 %9
  %11 = load ptr, ptr %3, align 8
  call void @llvm.stackrestore(ptr %11)
  ret ptr %10
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 15.0.3"}
