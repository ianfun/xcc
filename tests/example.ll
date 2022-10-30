; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@debug = common dso_local global i64 0
@assembly = common dso_local global i64 0
@token = common dso_local global i64 0
@text = common dso_local global ptr null
@stack = common dso_local global ptr null
@old_text = common dso_local global ptr null
@data = common dso_local global ptr null
@idmain = common dso_local global ptr null
@src = common dso_local global ptr null
@old_src = common dso_local global ptr null
@poolsize = common dso_local global i64 0
@pc = common dso_local global ptr null
@bp = common dso_local global ptr null
@sp = common dso_local global ptr null
@ax = common dso_local global i64 0
@cycle = common dso_local global i64 0
@current_id = common dso_local global ptr null
@symbols = common dso_local global ptr null
@line = common dso_local global i64 0
@token_val = common dso_local global i64 0
@basetype = common dso_local global i64 0
@expr_type = common dso_local global i64 0
@index_of_bp = common dso_local global i64 0
@0 = private unnamed_addr constant [101 x i8] c"char else enum if int return sizeof while open read close printf malloc memset memcmp exit void main\00", align 1

; Function Attrs: nounwind optsize
declare dso_local i32 @open(ptr, i32, ...) #0

; Function Attrs: nounwind optsize
declare dso_local i32 @close(i32) #0

; Function Attrs: nounwind optsize
declare dso_local i64 @read(i32, ptr, i64) #0

; Function Attrs: nounwind optsize
declare dso_local i32 @printf(ptr, ...) #0

; Function Attrs: nounwind optsize
declare dso_local ptr @malloc(i64) #0

; Function Attrs: nounwind optsize
declare dso_local void @free(ptr) #0

; Function Attrs: nounwind optsize
declare dso_local ptr @memset(ptr, i32, i64) #0

; Function Attrs: nounwind optsize
declare dso_local ptr @memcpy(ptr, ptr, i64) #0

; Function Attrs: nounwind optsize
declare dso_local i32 @memcmp(ptr, ptr, i64) #0

; Function Attrs: nounwind optsize
declare dso_local void @exit(i32) #0

; Function Attrs: nounwind optsize
define dso_local void @next(...) #0 {
  %1 = alloca ptr, align 8
  %2 = alloca i64, align 8
  br label %4

3:                                                ; preds = %4
  br label %4

4:                                                ; preds = %3, %0
  %5 = load ptr, ptr @src, align 8
  %6 = load i8, ptr %5, align 1
  %7 = sext i8 %6 to i64
  store i64 %7, ptr @token, align 8
  %8 = icmp ne i64 %7, 0
  br i1 %8, label %3, label %9

9:                                                ; preds = %4
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @match(i64 %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = load i64, ptr @token, align 8
  %4 = load i64, ptr %2, align 8
  %5 = icmp eq i64 %3, %4
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %8, label %9

8:                                                ; preds = %1
  br label %10

9:                                                ; preds = %1
  br label %10

10:                                               ; preds = %9, %8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @expression(i64 %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @statement(...) #0 {
  %1 = alloca ptr, align 8
  %2 = alloca ptr, align 8
  %3 = load i64, ptr @token, align 8
  %4 = icmp eq i64 %3, 137
  %5 = zext i1 %4 to i32
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %7, label %8

7:                                                ; preds = %0
  br label %37

8:                                                ; preds = %0
  %9 = load i64, ptr @token, align 8
  %10 = icmp eq i64 %9, 141
  %11 = zext i1 %10 to i32
  %12 = icmp ne i32 %11, 0
  br i1 %12, label %13, label %14

13:                                               ; preds = %8
  br label %36

14:                                               ; preds = %8
  %15 = load i64, ptr @token, align 8
  %16 = icmp eq i64 %15, 123
  %17 = zext i1 %16 to i32
  %18 = icmp ne i32 %17, 0
  br i1 %18, label %19, label %20

19:                                               ; preds = %14
  br label %35

20:                                               ; preds = %14
  %21 = load i64, ptr @token, align 8
  %22 = icmp eq i64 %21, 139
  %23 = zext i1 %22 to i32
  %24 = icmp ne i32 %23, 0
  br i1 %24, label %25, label %26

25:                                               ; preds = %20
  br label %34

26:                                               ; preds = %20
  %27 = load i64, ptr @token, align 8
  %28 = icmp eq i64 %27, 59
  %29 = zext i1 %28 to i32
  %30 = icmp ne i32 %29, 0
  br i1 %30, label %31, label %32

31:                                               ; preds = %26
  br label %33

32:                                               ; preds = %26
  br label %33

33:                                               ; preds = %32, %31
  br label %34

34:                                               ; preds = %33, %25
  br label %35

35:                                               ; preds = %34, %19
  br label %36

36:                                               ; preds = %35, %13
  br label %37

37:                                               ; preds = %36, %7
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @enum_declaration(...) #0 {
  %1 = alloca i64, align 8
  store i64 0, ptr %1, align 8
  br label %3

2:                                                ; preds = %3
  br label %3

3:                                                ; preds = %2, %0
  %4 = load i64, ptr @token, align 8
  %5 = icmp ne i64 %4, 125
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %2, label %8

8:                                                ; preds = %3
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @function_parameter(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  store i64 0, ptr %2, align 8
  br label %4

3:                                                ; preds = %4
  br label %4

4:                                                ; preds = %3, %0
  %5 = load i64, ptr @token, align 8
  %6 = icmp ne i64 %5, 41
  %7 = zext i1 %6 to i32
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %3, label %9

9:                                                ; preds = %4
  %10 = load i64, ptr %2, align 8
  %11 = add nsw i64 %10, 1
  store i64 %11, ptr @index_of_bp, align 8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @function_body(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = load i64, ptr @index_of_bp, align 8
  store i64 %3, ptr %1, align 8
  br label %5

4:                                                ; preds = %15
  br label %5

5:                                                ; preds = %4, %0
  %6 = load i64, ptr @token, align 8
  %7 = icmp eq i64 %6, 134
  %8 = zext i1 %7 to i32
  %9 = icmp ne i32 %8, 0
  br i1 %9, label %15, label %10

10:                                               ; preds = %5
  %11 = load i64, ptr @token, align 8
  %12 = icmp eq i64 %11, 138
  %13 = zext i1 %12 to i32
  %14 = icmp ne i32 %13, 0
  br label %15

15:                                               ; preds = %10, %5
  %16 = phi i1 [ %9, %5 ], [ %14, %10 ]
  %17 = zext i1 %16 to i32
  %18 = icmp ne i32 %17, 0
  br i1 %18, label %4, label %19

19:                                               ; preds = %15
  %20 = load ptr, ptr @text, align 8
  %21 = getelementptr inbounds i64, ptr %20, i32 1
  store ptr %21, ptr @text, align 8
  store i64 6, ptr %21, align 8
  %22 = load ptr, ptr @text, align 8
  %23 = getelementptr inbounds i64, ptr %22, i32 1
  store ptr %23, ptr @text, align 8
  %24 = load i64, ptr %1, align 8
  %25 = load i64, ptr @index_of_bp, align 8
  %26 = sub nsw i64 %24, %25
  store i64 %26, ptr %23, align 8
  br label %28

27:                                               ; preds = %28
  br label %28

28:                                               ; preds = %27, %19
  %29 = load i64, ptr @token, align 8
  %30 = icmp ne i64 %29, 125
  %31 = zext i1 %30 to i32
  %32 = icmp ne i32 %31, 0
  br i1 %32, label %27, label %33

33:                                               ; preds = %28
  %34 = load ptr, ptr @text, align 8
  %35 = getelementptr inbounds i64, ptr %34, i32 1
  store ptr %35, ptr @text, align 8
  store i64 8, ptr %35, align 8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @function_declaration(...) #0 {
  call void @match(i64 40)
  call void (...) @function_parameter()
  call void @match(i64 41)
  call void @match(i64 123)
  call void (...) @function_body()
  %1 = load ptr, ptr @symbols, align 8
  store ptr %1, ptr @current_id, align 8
  br label %3

2:                                                ; preds = %3
  br label %3

3:                                                ; preds = %2, %0
  %4 = load ptr, ptr @current_id, align 8
  %5 = getelementptr inbounds i64, ptr %4, i32 0
  %6 = load i64, ptr %5, align 8
  %7 = icmp ne i64 %6, 0
  br i1 %7, label %2, label %8

8:                                                ; preds = %3
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @global_declaration(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  store i64 1, ptr @basetype, align 8
  %3 = load i64, ptr @token, align 8
  %4 = icmp eq i64 %3, 136
  %5 = zext i1 %4 to i32
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %7, label %8

7:                                                ; preds = %0
  br label %8

8:                                                ; preds = %7, %0
  %9 = load i64, ptr @token, align 8
  %10 = icmp eq i64 %9, 138
  %11 = zext i1 %10 to i32
  %12 = icmp ne i32 %11, 0
  br i1 %12, label %13, label %14

13:                                               ; preds = %8
  br label %21

14:                                               ; preds = %8
  %15 = load i64, ptr @token, align 8
  %16 = icmp eq i64 %15, 134
  %17 = zext i1 %16 to i32
  %18 = icmp ne i32 %17, 0
  br i1 %18, label %19, label %20

19:                                               ; preds = %14
  br label %20

20:                                               ; preds = %19, %14
  br label %21

21:                                               ; preds = %20, %13
  br label %23

22:                                               ; preds = %33
  br label %23

23:                                               ; preds = %22, %21
  %24 = load i64, ptr @token, align 8
  %25 = icmp ne i64 %24, 59
  %26 = zext i1 %25 to i32
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %28, label %33

28:                                               ; preds = %23
  %29 = load i64, ptr @token, align 8
  %30 = icmp ne i64 %29, 125
  %31 = zext i1 %30 to i32
  %32 = icmp ne i32 %31, 0
  br label %33

33:                                               ; preds = %28, %23
  %34 = phi i1 [ %27, %23 ], [ %32, %28 ]
  %35 = zext i1 %34 to i32
  %36 = icmp ne i32 %35, 0
  br i1 %36, label %22, label %37

37:                                               ; preds = %33
  call void (...) @next()
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @program(...) #0 {
  call void (...) @next()
  br label %2

1:                                                ; preds = %2
  br label %2

2:                                                ; preds = %1, %0
  %3 = load i64, ptr @token, align 8
  %4 = icmp sgt i64 %3, 0
  %5 = zext i1 %4 to i32
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %1, label %7

7:                                                ; preds = %2
  ret void
}

; Function Attrs: nounwind optsize
define dso_local i64 @eval(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca ptr, align 8
  store i64 0, ptr @cycle, align 8
  br label %4

3:                                                ; preds = %4
  br label %4

4:                                                ; preds = %3, %0
  br i1 true, label %3, label %5

5:                                                ; preds = %4
  ret i64 undef
}

; Function Attrs: nounwind optsize
define dso_local i32 @main(i32 %0, ptr %1) #0 {
  %3 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  %4 = alloca ptr, align 8
  store ptr %1, ptr %4, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = load i32, ptr %3, align 4
  %9 = sub i32 %8, 1
  store i32 %9, ptr %3, align 4
  %10 = load ptr, ptr %4, align 8
  %11 = getelementptr inbounds ptr, ptr %10, i32 1
  store ptr %11, ptr %4, align 8
  %12 = load i32, ptr %3, align 4
  %13 = icmp sgt i32 %12, 0
  %14 = zext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %25, label %33

16:                                               ; preds = %33
  %17 = load ptr, ptr %4, align 8
  %18 = load ptr, ptr %17, align 8
  %19 = getelementptr inbounds i8, ptr %18, i32 1
  %20 = load i8, ptr %19, align 1
  %21 = sext i8 %20 to i32
  %22 = icmp eq i32 %21, 115
  %23 = zext i1 %22 to i32
  %24 = icmp ne i32 %23, 0
  br label %37

25:                                               ; preds = %2
  %26 = load ptr, ptr %4, align 8
  %27 = load ptr, ptr %26, align 8
  %28 = load i8, ptr %27, align 1
  %29 = sext i8 %28 to i32
  %30 = icmp eq i32 %29, 45
  %31 = zext i1 %30 to i32
  %32 = icmp ne i32 %31, 0
  br label %33

33:                                               ; preds = %25, %2
  %34 = phi i1 [ %15, %2 ], [ %32, %25 ]
  %35 = zext i1 %34 to i32
  %36 = icmp ne i32 %35, 0
  br i1 %36, label %16, label %37

37:                                               ; preds = %16, %33
  %38 = phi i1 [ %36, %33 ], [ %24, %16 ]
  %39 = zext i1 %38 to i32
  %40 = icmp ne i32 %39, 0
  br i1 %40, label %41, label %42

41:                                               ; preds = %37
  br label %42

42:                                               ; preds = %41, %37
  %43 = load i32, ptr %3, align 4
  %44 = icmp sgt i32 %43, 0
  %45 = zext i1 %44 to i32
  %46 = icmp ne i32 %45, 0
  br i1 %46, label %56, label %64

47:                                               ; preds = %64
  %48 = load ptr, ptr %4, align 8
  %49 = load ptr, ptr %48, align 8
  %50 = getelementptr inbounds i8, ptr %49, i32 1
  %51 = load i8, ptr %50, align 1
  %52 = sext i8 %51 to i32
  %53 = icmp eq i32 %52, 100
  %54 = zext i1 %53 to i32
  %55 = icmp ne i32 %54, 0
  br label %68

56:                                               ; preds = %42
  %57 = load ptr, ptr %4, align 8
  %58 = load ptr, ptr %57, align 8
  %59 = load i8, ptr %58, align 1
  %60 = sext i8 %59 to i32
  %61 = icmp eq i32 %60, 45
  %62 = zext i1 %61 to i32
  %63 = icmp ne i32 %62, 0
  br label %64

64:                                               ; preds = %56, %42
  %65 = phi i1 [ %46, %42 ], [ %63, %56 ]
  %66 = zext i1 %65 to i32
  %67 = icmp ne i32 %66, 0
  br i1 %67, label %47, label %68

68:                                               ; preds = %47, %64
  %69 = phi i1 [ %67, %64 ], [ %55, %47 ]
  %70 = zext i1 %69 to i32
  %71 = icmp ne i32 %70, 0
  br i1 %71, label %72, label %73

72:                                               ; preds = %68
  br label %73

73:                                               ; preds = %72, %68
  %74 = load i32, ptr %3, align 4
  %75 = icmp slt i32 %74, 1
  %76 = zext i1 %75 to i32
  %77 = icmp ne i32 %76, 0
  br i1 %77, label %78, label %79

78:                                               ; preds = %73
  br label %79

79:                                               ; preds = %78, %73
  %80 = load ptr, ptr %4, align 8
  %81 = load ptr, ptr %80, align 8
  %82 = call i32 (ptr, i32, ...) @open(ptr %81, i32 0)
  %83 = sext i32 %82 to i64
  store i64 %83, ptr %6, align 8
  %84 = icmp slt i64 %83, 0
  %85 = zext i1 %84 to i32
  %86 = icmp ne i32 %85, 0
  br i1 %86, label %87, label %88

87:                                               ; preds = %79
  br label %88

88:                                               ; preds = %87, %79
  store i64 262144, ptr @poolsize, align 8
  store i64 1, ptr @line, align 8
  %89 = load i64, ptr @poolsize, align 8
  %90 = call ptr @malloc(i64 %89)
  store ptr %90, ptr @text, align 8
  %91 = icmp eq ptr %90, null
  %92 = zext i1 %91 to i32
  %93 = icmp ne i32 %92, 0
  br i1 %93, label %94, label %95

94:                                               ; preds = %88
  br label %95

95:                                               ; preds = %94, %88
  %96 = load i64, ptr @poolsize, align 8
  %97 = call ptr @malloc(i64 %96)
  store ptr %97, ptr @data, align 8
  %98 = icmp eq ptr %97, null
  %99 = zext i1 %98 to i32
  %100 = icmp ne i32 %99, 0
  br i1 %100, label %101, label %102

101:                                              ; preds = %95
  br label %102

102:                                              ; preds = %101, %95
  %103 = load i64, ptr @poolsize, align 8
  %104 = call ptr @malloc(i64 %103)
  store ptr %104, ptr @stack, align 8
  %105 = icmp eq ptr %104, null
  %106 = zext i1 %105 to i32
  %107 = icmp ne i32 %106, 0
  br i1 %107, label %108, label %109

108:                                              ; preds = %102
  br label %109

109:                                              ; preds = %108, %102
  %110 = load i64, ptr @poolsize, align 8
  %111 = call ptr @malloc(i64 %110)
  store ptr %111, ptr @symbols, align 8
  %112 = icmp eq ptr %111, null
  %113 = zext i1 %112 to i32
  %114 = icmp ne i32 %113, 0
  br i1 %114, label %115, label %116

115:                                              ; preds = %109
  br label %116

116:                                              ; preds = %115, %109
  %117 = load ptr, ptr @text, align 8
  %118 = load i64, ptr @poolsize, align 8
  %119 = call ptr @memset(ptr %117, i32 0, i64 %118)
  %120 = load ptr, ptr @data, align 8
  %121 = load i64, ptr @poolsize, align 8
  %122 = call ptr @memset(ptr %120, i32 0, i64 %121)
  %123 = load ptr, ptr @stack, align 8
  %124 = load i64, ptr @poolsize, align 8
  %125 = call ptr @memset(ptr %123, i32 0, i64 %124)
  %126 = load ptr, ptr @symbols, align 8
  %127 = load i64, ptr @poolsize, align 8
  %128 = call ptr @memset(ptr %126, i32 0, i64 %127)
  %129 = load ptr, ptr @text, align 8
  store ptr %129, ptr @old_text, align 8
  store ptr @0, ptr @src, align 8
  store i64 134, ptr %5, align 8
  br label %131

130:                                              ; preds = %131
  br label %131

131:                                              ; preds = %130, %116
  %132 = load i64, ptr %5, align 8
  %133 = icmp sle i64 %132, 141
  %134 = zext i1 %133 to i32
  %135 = icmp ne i32 %134, 0
  br i1 %135, label %130, label %136

136:                                              ; preds = %131
  store i64 30, ptr %5, align 8
  br label %138

137:                                              ; preds = %138
  br label %138

138:                                              ; preds = %137, %136
  %139 = load i64, ptr %5, align 8
  %140 = icmp sle i64 %139, 37
  %141 = zext i1 %140 to i32
  %142 = icmp ne i32 %141, 0
  br i1 %142, label %137, label %143

143:                                              ; preds = %138
  call void (...) @next()
  %144 = load ptr, ptr @current_id, align 8
  %145 = getelementptr inbounds i64, ptr %144, i32 0
  store i64 134, ptr %145, align 8
  call void (...) @next()
  %146 = load ptr, ptr @current_id, align 8
  store ptr %146, ptr @idmain, align 8
  %147 = load i64, ptr @poolsize, align 8
  %148 = call ptr @malloc(i64 %147)
  store ptr %148, ptr @old_src, align 8
  store ptr %148, ptr @src, align 8
  %149 = icmp eq ptr %148, null
  %150 = zext i1 %149 to i32
  %151 = icmp ne i32 %150, 0
  br i1 %151, label %152, label %153

152:                                              ; preds = %143
  br label %153

153:                                              ; preds = %152, %143
  %154 = load i64, ptr %6, align 8
  %155 = trunc i64 %154 to i32
  %156 = load ptr, ptr @src, align 8
  %157 = load i64, ptr @poolsize, align 8
  %158 = sub nsw i64 %157, 1
  %159 = call i64 @read(i32 %155, ptr %156, i64 %158)
  store i64 %159, ptr %5, align 8
  %160 = icmp sle i64 %159, 0
  %161 = zext i1 %160 to i32
  %162 = icmp ne i32 %161, 0
  br i1 %162, label %163, label %164

163:                                              ; preds = %153
  br label %164

164:                                              ; preds = %163, %153
  %165 = load ptr, ptr @src, align 8
  %166 = load i64, ptr %5, align 8
  %167 = getelementptr inbounds i8, ptr %165, i64 %166
  store i8 0, ptr %167, align 1
  %168 = load i64, ptr %6, align 8
  %169 = trunc i64 %168 to i32
  %170 = call i32 @close(i32 %169)
  call void (...) @program()
  %171 = load ptr, ptr @idmain, align 8
  %172 = getelementptr inbounds i64, ptr %171, i32 5
  %173 = load i64, ptr %172, align 8
  %174 = inttoptr i64 %173 to ptr
  store ptr %174, ptr @pc, align 8
  %175 = icmp eq ptr %174, null
  %176 = zext i1 %175 to i32
  %177 = icmp ne i32 %176, 0
  br i1 %177, label %178, label %179

178:                                              ; preds = %164
  br label %179

179:                                              ; preds = %178, %164
  %180 = load i64, ptr @assembly, align 8
  %181 = icmp ne i64 %180, 0
  br i1 %181, label %182, label %183

182:                                              ; preds = %179
  br label %183

183:                                              ; preds = %182, %179
  %184 = load ptr, ptr @stack, align 8
  %185 = ptrtoint ptr %184 to i64
  %186 = load i64, ptr @poolsize, align 8
  %187 = add nsw i64 %185, %186
  %188 = inttoptr i64 %187 to ptr
  store ptr %188, ptr @sp, align 8
  %189 = load ptr, ptr @sp, align 8
  %190 = getelementptr inbounds i64, ptr %189, i32 -1
  store ptr %190, ptr @sp, align 8
  store i64 37, ptr %190, align 8
  %191 = load ptr, ptr @sp, align 8
  %192 = getelementptr inbounds i64, ptr %191, i32 -1
  store ptr %192, ptr @sp, align 8
  store i64 13, ptr %192, align 8
  %193 = load ptr, ptr @sp, align 8
  store ptr %193, ptr %7, align 8
  %194 = load ptr, ptr @sp, align 8
  %195 = getelementptr inbounds i64, ptr %194, i32 -1
  store ptr %195, ptr @sp, align 8
  %196 = load i32, ptr %3, align 4
  %197 = sext i32 %196 to i64
  store i64 %197, ptr %195, align 8
  %198 = load ptr, ptr @sp, align 8
  %199 = getelementptr inbounds i64, ptr %198, i32 -1
  store ptr %199, ptr @sp, align 8
  %200 = load ptr, ptr %4, align 8
  %201 = ptrtoint ptr %200 to i64
  store i64 %201, ptr %199, align 8
  %202 = load ptr, ptr @sp, align 8
  %203 = getelementptr inbounds i64, ptr %202, i32 -1
  store ptr %203, ptr @sp, align 8
  %204 = load ptr, ptr %7, align 8
  %205 = ptrtoint ptr %204 to i64
  store i64 %205, ptr %203, align 8
  %206 = call i64 (...) @eval()
  %207 = trunc i64 %206 to i32
  ret i32 %207
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"cc version alpha (https://github.com/ianfun/cc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
