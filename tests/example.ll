; ModuleID = 'main'
source_filename = "main"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.cstr = private unnamed_addr constant [9 x i8] c"%d: %.*s\00", align 1
@.cstr.1 = private unnamed_addr constant [6 x i8] c"%8.4s\00", align 1
@.cstr.2 = private unnamed_addr constant [190 x i8] c"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT\00", align 1
@.cstr.3 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1
@.cstr.4 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.cstr.5 = private unnamed_addr constant [24 x i8] c"%d: expected token: %d\0A\00", align 1
@.cstr.6 = private unnamed_addr constant [40 x i8] c"%d: unexpected token EOF of expression\0A\00", align 1
@.cstr.7 = private unnamed_addr constant [23 x i8] c"%d: bad function call\0A\00", align 1
@.cstr.8 = private unnamed_addr constant [24 x i8] c"%d: undefined variable\0A\00", align 1
@.cstr.9 = private unnamed_addr constant [21 x i8] c"%d: bad dereference\0A\00", align 1
@.cstr.10 = private unnamed_addr constant [20 x i8] c"%d: bad address of\0A\00", align 1
@.cstr.11 = private unnamed_addr constant [33 x i8] c"%d: bad lvalue of pre-increment\0A\00", align 1
@.cstr.12 = private unnamed_addr constant [20 x i8] c"%d: bad expression\0A\00", align 1
@.cstr.13 = private unnamed_addr constant [30 x i8] c"%d: bad lvalue in assignment\0A\00", align 1
@.cstr.14 = private unnamed_addr constant [34 x i8] c"%d: missing colon in conditional\0A\00", align 1
@.cstr.15 = private unnamed_addr constant [28 x i8] c"%d: bad value in increment\0A\00", align 1
@.cstr.16 = private unnamed_addr constant [27 x i8] c"%d: pointer type expected\0A\00", align 1
@.cstr.17 = private unnamed_addr constant [32 x i8] c"%d: compiler error, token = %d\0A\00", align 1
@.cstr.18 = private unnamed_addr constant [28 x i8] c"%d: bad enum identifier %d\0A\00", align 1
@.cstr.19 = private unnamed_addr constant [26 x i8] c"%d: bad enum initializer\0A\00", align 1
@.cstr.20 = private unnamed_addr constant [31 x i8] c"%d: bad parameter declaration\0A\00", align 1
@.cstr.21 = private unnamed_addr constant [37 x i8] c"%d: duplicate parameter declaration\0A\00", align 1
@.cstr.22 = private unnamed_addr constant [27 x i8] c"%d: bad local declaration\0A\00", align 1
@.cstr.23 = private unnamed_addr constant [33 x i8] c"%d: duplicate local declaration\0A\00", align 1
@.cstr.24 = private unnamed_addr constant [28 x i8] c"%d: bad global declaration\0A\00", align 1
@.cstr.25 = private unnamed_addr constant [34 x i8] c"%d: duplicate global declaration\0A\00", align 1
@.cstr.26 = private unnamed_addr constant [9 x i8] c"%d> %.4s\00", align 1
@.cstr.27 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1
@.cstr.28 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@.cstr.29 = private unnamed_addr constant [9 x i8] c"exit(%d)\00", align 1
@.cstr.30 = private unnamed_addr constant [24 x i8] c"unknown instruction:%d\0A\00", align 1
@.cstr.31 = private unnamed_addr constant [30 x i8] c"usage: xc [-s] [-d] file ...\0A\00", align 1
@.cstr.32 = private unnamed_addr constant [20 x i8] c"could not open(%s)\0A\00", align 1
@.cstr.33 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for text area\0A\00", align 1
@.cstr.34 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for data area\0A\00", align 1
@.cstr.35 = private unnamed_addr constant [37 x i8] c"could not malloc(%d) for stack area\0A\00", align 1
@.cstr.36 = private unnamed_addr constant [39 x i8] c"could not malloc(%d) for symbol table\0A\00", align 1
@.cstr.37 = private unnamed_addr constant [101 x i8] c"char else enum if int return sizeof while open read close printf malloc memset memcmp exit void main\00", align 1
@.cstr.38 = private unnamed_addr constant [38 x i8] c"could not malloc(%d) for source area\0A\00", align 1
@.cstr.39 = private unnamed_addr constant [20 x i8] c"read() returned %d\0A\00", align 1
@.cstr.40 = private unnamed_addr constant [20 x i8] c"main() not defined\0A\00", align 1
@debug = common dso_local global i64 0, align 8
@assembly = common dso_local global i64 0, align 8
@token = common dso_local global i64 0, align 8
@text = common dso_local global ptr null, align 8
@stack = common dso_local global ptr null, align 8
@old_text = common dso_local global ptr null, align 8
@data = common dso_local global ptr null, align 8
@idmain = common dso_local global ptr null, align 8
@src = common dso_local global ptr null, align 8
@old_src = common dso_local global ptr null, align 8
@poolsize = common dso_local global i64 0, align 8
@pc = common dso_local global ptr null, align 8
@bp = common dso_local global ptr null, align 8
@sp = common dso_local global ptr null, align 8
@ax = common dso_local global i64 0, align 8
@cycle = common dso_local global i64 0, align 8
@current_id = common dso_local global ptr null, align 8
@symbols = common dso_local global ptr null, align 8
@line = common dso_local global i64 0, align 8
@token_val = common dso_local global i64 0, align 8
@basetype = common dso_local global i64 0, align 8
@expr_type = common dso_local global i64 0, align 8
@index_of_bp = common dso_local global i64 0, align 8

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
  br label %727

3:                                                ; preds = %727
  %4 = load ptr, ptr @src, align 8
  %5 = getelementptr inbounds i8, ptr %4, i64 1
  store ptr %5, ptr @src, align 8
  %6 = load i64, ptr @token, align 8
  %7 = icmp eq i64 %6, 10
  %8 = zext i1 %7 to i32
  %9 = icmp ne i32 %8, 0
  br i1 %9, label %10, label %53

10:                                               ; preds = %3
  %11 = load i64, ptr @assembly, align 8
  %12 = icmp ne i64 %11, 0
  br i1 %12, label %13, label %50

13:                                               ; preds = %10
  %14 = load i64, ptr @line, align 8
  %15 = load ptr, ptr @src, align 8
  %16 = ptrtoint ptr %15 to i64
  %17 = load ptr, ptr @old_src, align 8
  %18 = ptrtoint ptr %17 to i64
  %19 = sub i64 %16, %18
  %20 = load ptr, ptr @old_src, align 8
  %21 = call i32 (ptr, ...) @printf(ptr @.cstr, i64 %14, i64 %19, ptr %20)
  %22 = load ptr, ptr @src, align 8
  store ptr %22, ptr @old_src, align 8
  br label %43

23:                                               ; preds = %43
  %24 = load ptr, ptr @old_text, align 8
  %25 = getelementptr inbounds i64, ptr %24, i64 1
  store ptr %25, ptr @old_text, align 8
  %26 = load i64, ptr %25, align 8
  %27 = mul nsw i64 %26, 5
  %28 = getelementptr inbounds i8, ptr @.cstr.2, i64 %27
  %29 = call i32 (ptr, ...) @printf(ptr @.cstr.1, ptr %28)
  %30 = load ptr, ptr @old_text, align 8
  %31 = load i64, ptr %30, align 8
  %32 = icmp sle i64 %31, 7
  %33 = zext i1 %32 to i32
  %34 = icmp ne i32 %33, 0
  br i1 %34, label %35, label %40

35:                                               ; preds = %23
  %36 = load ptr, ptr @old_text, align 8
  %37 = getelementptr inbounds i64, ptr %36, i64 1
  store ptr %37, ptr @old_text, align 8
  %38 = load i64, ptr %37, align 8
  %39 = call i32 (ptr, ...) @printf(ptr @.cstr.3, i64 %38)
  br label %42

40:                                               ; preds = %23
  %41 = call i32 (ptr, ...) @printf(ptr @.cstr.4)
  br label %42

42:                                               ; preds = %40, %35
  br label %43

43:                                               ; preds = %42, %13
  %44 = load ptr, ptr @old_text, align 8
  %45 = load ptr, ptr @text, align 8
  %46 = icmp ult ptr %44, %45
  %47 = zext i1 %46 to i32
  %48 = icmp ne i32 %47, 0
  br i1 %48, label %23, label %49

49:                                               ; preds = %43
  br label %50

50:                                               ; preds = %49, %10
  %51 = load i64, ptr @line, align 8
  %52 = add nsw i64 %51, 1
  store i64 %52, ptr @line, align 8
  br label %726

53:                                               ; preds = %3
  %54 = load i64, ptr @token, align 8
  %55 = icmp eq i64 %54, 35
  %56 = zext i1 %55 to i32
  %57 = icmp ne i32 %56, 0
  br i1 %57, label %58, label %77

58:                                               ; preds = %53
  br label %62

59:                                               ; preds = %72
  %60 = load ptr, ptr @src, align 8
  %61 = getelementptr inbounds i8, ptr %60, i16 1
  store ptr %61, ptr @src, align 8
  br label %62

62:                                               ; preds = %59, %58
  %63 = load ptr, ptr @src, align 8
  %64 = load i8, ptr %63, align 1
  %65 = sext i8 %64 to i32
  %66 = icmp ne i32 %65, 0
  br i1 %66, label %67, label %72

67:                                               ; preds = %62
  %68 = load ptr, ptr @src, align 8
  %69 = load i8, ptr %68, align 1
  %70 = sext i8 %69 to i32
  %71 = icmp ne i32 %70, 10
  br label %72

72:                                               ; preds = %67, %62
  %73 = phi i1 [ %66, %62 ], [ %71, %67 ]
  %74 = zext i1 %73 to i32
  %75 = icmp ne i32 %74, 0
  br i1 %75, label %59, label %76

76:                                               ; preds = %72
  br label %725

77:                                               ; preds = %53
  %78 = load i64, ptr @token, align 8
  %79 = icmp eq i64 %78, 95
  br i1 %79, label %101, label %80

80:                                               ; preds = %77
  %81 = load i64, ptr @token, align 8
  %82 = icmp sge i64 %81, 65
  br i1 %82, label %83, label %86

83:                                               ; preds = %80
  %84 = load i64, ptr @token, align 8
  %85 = icmp sle i64 %84, 90
  br label %86

86:                                               ; preds = %83, %80
  %87 = phi i1 [ %82, %80 ], [ %85, %83 ]
  %88 = icmp ne i1 %87, false
  br i1 %88, label %98, label %89

89:                                               ; preds = %86
  %90 = load i64, ptr @token, align 8
  %91 = icmp sge i64 %90, 97
  br i1 %91, label %92, label %95

92:                                               ; preds = %89
  %93 = load i64, ptr @token, align 8
  %94 = icmp sle i64 %93, 122
  br label %95

95:                                               ; preds = %92, %89
  %96 = phi i1 [ %91, %89 ], [ %94, %92 ]
  %97 = icmp ne i1 %96, false
  br label %98

98:                                               ; preds = %95, %86
  %99 = phi i1 [ %88, %86 ], [ %97, %95 ]
  %100 = icmp ne i1 %99, false
  br label %101

101:                                              ; preds = %98, %77
  %102 = phi i1 [ %79, %77 ], [ %100, %98 ]
  %103 = zext i1 %102 to i32
  %104 = icmp ne i32 %103, 0
  br i1 %104, label %105, label %221

105:                                              ; preds = %101
  %106 = load ptr, ptr @src, align 8
  %107 = getelementptr inbounds i8, ptr %106, i32 -1
  store ptr %107, ptr %1, align 8
  %108 = load i64, ptr @token, align 8
  store i64 %108, ptr %2, align 8
  br label %119

109:                                              ; preds = %169
  %110 = load i64, ptr %2, align 8
  %111 = mul nsw i64 %110, 147
  %112 = load ptr, ptr @src, align 8
  %113 = load i8, ptr %112, align 1
  %114 = sext i8 %113 to i32
  %115 = sext i32 %114 to i64
  %116 = add nsw i64 %111, %115
  store i64 %116, ptr %2, align 8
  %117 = load ptr, ptr @src, align 8
  %118 = getelementptr inbounds i8, ptr %117, i16 1
  store ptr %118, ptr @src, align 8
  br label %119

119:                                              ; preds = %109, %105
  %120 = load ptr, ptr @src, align 8
  %121 = load i8, ptr %120, align 1
  %122 = sext i8 %121 to i32
  %123 = icmp eq i32 %122, 95
  br i1 %123, label %169, label %124

124:                                              ; preds = %119
  %125 = load ptr, ptr @src, align 8
  %126 = load i8, ptr %125, align 1
  %127 = sext i8 %126 to i32
  %128 = icmp sge i32 %127, 48
  br i1 %128, label %129, label %134

129:                                              ; preds = %124
  %130 = load ptr, ptr @src, align 8
  %131 = load i8, ptr %130, align 1
  %132 = sext i8 %131 to i32
  %133 = icmp sle i32 %132, 57
  br label %134

134:                                              ; preds = %129, %124
  %135 = phi i1 [ %128, %124 ], [ %133, %129 ]
  %136 = icmp ne i1 %135, false
  br i1 %136, label %166, label %137

137:                                              ; preds = %134
  %138 = load ptr, ptr @src, align 8
  %139 = load i8, ptr %138, align 1
  %140 = sext i8 %139 to i32
  %141 = icmp sge i32 %140, 65
  br i1 %141, label %142, label %147

142:                                              ; preds = %137
  %143 = load ptr, ptr @src, align 8
  %144 = load i8, ptr %143, align 1
  %145 = sext i8 %144 to i32
  %146 = icmp sle i32 %145, 90
  br label %147

147:                                              ; preds = %142, %137
  %148 = phi i1 [ %141, %137 ], [ %146, %142 ]
  %149 = icmp ne i1 %148, false
  br i1 %149, label %163, label %150

150:                                              ; preds = %147
  %151 = load ptr, ptr @src, align 8
  %152 = load i8, ptr %151, align 1
  %153 = sext i8 %152 to i32
  %154 = icmp sge i32 %153, 97
  br i1 %154, label %155, label %160

155:                                              ; preds = %150
  %156 = load ptr, ptr @src, align 8
  %157 = load i8, ptr %156, align 1
  %158 = sext i8 %157 to i32
  %159 = icmp sle i32 %158, 122
  br label %160

160:                                              ; preds = %155, %150
  %161 = phi i1 [ %154, %150 ], [ %159, %155 ]
  %162 = icmp ne i1 %161, false
  br label %163

163:                                              ; preds = %160, %147
  %164 = phi i1 [ %149, %147 ], [ %162, %160 ]
  %165 = icmp ne i1 %164, false
  br label %166

166:                                              ; preds = %163, %134
  %167 = phi i1 [ %136, %134 ], [ %165, %163 ]
  %168 = icmp ne i1 %167, false
  br label %169

169:                                              ; preds = %166, %119
  %170 = phi i1 [ %123, %119 ], [ %168, %166 ]
  %171 = zext i1 %170 to i32
  %172 = icmp ne i32 %171, 0
  br i1 %172, label %109, label %173

173:                                              ; preds = %169
  %174 = load ptr, ptr @symbols, align 8
  store ptr %174, ptr @current_id, align 8
  br label %206

175:                                              ; preds = %206
  %176 = load ptr, ptr @current_id, align 8
  %177 = getelementptr inbounds i64, ptr %176, i32 1
  %178 = load i64, ptr %177, align 8
  %179 = load i64, ptr %2, align 8
  %180 = icmp eq i64 %178, %179
  br i1 %180, label %181, label %195

181:                                              ; preds = %175
  %182 = load ptr, ptr @current_id, align 8
  %183 = getelementptr inbounds i64, ptr %182, i32 2
  %184 = load i64, ptr %183, align 8
  %185 = inttoptr i64 %184 to ptr
  %186 = load ptr, ptr %1, align 8
  %187 = load ptr, ptr @src, align 8
  %188 = ptrtoint ptr %187 to i64
  %189 = load ptr, ptr %1, align 8
  %190 = ptrtoint ptr %189 to i64
  %191 = sub i64 %188, %190
  %192 = call i32 @memcmp(ptr %185, ptr %186, i64 %191)
  %193 = icmp eq i32 %192, 0
  %194 = icmp ne i1 %193, false
  br label %195

195:                                              ; preds = %181, %175
  %196 = phi i1 [ %180, %175 ], [ %194, %181 ]
  %197 = zext i1 %196 to i32
  %198 = icmp ne i32 %197, 0
  br i1 %198, label %199, label %203

199:                                              ; preds = %195
  %200 = load ptr, ptr @current_id, align 8
  %201 = getelementptr inbounds i64, ptr %200, i32 0
  %202 = load i64, ptr %201, align 8
  store i64 %202, ptr @token, align 8
  ret void

203:                                              ; preds = %195
  %204 = load ptr, ptr @current_id, align 8
  %205 = getelementptr inbounds i64, ptr %204, i32 9
  store ptr %205, ptr @current_id, align 8
  br label %206

206:                                              ; preds = %203, %173
  %207 = load ptr, ptr @current_id, align 8
  %208 = getelementptr inbounds i64, ptr %207, i32 0
  %209 = load i64, ptr %208, align 8
  %210 = icmp ne i64 %209, 0
  br i1 %210, label %175, label %211

211:                                              ; preds = %206
  %212 = load ptr, ptr @current_id, align 8
  %213 = getelementptr inbounds i64, ptr %212, i32 2
  %214 = load ptr, ptr %1, align 8
  %215 = ptrtoint ptr %214 to i64
  store i64 %215, ptr %213, align 8
  %216 = load ptr, ptr @current_id, align 8
  %217 = getelementptr inbounds i64, ptr %216, i32 1
  %218 = load i64, ptr %2, align 8
  store i64 %218, ptr %217, align 8
  %219 = load ptr, ptr @current_id, align 8
  %220 = getelementptr inbounds i64, ptr %219, i32 0
  store i64 133, ptr %220, align 8
  store i64 133, ptr @token, align 8
  ret void

221:                                              ; preds = %101
  %222 = load i64, ptr @token, align 8
  %223 = icmp sge i64 %222, 48
  br i1 %223, label %224, label %227

224:                                              ; preds = %221
  %225 = load i64, ptr @token, align 8
  %226 = icmp sle i64 %225, 57
  br label %227

227:                                              ; preds = %224, %221
  %228 = phi i1 [ %223, %221 ], [ %226, %224 ]
  %229 = zext i1 %228 to i32
  %230 = icmp ne i32 %229, 0
  br i1 %230, label %231, label %363

231:                                              ; preds = %227
  %232 = load i64, ptr @token, align 8
  %233 = sub nsw i64 %232, 48
  store i64 %233, ptr @token_val, align 8
  %234 = load i64, ptr @token_val, align 8
  %235 = icmp sgt i64 %234, 0
  %236 = zext i1 %235 to i32
  %237 = icmp ne i32 %236, 0
  br i1 %237, label %238, label %264

238:                                              ; preds = %231
  br label %249

239:                                              ; preds = %259
  %240 = load i64, ptr @token_val, align 8
  %241 = mul nsw i64 %240, 10
  %242 = load ptr, ptr @src, align 8
  %243 = getelementptr inbounds i8, ptr %242, i16 1
  store ptr %243, ptr @src, align 8
  %244 = load i8, ptr %242, align 1
  %245 = sext i8 %244 to i32
  %246 = sext i32 %245 to i64
  %247 = add nsw i64 %241, %246
  %248 = sub nsw i64 %247, 48
  store i64 %248, ptr @token_val, align 8
  br label %249

249:                                              ; preds = %239, %238
  %250 = load ptr, ptr @src, align 8
  %251 = load i8, ptr %250, align 1
  %252 = sext i8 %251 to i32
  %253 = icmp sge i32 %252, 48
  br i1 %253, label %254, label %259

254:                                              ; preds = %249
  %255 = load ptr, ptr @src, align 8
  %256 = load i8, ptr %255, align 1
  %257 = sext i8 %256 to i32
  %258 = icmp sle i32 %257, 57
  br label %259

259:                                              ; preds = %254, %249
  %260 = phi i1 [ %253, %249 ], [ %258, %254 ]
  %261 = zext i1 %260 to i32
  %262 = icmp ne i32 %261, 0
  br i1 %262, label %239, label %263

263:                                              ; preds = %259
  br label %362

264:                                              ; preds = %231
  %265 = load ptr, ptr @src, align 8
  %266 = load i8, ptr %265, align 1
  %267 = sext i8 %266 to i32
  %268 = icmp eq i32 %267, 88
  br i1 %268, label %274, label %269

269:                                              ; preds = %264
  %270 = load ptr, ptr @src, align 8
  %271 = load i8, ptr %270, align 1
  %272 = sext i8 %271 to i32
  %273 = icmp eq i32 %272, 120
  br label %274

274:                                              ; preds = %269, %264
  %275 = phi i1 [ %268, %264 ], [ %273, %269 ]
  %276 = zext i1 %275 to i32
  %277 = icmp ne i32 %276, 0
  br i1 %277, label %278, label %335

278:                                              ; preds = %274
  %279 = load ptr, ptr @src, align 8
  %280 = getelementptr inbounds i8, ptr %279, i64 1
  store ptr %280, ptr @src, align 8
  %281 = load i8, ptr %280, align 1
  %282 = sext i8 %281 to i64
  store i64 %282, ptr @token, align 8
  br label %300

283:                                              ; preds = %330
  %284 = load i64, ptr @token_val, align 8
  %285 = mul nsw i64 %284, 16
  %286 = load i64, ptr @token, align 8
  %287 = and i64 %286, 15
  %288 = add nsw i64 %285, %287
  %289 = load i64, ptr @token, align 8
  %290 = icmp sge i64 %289, 65
  %291 = zext i1 %290 to i32
  %292 = icmp ne i32 %291, 0
  %293 = select i1 %292, i32 9, i32 0
  %294 = sext i32 %293 to i64
  %295 = add nsw i64 %288, %294
  store i64 %295, ptr @token_val, align 8
  %296 = load ptr, ptr @src, align 8
  %297 = getelementptr inbounds i8, ptr %296, i64 1
  store ptr %297, ptr @src, align 8
  %298 = load i8, ptr %297, align 1
  %299 = sext i8 %298 to i64
  store i64 %299, ptr @token, align 8
  br label %300

300:                                              ; preds = %283, %278
  %301 = load i64, ptr @token, align 8
  %302 = icmp sge i64 %301, 65
  br i1 %302, label %303, label %306

303:                                              ; preds = %300
  %304 = load i64, ptr @token, align 8
  %305 = icmp sle i64 %304, 70
  br label %306

306:                                              ; preds = %303, %300
  %307 = phi i1 [ %302, %300 ], [ %305, %303 ]
  %308 = icmp ne i1 %307, false
  br i1 %308, label %330, label %309

309:                                              ; preds = %306
  %310 = load i64, ptr @token, align 8
  %311 = icmp sge i64 %310, 97
  br i1 %311, label %312, label %315

312:                                              ; preds = %309
  %313 = load i64, ptr @token, align 8
  %314 = icmp sle i64 %313, 102
  br label %315

315:                                              ; preds = %312, %309
  %316 = phi i1 [ %311, %309 ], [ %314, %312 ]
  %317 = icmp ne i1 %316, false
  br i1 %317, label %327, label %318

318:                                              ; preds = %315
  %319 = load i64, ptr @token, align 8
  %320 = icmp sge i64 %319, 48
  br i1 %320, label %321, label %324

321:                                              ; preds = %318
  %322 = load i64, ptr @token, align 8
  %323 = icmp sle i64 %322, 57
  br label %324

324:                                              ; preds = %321, %318
  %325 = phi i1 [ %320, %318 ], [ %323, %321 ]
  %326 = icmp ne i1 %325, false
  br label %327

327:                                              ; preds = %324, %315
  %328 = phi i1 [ %317, %315 ], [ %326, %324 ]
  %329 = icmp ne i1 %328, false
  br label %330

330:                                              ; preds = %327, %306
  %331 = phi i1 [ %308, %306 ], [ %329, %327 ]
  %332 = zext i1 %331 to i32
  %333 = icmp ne i32 %332, 0
  br i1 %333, label %283, label %334

334:                                              ; preds = %330
  br label %361

335:                                              ; preds = %274
  br label %346

336:                                              ; preds = %356
  %337 = load i64, ptr @token_val, align 8
  %338 = mul nsw i64 %337, 8
  %339 = load ptr, ptr @src, align 8
  %340 = getelementptr inbounds i8, ptr %339, i16 1
  store ptr %340, ptr @src, align 8
  %341 = load i8, ptr %339, align 1
  %342 = sext i8 %341 to i32
  %343 = sext i32 %342 to i64
  %344 = add nsw i64 %338, %343
  %345 = sub nsw i64 %344, 48
  store i64 %345, ptr @token_val, align 8
  br label %346

346:                                              ; preds = %336, %335
  %347 = load ptr, ptr @src, align 8
  %348 = load i8, ptr %347, align 1
  %349 = sext i8 %348 to i32
  %350 = icmp sge i32 %349, 48
  br i1 %350, label %351, label %356

351:                                              ; preds = %346
  %352 = load ptr, ptr @src, align 8
  %353 = load i8, ptr %352, align 1
  %354 = sext i8 %353 to i32
  %355 = icmp sle i32 %354, 55
  br label %356

356:                                              ; preds = %351, %346
  %357 = phi i1 [ %350, %346 ], [ %355, %351 ]
  %358 = zext i1 %357 to i32
  %359 = icmp ne i32 %358, 0
  br i1 %359, label %336, label %360

360:                                              ; preds = %356
  br label %361

361:                                              ; preds = %360, %334
  br label %362

362:                                              ; preds = %361, %263
  store i64 128, ptr @token, align 8
  ret void

363:                                              ; preds = %227
  %364 = load i64, ptr @token, align 8
  %365 = icmp eq i64 %364, 47
  %366 = zext i1 %365 to i32
  %367 = icmp ne i32 %366, 0
  br i1 %367, label %368, label %396

368:                                              ; preds = %363
  %369 = load ptr, ptr @src, align 8
  %370 = load i8, ptr %369, align 1
  %371 = sext i8 %370 to i32
  %372 = icmp eq i32 %371, 47
  %373 = zext i1 %372 to i32
  %374 = icmp ne i32 %373, 0
  br i1 %374, label %375, label %394

375:                                              ; preds = %368
  br label %379

376:                                              ; preds = %389
  %377 = load ptr, ptr @src, align 8
  %378 = getelementptr inbounds i8, ptr %377, i64 1
  store ptr %378, ptr @src, align 8
  br label %379

379:                                              ; preds = %376, %375
  %380 = load ptr, ptr @src, align 8
  %381 = load i8, ptr %380, align 1
  %382 = sext i8 %381 to i32
  %383 = icmp ne i32 %382, 0
  br i1 %383, label %384, label %389

384:                                              ; preds = %379
  %385 = load ptr, ptr @src, align 8
  %386 = load i8, ptr %385, align 1
  %387 = sext i8 %386 to i32
  %388 = icmp ne i32 %387, 10
  br label %389

389:                                              ; preds = %384, %379
  %390 = phi i1 [ %383, %379 ], [ %388, %384 ]
  %391 = zext i1 %390 to i32
  %392 = icmp ne i32 %391, 0
  br i1 %392, label %376, label %393

393:                                              ; preds = %389
  br label %395

394:                                              ; preds = %368
  store i64 160, ptr @token, align 8
  ret void

395:                                              ; preds = %393
  br label %722

396:                                              ; preds = %363
  %397 = load i64, ptr @token, align 8
  %398 = icmp eq i64 %397, 39
  br i1 %398, label %402, label %399

399:                                              ; preds = %396
  %400 = load i64, ptr @token, align 8
  %401 = icmp eq i64 %400, 34
  br label %402

402:                                              ; preds = %399, %396
  %403 = phi i1 [ %398, %396 ], [ %401, %399 ]
  %404 = zext i1 %403 to i32
  %405 = icmp ne i32 %404, 0
  br i1 %405, label %406, label %467

406:                                              ; preds = %402
  %407 = load ptr, ptr @data, align 8
  store ptr %407, ptr %1, align 8
  br label %439

408:                                              ; preds = %451
  %409 = load ptr, ptr @src, align 8
  %410 = getelementptr inbounds i8, ptr %409, i16 1
  store ptr %410, ptr @src, align 8
  %411 = load i8, ptr %409, align 1
  %412 = sext i8 %411 to i64
  store i64 %412, ptr @token_val, align 8
  %413 = load i64, ptr @token_val, align 8
  %414 = icmp eq i64 %413, 92
  %415 = zext i1 %414 to i32
  %416 = icmp ne i32 %415, 0
  br i1 %416, label %417, label %428

417:                                              ; preds = %408
  %418 = load ptr, ptr @src, align 8
  %419 = getelementptr inbounds i8, ptr %418, i16 1
  store ptr %419, ptr @src, align 8
  %420 = load i8, ptr %418, align 1
  %421 = sext i8 %420 to i64
  store i64 %421, ptr @token_val, align 8
  %422 = load i64, ptr @token_val, align 8
  %423 = icmp eq i64 %422, 110
  %424 = zext i1 %423 to i32
  %425 = icmp ne i32 %424, 0
  br i1 %425, label %426, label %427

426:                                              ; preds = %417
  store i64 10, ptr @token_val, align 8
  br label %427

427:                                              ; preds = %426, %417
  br label %428

428:                                              ; preds = %427, %408
  %429 = load i64, ptr @token, align 8
  %430 = icmp eq i64 %429, 34
  %431 = zext i1 %430 to i32
  %432 = icmp ne i32 %431, 0
  br i1 %432, label %433, label %438

433:                                              ; preds = %428
  %434 = load ptr, ptr @data, align 8
  %435 = getelementptr inbounds i8, ptr %434, i16 1
  store ptr %435, ptr @data, align 8
  %436 = load i64, ptr @token_val, align 8
  %437 = trunc i64 %436 to i8
  store i8 %437, ptr %434, align 1
  br label %438

438:                                              ; preds = %433, %428
  br label %439

439:                                              ; preds = %438, %406
  %440 = load ptr, ptr @src, align 8
  %441 = load i8, ptr %440, align 1
  %442 = sext i8 %441 to i32
  %443 = icmp ne i32 %442, 0
  br i1 %443, label %444, label %451

444:                                              ; preds = %439
  %445 = load ptr, ptr @src, align 8
  %446 = load i8, ptr %445, align 1
  %447 = sext i8 %446 to i32
  %448 = sext i32 %447 to i64
  %449 = load i64, ptr @token, align 8
  %450 = icmp ne i64 %448, %449
  br label %451

451:                                              ; preds = %444, %439
  %452 = phi i1 [ %443, %439 ], [ %450, %444 ]
  %453 = zext i1 %452 to i32
  %454 = icmp ne i32 %453, 0
  br i1 %454, label %408, label %455

455:                                              ; preds = %451
  %456 = load ptr, ptr @src, align 8
  %457 = getelementptr inbounds i8, ptr %456, i16 1
  store ptr %457, ptr @src, align 8
  %458 = load i64, ptr @token, align 8
  %459 = icmp eq i64 %458, 34
  %460 = zext i1 %459 to i32
  %461 = icmp ne i32 %460, 0
  br i1 %461, label %462, label %465

462:                                              ; preds = %455
  %463 = load ptr, ptr %1, align 8
  %464 = ptrtoint ptr %463 to i64
  store i64 %464, ptr @token_val, align 8
  br label %466

465:                                              ; preds = %455
  store i64 128, ptr @token, align 8
  br label %466

466:                                              ; preds = %465, %462
  ret void

467:                                              ; preds = %402
  %468 = load i64, ptr @token, align 8
  %469 = icmp eq i64 %468, 61
  %470 = zext i1 %469 to i32
  %471 = icmp ne i32 %470, 0
  br i1 %471, label %472, label %484

472:                                              ; preds = %467
  %473 = load ptr, ptr @src, align 8
  %474 = load i8, ptr %473, align 1
  %475 = sext i8 %474 to i32
  %476 = icmp eq i32 %475, 61
  %477 = zext i1 %476 to i32
  %478 = icmp ne i32 %477, 0
  br i1 %478, label %479, label %482

479:                                              ; preds = %472
  %480 = load ptr, ptr @src, align 8
  %481 = getelementptr inbounds i8, ptr %480, i16 1
  store ptr %481, ptr @src, align 8
  store i64 149, ptr @token, align 8
  br label %483

482:                                              ; preds = %472
  store i64 142, ptr @token, align 8
  br label %483

483:                                              ; preds = %482, %479
  ret void

484:                                              ; preds = %467
  %485 = load i64, ptr @token, align 8
  %486 = icmp eq i64 %485, 43
  %487 = zext i1 %486 to i32
  %488 = icmp ne i32 %487, 0
  br i1 %488, label %489, label %501

489:                                              ; preds = %484
  %490 = load ptr, ptr @src, align 8
  %491 = load i8, ptr %490, align 1
  %492 = sext i8 %491 to i32
  %493 = icmp eq i32 %492, 43
  %494 = zext i1 %493 to i32
  %495 = icmp ne i32 %494, 0
  br i1 %495, label %496, label %499

496:                                              ; preds = %489
  %497 = load ptr, ptr @src, align 8
  %498 = getelementptr inbounds i8, ptr %497, i16 1
  store ptr %498, ptr @src, align 8
  store i64 162, ptr @token, align 8
  br label %500

499:                                              ; preds = %489
  store i64 157, ptr @token, align 8
  br label %500

500:                                              ; preds = %499, %496
  ret void

501:                                              ; preds = %484
  %502 = load i64, ptr @token, align 8
  %503 = icmp eq i64 %502, 45
  %504 = zext i1 %503 to i32
  %505 = icmp ne i32 %504, 0
  br i1 %505, label %506, label %518

506:                                              ; preds = %501
  %507 = load ptr, ptr @src, align 8
  %508 = load i8, ptr %507, align 1
  %509 = sext i8 %508 to i32
  %510 = icmp eq i32 %509, 45
  %511 = zext i1 %510 to i32
  %512 = icmp ne i32 %511, 0
  br i1 %512, label %513, label %516

513:                                              ; preds = %506
  %514 = load ptr, ptr @src, align 8
  %515 = getelementptr inbounds i8, ptr %514, i16 1
  store ptr %515, ptr @src, align 8
  store i64 163, ptr @token, align 8
  br label %517

516:                                              ; preds = %506
  store i64 158, ptr @token, align 8
  br label %517

517:                                              ; preds = %516, %513
  ret void

518:                                              ; preds = %501
  %519 = load i64, ptr @token, align 8
  %520 = icmp eq i64 %519, 33
  %521 = zext i1 %520 to i32
  %522 = icmp ne i32 %521, 0
  br i1 %522, label %523, label %534

523:                                              ; preds = %518
  %524 = load ptr, ptr @src, align 8
  %525 = load i8, ptr %524, align 1
  %526 = sext i8 %525 to i32
  %527 = icmp eq i32 %526, 61
  %528 = zext i1 %527 to i32
  %529 = icmp ne i32 %528, 0
  br i1 %529, label %530, label %533

530:                                              ; preds = %523
  %531 = load ptr, ptr @src, align 8
  %532 = getelementptr inbounds i8, ptr %531, i16 1
  store ptr %532, ptr @src, align 8
  store i64 150, ptr @token, align 8
  br label %533

533:                                              ; preds = %530, %523
  ret void

534:                                              ; preds = %518
  %535 = load i64, ptr @token, align 8
  %536 = icmp eq i64 %535, 60
  %537 = zext i1 %536 to i32
  %538 = icmp ne i32 %537, 0
  br i1 %538, label %539, label %562

539:                                              ; preds = %534
  %540 = load ptr, ptr @src, align 8
  %541 = load i8, ptr %540, align 1
  %542 = sext i8 %541 to i32
  %543 = icmp eq i32 %542, 61
  %544 = zext i1 %543 to i32
  %545 = icmp ne i32 %544, 0
  br i1 %545, label %546, label %549

546:                                              ; preds = %539
  %547 = load ptr, ptr @src, align 8
  %548 = getelementptr inbounds i8, ptr %547, i16 1
  store ptr %548, ptr @src, align 8
  store i64 153, ptr @token, align 8
  br label %561

549:                                              ; preds = %539
  %550 = load ptr, ptr @src, align 8
  %551 = load i8, ptr %550, align 1
  %552 = sext i8 %551 to i32
  %553 = icmp eq i32 %552, 60
  %554 = zext i1 %553 to i32
  %555 = icmp ne i32 %554, 0
  br i1 %555, label %556, label %559

556:                                              ; preds = %549
  %557 = load ptr, ptr @src, align 8
  %558 = getelementptr inbounds i8, ptr %557, i16 1
  store ptr %558, ptr @src, align 8
  store i64 155, ptr @token, align 8
  br label %560

559:                                              ; preds = %549
  store i64 151, ptr @token, align 8
  br label %560

560:                                              ; preds = %559, %556
  br label %561

561:                                              ; preds = %560, %546
  ret void

562:                                              ; preds = %534
  %563 = load i64, ptr @token, align 8
  %564 = icmp eq i64 %563, 62
  %565 = zext i1 %564 to i32
  %566 = icmp ne i32 %565, 0
  br i1 %566, label %567, label %590

567:                                              ; preds = %562
  %568 = load ptr, ptr @src, align 8
  %569 = load i8, ptr %568, align 1
  %570 = sext i8 %569 to i32
  %571 = icmp eq i32 %570, 61
  %572 = zext i1 %571 to i32
  %573 = icmp ne i32 %572, 0
  br i1 %573, label %574, label %577

574:                                              ; preds = %567
  %575 = load ptr, ptr @src, align 8
  %576 = getelementptr inbounds i8, ptr %575, i16 1
  store ptr %576, ptr @src, align 8
  store i64 154, ptr @token, align 8
  br label %589

577:                                              ; preds = %567
  %578 = load ptr, ptr @src, align 8
  %579 = load i8, ptr %578, align 1
  %580 = sext i8 %579 to i32
  %581 = icmp eq i32 %580, 62
  %582 = zext i1 %581 to i32
  %583 = icmp ne i32 %582, 0
  br i1 %583, label %584, label %587

584:                                              ; preds = %577
  %585 = load ptr, ptr @src, align 8
  %586 = getelementptr inbounds i8, ptr %585, i16 1
  store ptr %586, ptr @src, align 8
  store i64 156, ptr @token, align 8
  br label %588

587:                                              ; preds = %577
  store i64 152, ptr @token, align 8
  br label %588

588:                                              ; preds = %587, %584
  br label %589

589:                                              ; preds = %588, %574
  ret void

590:                                              ; preds = %562
  %591 = load i64, ptr @token, align 8
  %592 = icmp eq i64 %591, 124
  %593 = zext i1 %592 to i32
  %594 = icmp ne i32 %593, 0
  br i1 %594, label %595, label %607

595:                                              ; preds = %590
  %596 = load ptr, ptr @src, align 8
  %597 = load i8, ptr %596, align 1
  %598 = sext i8 %597 to i32
  %599 = icmp eq i32 %598, 124
  %600 = zext i1 %599 to i32
  %601 = icmp ne i32 %600, 0
  br i1 %601, label %602, label %605

602:                                              ; preds = %595
  %603 = load ptr, ptr @src, align 8
  %604 = getelementptr inbounds i8, ptr %603, i16 1
  store ptr %604, ptr @src, align 8
  store i64 144, ptr @token, align 8
  br label %606

605:                                              ; preds = %595
  store i64 146, ptr @token, align 8
  br label %606

606:                                              ; preds = %605, %602
  ret void

607:                                              ; preds = %590
  %608 = load i64, ptr @token, align 8
  %609 = icmp eq i64 %608, 38
  %610 = zext i1 %609 to i32
  %611 = icmp ne i32 %610, 0
  br i1 %611, label %612, label %624

612:                                              ; preds = %607
  %613 = load ptr, ptr @src, align 8
  %614 = load i8, ptr %613, align 1
  %615 = sext i8 %614 to i32
  %616 = icmp eq i32 %615, 38
  %617 = zext i1 %616 to i32
  %618 = icmp ne i32 %617, 0
  br i1 %618, label %619, label %622

619:                                              ; preds = %612
  %620 = load ptr, ptr @src, align 8
  %621 = getelementptr inbounds i8, ptr %620, i16 1
  store ptr %621, ptr @src, align 8
  store i64 145, ptr @token, align 8
  br label %623

622:                                              ; preds = %612
  store i64 148, ptr @token, align 8
  br label %623

623:                                              ; preds = %622, %619
  ret void

624:                                              ; preds = %607
  %625 = load i64, ptr @token, align 8
  %626 = icmp eq i64 %625, 94
  %627 = zext i1 %626 to i32
  %628 = icmp ne i32 %627, 0
  br i1 %628, label %629, label %630

629:                                              ; preds = %624
  store i64 147, ptr @token, align 8
  ret void

630:                                              ; preds = %624
  %631 = load i64, ptr @token, align 8
  %632 = icmp eq i64 %631, 37
  %633 = zext i1 %632 to i32
  %634 = icmp ne i32 %633, 0
  br i1 %634, label %635, label %636

635:                                              ; preds = %630
  store i64 161, ptr @token, align 8
  ret void

636:                                              ; preds = %630
  %637 = load i64, ptr @token, align 8
  %638 = icmp eq i64 %637, 42
  %639 = zext i1 %638 to i32
  %640 = icmp ne i32 %639, 0
  br i1 %640, label %641, label %642

641:                                              ; preds = %636
  store i64 159, ptr @token, align 8
  ret void

642:                                              ; preds = %636
  %643 = load i64, ptr @token, align 8
  %644 = icmp eq i64 %643, 91
  %645 = zext i1 %644 to i32
  %646 = icmp ne i32 %645, 0
  br i1 %646, label %647, label %648

647:                                              ; preds = %642
  store i64 164, ptr @token, align 8
  ret void

648:                                              ; preds = %642
  %649 = load i64, ptr @token, align 8
  %650 = icmp eq i64 %649, 63
  %651 = zext i1 %650 to i32
  %652 = icmp ne i32 %651, 0
  br i1 %652, label %653, label %654

653:                                              ; preds = %648
  store i64 143, ptr @token, align 8
  ret void

654:                                              ; preds = %648
  %655 = load i64, ptr @token, align 8
  %656 = icmp eq i64 %655, 58
  br i1 %656, label %702, label %657

657:                                              ; preds = %654
  %658 = load i64, ptr @token, align 8
  %659 = icmp eq i64 %658, 44
  br i1 %659, label %699, label %660

660:                                              ; preds = %657
  %661 = load i64, ptr @token, align 8
  %662 = icmp eq i64 %661, 93
  br i1 %662, label %696, label %663

663:                                              ; preds = %660
  %664 = load i64, ptr @token, align 8
  %665 = icmp eq i64 %664, 41
  br i1 %665, label %693, label %666

666:                                              ; preds = %663
  %667 = load i64, ptr @token, align 8
  %668 = icmp eq i64 %667, 40
  br i1 %668, label %690, label %669

669:                                              ; preds = %666
  %670 = load i64, ptr @token, align 8
  %671 = icmp eq i64 %670, 125
  br i1 %671, label %687, label %672

672:                                              ; preds = %669
  %673 = load i64, ptr @token, align 8
  %674 = icmp eq i64 %673, 123
  br i1 %674, label %684, label %675

675:                                              ; preds = %672
  %676 = load i64, ptr @token, align 8
  %677 = icmp eq i64 %676, 59
  br i1 %677, label %681, label %678

678:                                              ; preds = %675
  %679 = load i64, ptr @token, align 8
  %680 = icmp eq i64 %679, 126
  br label %681

681:                                              ; preds = %678, %675
  %682 = phi i1 [ %677, %675 ], [ %680, %678 ]
  %683 = icmp ne i1 %682, false
  br label %684

684:                                              ; preds = %681, %672
  %685 = phi i1 [ %674, %672 ], [ %683, %681 ]
  %686 = icmp ne i1 %685, false
  br label %687

687:                                              ; preds = %684, %669
  %688 = phi i1 [ %671, %669 ], [ %686, %684 ]
  %689 = icmp ne i1 %688, false
  br label %690

690:                                              ; preds = %687, %666
  %691 = phi i1 [ %668, %666 ], [ %689, %687 ]
  %692 = icmp ne i1 %691, false
  br label %693

693:                                              ; preds = %690, %663
  %694 = phi i1 [ %665, %663 ], [ %692, %690 ]
  %695 = icmp ne i1 %694, false
  br label %696

696:                                              ; preds = %693, %660
  %697 = phi i1 [ %662, %660 ], [ %695, %693 ]
  %698 = icmp ne i1 %697, false
  br label %699

699:                                              ; preds = %696, %657
  %700 = phi i1 [ %659, %657 ], [ %698, %696 ]
  %701 = icmp ne i1 %700, false
  br label %702

702:                                              ; preds = %699, %654
  %703 = phi i1 [ %656, %654 ], [ %701, %699 ]
  %704 = zext i1 %703 to i32
  %705 = icmp ne i32 %704, 0
  br i1 %705, label %706, label %707

706:                                              ; preds = %702
  ret void

707:                                              ; preds = %702
  br label %708

708:                                              ; preds = %707
  br label %709

709:                                              ; preds = %708
  br label %710

710:                                              ; preds = %709
  br label %711

711:                                              ; preds = %710
  br label %712

712:                                              ; preds = %711
  br label %713

713:                                              ; preds = %712
  br label %714

714:                                              ; preds = %713
  br label %715

715:                                              ; preds = %714
  br label %716

716:                                              ; preds = %715
  br label %717

717:                                              ; preds = %716
  br label %718

718:                                              ; preds = %717
  br label %719

719:                                              ; preds = %718
  br label %720

720:                                              ; preds = %719
  br label %721

721:                                              ; preds = %720
  br label %722

722:                                              ; preds = %721, %395
  br label %723

723:                                              ; preds = %722
  br label %724

724:                                              ; preds = %723
  br label %725

725:                                              ; preds = %724, %76
  br label %726

726:                                              ; preds = %725, %50
  br label %727

727:                                              ; preds = %726, %0
  %728 = load ptr, ptr @src, align 8
  %729 = load i8, ptr %728, align 1
  %730 = sext i8 %729 to i64
  store i64 %730, ptr @token, align 8
  %731 = icmp ne i64 %730, 0
  br i1 %731, label %3, label %732

732:                                              ; preds = %727
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
  call void (...) @next()
  br label %13

9:                                                ; preds = %1
  %10 = load i64, ptr @line, align 8
  %11 = load i64, ptr %2, align 8
  %12 = call i32 (ptr, ...) @printf(ptr @.cstr.5, i64 %10, i64 %11)
  call void @exit(i32 -1)
  br label %13

13:                                               ; preds = %9, %8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @expression(i64 %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = load i64, ptr @token, align 8
  %7 = icmp eq i64 %6, 0
  %8 = zext i1 %7 to i32
  %9 = icmp ne i32 %8, 0
  br i1 %9, label %10, label %13

10:                                               ; preds = %1
  %11 = load i64, ptr @line, align 8
  %12 = call i32 (ptr, ...) @printf(ptr @.cstr.6, i64 %11)
  call void @exit(i32 -1)
  br label %13

13:                                               ; preds = %10, %1
  %14 = load i64, ptr @token, align 8
  %15 = icmp eq i64 %14, 128
  %16 = zext i1 %15 to i32
  %17 = icmp ne i32 %16, 0
  br i1 %17, label %18, label %24

18:                                               ; preds = %13
  call void @match(i64 128)
  %19 = load ptr, ptr @text, align 8
  %20 = getelementptr inbounds i64, ptr %19, i64 1
  store ptr %20, ptr @text, align 8
  store i64 1, ptr %20, align 8
  %21 = load ptr, ptr @text, align 8
  %22 = getelementptr inbounds i64, ptr %21, i64 1
  store ptr %22, ptr @text, align 8
  %23 = load i64, ptr @token_val, align 8
  store i64 %23, ptr %22, align 8
  store i64 1, ptr @expr_type, align 8
  br label %449

24:                                               ; preds = %13
  %25 = load i64, ptr @token, align 8
  %26 = icmp eq i64 %25, 34
  %27 = zext i1 %26 to i32
  %28 = icmp ne i32 %27, 0
  br i1 %28, label %29, label %47

29:                                               ; preds = %24
  %30 = load ptr, ptr @text, align 8
  %31 = getelementptr inbounds i64, ptr %30, i64 1
  store ptr %31, ptr @text, align 8
  store i64 1, ptr %31, align 8
  %32 = load ptr, ptr @text, align 8
  %33 = getelementptr inbounds i64, ptr %32, i64 1
  store ptr %33, ptr @text, align 8
  %34 = load i64, ptr @token_val, align 8
  store i64 %34, ptr %33, align 8
  call void @match(i64 34)
  br label %36

35:                                               ; preds = %36
  call void @match(i64 34)
  br label %36

36:                                               ; preds = %35, %29
  %37 = load i64, ptr @token, align 8
  %38 = icmp eq i64 %37, 34
  %39 = zext i1 %38 to i32
  %40 = icmp ne i32 %39, 0
  br i1 %40, label %35, label %41

41:                                               ; preds = %36
  %42 = load ptr, ptr @data, align 8
  %43 = ptrtoint ptr %42 to i64
  %44 = add i64 %43, 8
  %45 = and i64 %44, -8
  %46 = inttoptr i64 %45 to ptr
  store ptr %46, ptr @data, align 8
  store i64 2, ptr @expr_type, align 8
  br label %448

47:                                               ; preds = %24
  %48 = load i64, ptr @token, align 8
  %49 = icmp eq i64 %48, 140
  %50 = zext i1 %49 to i32
  %51 = icmp ne i32 %50, 0
  br i1 %51, label %52, label %84

52:                                               ; preds = %47
  call void @match(i64 140)
  call void @match(i64 40)
  store i64 1, ptr @expr_type, align 8
  %53 = load i64, ptr @token, align 8
  %54 = icmp eq i64 %53, 138
  %55 = zext i1 %54 to i32
  %56 = icmp ne i32 %55, 0
  br i1 %56, label %57, label %58

57:                                               ; preds = %52
  call void @match(i64 138)
  br label %65

58:                                               ; preds = %52
  %59 = load i64, ptr @token, align 8
  %60 = icmp eq i64 %59, 134
  %61 = zext i1 %60 to i32
  %62 = icmp ne i32 %61, 0
  br i1 %62, label %63, label %64

63:                                               ; preds = %58
  call void @match(i64 134)
  store i64 0, ptr @expr_type, align 8
  br label %64

64:                                               ; preds = %63, %58
  br label %65

65:                                               ; preds = %64, %57
  br label %69

66:                                               ; preds = %69
  call void @match(i64 159)
  %67 = load i64, ptr @expr_type, align 8
  %68 = add nsw i64 %67, 2
  store i64 %68, ptr @expr_type, align 8
  br label %69

69:                                               ; preds = %66, %65
  %70 = load i64, ptr @token, align 8
  %71 = icmp eq i64 %70, 159
  %72 = zext i1 %71 to i32
  %73 = icmp ne i32 %72, 0
  br i1 %73, label %66, label %74

74:                                               ; preds = %69
  call void @match(i64 41)
  %75 = load ptr, ptr @text, align 8
  %76 = getelementptr inbounds i64, ptr %75, i64 1
  store ptr %76, ptr @text, align 8
  store i64 1, ptr %76, align 8
  %77 = load ptr, ptr @text, align 8
  %78 = getelementptr inbounds i64, ptr %77, i64 1
  store ptr %78, ptr @text, align 8
  %79 = load i64, ptr @expr_type, align 8
  %80 = icmp eq i64 %79, 0
  %81 = zext i1 %80 to i32
  %82 = icmp ne i32 %81, 0
  %83 = select i1 %82, i64 1, i64 8
  store i64 %83, ptr %78, align 8
  store i64 1, ptr @expr_type, align 8
  br label %447

84:                                               ; preds = %47
  %85 = load i64, ptr @token, align 8
  %86 = icmp eq i64 %85, 133
  %87 = zext i1 %86 to i32
  %88 = icmp ne i32 %87, 0
  br i1 %88, label %89, label %224

89:                                               ; preds = %84
  call void @match(i64 133)
  %90 = load ptr, ptr @current_id, align 8
  store ptr %90, ptr %3, align 8
  %91 = load i64, ptr @token, align 8
  %92 = icmp eq i64 %91, 40
  %93 = zext i1 %92 to i32
  %94 = icmp ne i32 %93, 0
  br i1 %94, label %95, label %159

95:                                               ; preds = %89
  call void @match(i64 40)
  store i64 0, ptr %4, align 8
  br label %107

96:                                               ; preds = %107
  call void @expression(i64 142)
  %97 = load ptr, ptr @text, align 8
  %98 = getelementptr inbounds i64, ptr %97, i64 1
  store ptr %98, ptr @text, align 8
  store i64 13, ptr %98, align 8
  %99 = load i64, ptr %4, align 8
  %100 = add i64 %99, 1
  store i64 %100, ptr %4, align 8
  %101 = load i64, ptr @token, align 8
  %102 = icmp eq i64 %101, 44
  %103 = zext i1 %102 to i32
  %104 = icmp ne i32 %103, 0
  br i1 %104, label %105, label %106

105:                                              ; preds = %96
  call void @match(i64 44)
  br label %106

106:                                              ; preds = %105, %96
  br label %107

107:                                              ; preds = %106, %95
  %108 = load i64, ptr @token, align 8
  %109 = icmp ne i64 %108, 41
  %110 = zext i1 %109 to i32
  %111 = icmp ne i32 %110, 0
  br i1 %111, label %96, label %112

112:                                              ; preds = %107
  call void @match(i64 41)
  %113 = load ptr, ptr %3, align 8
  %114 = getelementptr inbounds i64, ptr %113, i32 4
  %115 = load i64, ptr %114, align 8
  %116 = icmp eq i64 %115, 130
  %117 = zext i1 %116 to i32
  %118 = icmp ne i32 %117, 0
  br i1 %118, label %119, label %125

119:                                              ; preds = %112
  %120 = load ptr, ptr @text, align 8
  %121 = getelementptr inbounds i64, ptr %120, i64 1
  store ptr %121, ptr @text, align 8
  %122 = load ptr, ptr %3, align 8
  %123 = getelementptr inbounds i64, ptr %122, i32 5
  %124 = load i64, ptr %123, align 8
  store i64 %124, ptr %121, align 8
  br label %144

125:                                              ; preds = %112
  %126 = load ptr, ptr %3, align 8
  %127 = getelementptr inbounds i64, ptr %126, i32 4
  %128 = load i64, ptr %127, align 8
  %129 = icmp eq i64 %128, 129
  %130 = zext i1 %129 to i32
  %131 = icmp ne i32 %130, 0
  br i1 %131, label %132, label %140

132:                                              ; preds = %125
  %133 = load ptr, ptr @text, align 8
  %134 = getelementptr inbounds i64, ptr %133, i64 1
  store ptr %134, ptr @text, align 8
  store i64 3, ptr %134, align 8
  %135 = load ptr, ptr @text, align 8
  %136 = getelementptr inbounds i64, ptr %135, i64 1
  store ptr %136, ptr @text, align 8
  %137 = load ptr, ptr %3, align 8
  %138 = getelementptr inbounds i64, ptr %137, i32 5
  %139 = load i64, ptr %138, align 8
  store i64 %139, ptr %136, align 8
  br label %143

140:                                              ; preds = %125
  %141 = load i64, ptr @line, align 8
  %142 = call i32 (ptr, ...) @printf(ptr @.cstr.7, i64 %141)
  call void @exit(i32 -1)
  br label %143

143:                                              ; preds = %140, %132
  br label %144

144:                                              ; preds = %143, %119
  %145 = load i64, ptr %4, align 8
  %146 = icmp sgt i64 %145, 0
  %147 = zext i1 %146 to i32
  %148 = icmp ne i32 %147, 0
  br i1 %148, label %149, label %155

149:                                              ; preds = %144
  %150 = load ptr, ptr @text, align 8
  %151 = getelementptr inbounds i64, ptr %150, i64 1
  store ptr %151, ptr @text, align 8
  store i64 7, ptr %151, align 8
  %152 = load ptr, ptr @text, align 8
  %153 = getelementptr inbounds i64, ptr %152, i64 1
  store ptr %153, ptr @text, align 8
  %154 = load i64, ptr %4, align 8
  store i64 %154, ptr %153, align 8
  br label %155

155:                                              ; preds = %149, %144
  %156 = load ptr, ptr %3, align 8
  %157 = getelementptr inbounds i64, ptr %156, i32 3
  %158 = load i64, ptr %157, align 8
  store i64 %158, ptr @expr_type, align 8
  br label %223

159:                                              ; preds = %89
  %160 = load ptr, ptr %3, align 8
  %161 = getelementptr inbounds i64, ptr %160, i32 4
  %162 = load i64, ptr %161, align 8
  %163 = icmp eq i64 %162, 128
  %164 = zext i1 %163 to i32
  %165 = icmp ne i32 %164, 0
  br i1 %165, label %166, label %174

166:                                              ; preds = %159
  %167 = load ptr, ptr @text, align 8
  %168 = getelementptr inbounds i64, ptr %167, i64 1
  store ptr %168, ptr @text, align 8
  store i64 1, ptr %168, align 8
  %169 = load ptr, ptr @text, align 8
  %170 = getelementptr inbounds i64, ptr %169, i64 1
  store ptr %170, ptr @text, align 8
  %171 = load ptr, ptr %3, align 8
  %172 = getelementptr inbounds i64, ptr %171, i32 5
  %173 = load i64, ptr %172, align 8
  store i64 %173, ptr %170, align 8
  store i64 1, ptr @expr_type, align 8
  br label %222

174:                                              ; preds = %159
  %175 = load ptr, ptr %3, align 8
  %176 = getelementptr inbounds i64, ptr %175, i32 4
  %177 = load i64, ptr %176, align 8
  %178 = icmp eq i64 %177, 132
  %179 = zext i1 %178 to i32
  %180 = icmp ne i32 %179, 0
  br i1 %180, label %181, label %191

181:                                              ; preds = %174
  %182 = load ptr, ptr @text, align 8
  %183 = getelementptr inbounds i64, ptr %182, i64 1
  store ptr %183, ptr @text, align 8
  store i64 0, ptr %183, align 8
  %184 = load ptr, ptr @text, align 8
  %185 = getelementptr inbounds i64, ptr %184, i64 1
  store ptr %185, ptr @text, align 8
  %186 = load i64, ptr @index_of_bp, align 8
  %187 = load ptr, ptr %3, align 8
  %188 = getelementptr inbounds i64, ptr %187, i32 5
  %189 = load i64, ptr %188, align 8
  %190 = sub nsw i64 %186, %189
  store i64 %190, ptr %185, align 8
  br label %210

191:                                              ; preds = %174
  %192 = load ptr, ptr %3, align 8
  %193 = getelementptr inbounds i64, ptr %192, i32 4
  %194 = load i64, ptr %193, align 8
  %195 = icmp eq i64 %194, 131
  %196 = zext i1 %195 to i32
  %197 = icmp ne i32 %196, 0
  br i1 %197, label %198, label %206

198:                                              ; preds = %191
  %199 = load ptr, ptr @text, align 8
  %200 = getelementptr inbounds i64, ptr %199, i64 1
  store ptr %200, ptr @text, align 8
  store i64 1, ptr %200, align 8
  %201 = load ptr, ptr @text, align 8
  %202 = getelementptr inbounds i64, ptr %201, i64 1
  store ptr %202, ptr @text, align 8
  %203 = load ptr, ptr %3, align 8
  %204 = getelementptr inbounds i64, ptr %203, i32 5
  %205 = load i64, ptr %204, align 8
  store i64 %205, ptr %202, align 8
  br label %209

206:                                              ; preds = %191
  %207 = load i64, ptr @line, align 8
  %208 = call i32 (ptr, ...) @printf(ptr @.cstr.8, i64 %207)
  call void @exit(i32 -1)
  br label %209

209:                                              ; preds = %206, %198
  br label %210

210:                                              ; preds = %209, %181
  %211 = load ptr, ptr %3, align 8
  %212 = getelementptr inbounds i64, ptr %211, i32 3
  %213 = load i64, ptr %212, align 8
  store i64 %213, ptr @expr_type, align 8
  %214 = load ptr, ptr @text, align 8
  %215 = getelementptr inbounds i64, ptr %214, i64 1
  store ptr %215, ptr @text, align 8
  %216 = load i64, ptr @expr_type, align 8
  %217 = icmp eq i64 %216, 0
  %218 = zext i1 %217 to i32
  %219 = icmp ne i32 %218, 0
  %220 = select i1 %219, i32 10, i32 9
  %221 = sext i32 %220 to i64
  store i64 %221, ptr %215, align 8
  br label %222

222:                                              ; preds = %210, %166
  br label %223

223:                                              ; preds = %222, %155
  br label %446

224:                                              ; preds = %84
  %225 = load i64, ptr @token, align 8
  %226 = icmp eq i64 %225, 40
  %227 = zext i1 %226 to i32
  %228 = icmp ne i32 %227, 0
  br i1 %228, label %229, label %259

229:                                              ; preds = %224
  call void @match(i64 40)
  %230 = load i64, ptr @token, align 8
  %231 = icmp eq i64 %230, 134
  br i1 %231, label %235, label %232

232:                                              ; preds = %229
  %233 = load i64, ptr @token, align 8
  %234 = icmp eq i64 %233, 138
  br label %235

235:                                              ; preds = %232, %229
  %236 = phi i1 [ %231, %229 ], [ %234, %232 ]
  %237 = zext i1 %236 to i32
  %238 = icmp ne i32 %237, 0
  br i1 %238, label %239, label %257

239:                                              ; preds = %235
  %240 = load i64, ptr @token, align 8
  %241 = icmp eq i64 %240, 134
  %242 = zext i1 %241 to i32
  %243 = icmp ne i32 %242, 0
  %244 = select i1 %243, i32 0, i32 1
  %245 = sext i32 %244 to i64
  store i64 %245, ptr %4, align 8
  %246 = load i64, ptr @token, align 8
  call void @match(i64 %246)
  br label %250

247:                                              ; preds = %250
  call void @match(i64 159)
  %248 = load i64, ptr %4, align 8
  %249 = add nsw i64 %248, 2
  store i64 %249, ptr %4, align 8
  br label %250

250:                                              ; preds = %247, %239
  %251 = load i64, ptr @token, align 8
  %252 = icmp eq i64 %251, 159
  %253 = zext i1 %252 to i32
  %254 = icmp ne i32 %253, 0
  br i1 %254, label %247, label %255

255:                                              ; preds = %250
  call void @match(i64 41)
  call void @expression(i64 162)
  %256 = load i64, ptr %4, align 8
  store i64 %256, ptr @expr_type, align 8
  br label %258

257:                                              ; preds = %235
  call void @expression(i64 142)
  call void @match(i64 41)
  br label %258

258:                                              ; preds = %257, %255
  br label %445

259:                                              ; preds = %224
  %260 = load i64, ptr @token, align 8
  %261 = icmp eq i64 %260, 159
  %262 = zext i1 %261 to i32
  %263 = icmp ne i32 %262, 0
  br i1 %263, label %264, label %284

264:                                              ; preds = %259
  call void @match(i64 159)
  call void @expression(i64 162)
  %265 = load i64, ptr @expr_type, align 8
  %266 = icmp sge i64 %265, 2
  %267 = zext i1 %266 to i32
  %268 = icmp ne i32 %267, 0
  br i1 %268, label %269, label %272

269:                                              ; preds = %264
  %270 = load i64, ptr @expr_type, align 8
  %271 = sub nsw i64 %270, 2
  store i64 %271, ptr @expr_type, align 8
  br label %275

272:                                              ; preds = %264
  %273 = load i64, ptr @line, align 8
  %274 = call i32 (ptr, ...) @printf(ptr @.cstr.9, i64 %273)
  call void @exit(i32 -1)
  br label %275

275:                                              ; preds = %272, %269
  %276 = load ptr, ptr @text, align 8
  %277 = getelementptr inbounds i64, ptr %276, i64 1
  store ptr %277, ptr @text, align 8
  %278 = load i64, ptr @expr_type, align 8
  %279 = icmp eq i64 %278, 0
  %280 = zext i1 %279 to i32
  %281 = icmp ne i32 %280, 0
  %282 = select i1 %281, i32 10, i32 9
  %283 = sext i32 %282 to i64
  store i64 %283, ptr %277, align 8
  br label %444

284:                                              ; preds = %259
  %285 = load i64, ptr @token, align 8
  %286 = icmp eq i64 %285, 148
  %287 = zext i1 %286 to i32
  %288 = icmp ne i32 %287, 0
  br i1 %288, label %289, label %310

289:                                              ; preds = %284
  call void @match(i64 148)
  call void @expression(i64 162)
  %290 = load ptr, ptr @text, align 8
  %291 = load i64, ptr %290, align 8
  %292 = icmp eq i64 %291, 9
  br i1 %292, label %297, label %293

293:                                              ; preds = %289
  %294 = load ptr, ptr @text, align 8
  %295 = load i64, ptr %294, align 8
  %296 = icmp eq i64 %295, 10
  br label %297

297:                                              ; preds = %293, %289
  %298 = phi i1 [ %292, %289 ], [ %296, %293 ]
  %299 = zext i1 %298 to i32
  %300 = icmp ne i32 %299, 0
  br i1 %300, label %301, label %304

301:                                              ; preds = %297
  %302 = load ptr, ptr @text, align 8
  %303 = getelementptr inbounds i64, ptr %302, i16 -1
  store ptr %303, ptr @text, align 8
  br label %307

304:                                              ; preds = %297
  %305 = load i64, ptr @line, align 8
  %306 = call i32 (ptr, ...) @printf(ptr @.cstr.10, i64 %305)
  call void @exit(i32 -1)
  br label %307

307:                                              ; preds = %304, %301
  %308 = load i64, ptr @expr_type, align 8
  %309 = add nsw i64 %308, 2
  store i64 %309, ptr @expr_type, align 8
  br label %443

310:                                              ; preds = %284
  %311 = load i64, ptr @token, align 8
  %312 = icmp eq i64 %311, 33
  %313 = zext i1 %312 to i32
  %314 = icmp ne i32 %313, 0
  br i1 %314, label %315, label %324

315:                                              ; preds = %310
  call void @match(i64 33)
  call void @expression(i64 162)
  %316 = load ptr, ptr @text, align 8
  %317 = getelementptr inbounds i64, ptr %316, i64 1
  store ptr %317, ptr @text, align 8
  store i64 13, ptr %317, align 8
  %318 = load ptr, ptr @text, align 8
  %319 = getelementptr inbounds i64, ptr %318, i64 1
  store ptr %319, ptr @text, align 8
  store i64 1, ptr %319, align 8
  %320 = load ptr, ptr @text, align 8
  %321 = getelementptr inbounds i64, ptr %320, i64 1
  store ptr %321, ptr @text, align 8
  store i64 0, ptr %321, align 8
  %322 = load ptr, ptr @text, align 8
  %323 = getelementptr inbounds i64, ptr %322, i64 1
  store ptr %323, ptr @text, align 8
  store i64 17, ptr %323, align 8
  store i64 1, ptr @expr_type, align 8
  br label %442

324:                                              ; preds = %310
  %325 = load i64, ptr @token, align 8
  %326 = icmp eq i64 %325, 126
  %327 = zext i1 %326 to i32
  %328 = icmp ne i32 %327, 0
  br i1 %328, label %329, label %338

329:                                              ; preds = %324
  call void @match(i64 126)
  call void @expression(i64 162)
  %330 = load ptr, ptr @text, align 8
  %331 = getelementptr inbounds i64, ptr %330, i64 1
  store ptr %331, ptr @text, align 8
  store i64 13, ptr %331, align 8
  %332 = load ptr, ptr @text, align 8
  %333 = getelementptr inbounds i64, ptr %332, i64 1
  store ptr %333, ptr @text, align 8
  store i64 1, ptr %333, align 8
  %334 = load ptr, ptr @text, align 8
  %335 = getelementptr inbounds i64, ptr %334, i64 1
  store ptr %335, ptr @text, align 8
  store i64 -1, ptr %335, align 8
  %336 = load ptr, ptr @text, align 8
  %337 = getelementptr inbounds i64, ptr %336, i64 1
  store ptr %337, ptr @text, align 8
  store i64 15, ptr %337, align 8
  store i64 1, ptr @expr_type, align 8
  br label %441

338:                                              ; preds = %324
  %339 = load i64, ptr @token, align 8
  %340 = icmp eq i64 %339, 157
  %341 = zext i1 %340 to i32
  %342 = icmp ne i32 %341, 0
  br i1 %342, label %343, label %344

343:                                              ; preds = %338
  call void @match(i64 157)
  call void @expression(i64 162)
  store i64 1, ptr @expr_type, align 8
  br label %440

344:                                              ; preds = %338
  %345 = load i64, ptr @token, align 8
  %346 = icmp eq i64 %345, 158
  %347 = zext i1 %346 to i32
  %348 = icmp ne i32 %347, 0
  br i1 %348, label %349, label %371

349:                                              ; preds = %344
  call void @match(i64 158)
  %350 = load i64, ptr @token, align 8
  %351 = icmp eq i64 %350, 128
  %352 = zext i1 %351 to i32
  %353 = icmp ne i32 %352, 0
  br i1 %353, label %354, label %361

354:                                              ; preds = %349
  %355 = load ptr, ptr @text, align 8
  %356 = getelementptr inbounds i64, ptr %355, i64 1
  store ptr %356, ptr @text, align 8
  store i64 1, ptr %356, align 8
  %357 = load ptr, ptr @text, align 8
  %358 = getelementptr inbounds i64, ptr %357, i64 1
  store ptr %358, ptr @text, align 8
  %359 = load i64, ptr @token_val, align 8
  %360 = sub nsw i64 0, %359
  store i64 %360, ptr %358, align 8
  call void @match(i64 128)
  br label %370

361:                                              ; preds = %349
  %362 = load ptr, ptr @text, align 8
  %363 = getelementptr inbounds i64, ptr %362, i64 1
  store ptr %363, ptr @text, align 8
  store i64 1, ptr %363, align 8
  %364 = load ptr, ptr @text, align 8
  %365 = getelementptr inbounds i64, ptr %364, i64 1
  store ptr %365, ptr @text, align 8
  store i64 -1, ptr %365, align 8
  %366 = load ptr, ptr @text, align 8
  %367 = getelementptr inbounds i64, ptr %366, i64 1
  store ptr %367, ptr @text, align 8
  store i64 13, ptr %367, align 8
  call void @expression(i64 162)
  %368 = load ptr, ptr @text, align 8
  %369 = getelementptr inbounds i64, ptr %368, i64 1
  store ptr %369, ptr @text, align 8
  store i64 27, ptr %369, align 8
  br label %370

370:                                              ; preds = %361, %354
  store i64 1, ptr @expr_type, align 8
  br label %439

371:                                              ; preds = %344
  %372 = load i64, ptr @token, align 8
  %373 = icmp eq i64 %372, 163
  br i1 %373, label %377, label %374

374:                                              ; preds = %371
  %375 = load i64, ptr @token, align 8
  %376 = icmp eq i64 %375, 162
  br label %377

377:                                              ; preds = %374, %371
  %378 = phi i1 [ %373, %371 ], [ %376, %374 ]
  %379 = zext i1 %378 to i32
  %380 = icmp ne i32 %379, 0
  br i1 %380, label %381, label %435

381:                                              ; preds = %377
  %382 = load i64, ptr @token, align 8
  store i64 %382, ptr %4, align 8
  %383 = load i64, ptr @token, align 8
  call void @match(i64 %383)
  call void @expression(i64 162)
  %384 = load ptr, ptr @text, align 8
  %385 = load i64, ptr %384, align 8
  %386 = icmp eq i64 %385, 10
  %387 = zext i1 %386 to i32
  %388 = icmp ne i32 %387, 0
  br i1 %388, label %389, label %393

389:                                              ; preds = %381
  %390 = load ptr, ptr @text, align 8
  store i64 13, ptr %390, align 8
  %391 = load ptr, ptr @text, align 8
  %392 = getelementptr inbounds i64, ptr %391, i64 1
  store ptr %392, ptr @text, align 8
  store i64 10, ptr %392, align 8
  br label %407

393:                                              ; preds = %381
  %394 = load ptr, ptr @text, align 8
  %395 = load i64, ptr %394, align 8
  %396 = icmp eq i64 %395, 9
  %397 = zext i1 %396 to i32
  %398 = icmp ne i32 %397, 0
  br i1 %398, label %399, label %403

399:                                              ; preds = %393
  %400 = load ptr, ptr @text, align 8
  store i64 13, ptr %400, align 8
  %401 = load ptr, ptr @text, align 8
  %402 = getelementptr inbounds i64, ptr %401, i64 1
  store ptr %402, ptr @text, align 8
  store i64 9, ptr %402, align 8
  br label %406

403:                                              ; preds = %393
  %404 = load i64, ptr @line, align 8
  %405 = call i32 (ptr, ...) @printf(ptr @.cstr.11, i64 %404)
  call void @exit(i32 -1)
  br label %406

406:                                              ; preds = %403, %399
  br label %407

407:                                              ; preds = %406, %389
  %408 = load ptr, ptr @text, align 8
  %409 = getelementptr inbounds i64, ptr %408, i64 1
  store ptr %409, ptr @text, align 8
  store i64 13, ptr %409, align 8
  %410 = load ptr, ptr @text, align 8
  %411 = getelementptr inbounds i64, ptr %410, i64 1
  store ptr %411, ptr @text, align 8
  store i64 1, ptr %411, align 8
  %412 = load ptr, ptr @text, align 8
  %413 = getelementptr inbounds i64, ptr %412, i64 1
  store ptr %413, ptr @text, align 8
  %414 = load i64, ptr @expr_type, align 8
  %415 = icmp sgt i64 %414, 2
  %416 = zext i1 %415 to i32
  %417 = icmp ne i32 %416, 0
  %418 = select i1 %417, i64 8, i64 1
  store i64 %418, ptr %413, align 8
  %419 = load ptr, ptr @text, align 8
  %420 = getelementptr inbounds i64, ptr %419, i64 1
  store ptr %420, ptr @text, align 8
  %421 = load i64, ptr %4, align 8
  %422 = icmp eq i64 %421, 162
  %423 = zext i1 %422 to i32
  %424 = icmp ne i32 %423, 0
  %425 = select i1 %424, i32 25, i32 26
  %426 = sext i32 %425 to i64
  store i64 %426, ptr %420, align 8
  %427 = load ptr, ptr @text, align 8
  %428 = getelementptr inbounds i64, ptr %427, i64 1
  store ptr %428, ptr @text, align 8
  %429 = load i64, ptr @expr_type, align 8
  %430 = icmp eq i64 %429, 0
  %431 = zext i1 %430 to i32
  %432 = icmp ne i32 %431, 0
  %433 = select i1 %432, i32 12, i32 11
  %434 = sext i32 %433 to i64
  store i64 %434, ptr %428, align 8
  br label %438

435:                                              ; preds = %377
  %436 = load i64, ptr @line, align 8
  %437 = call i32 (ptr, ...) @printf(ptr @.cstr.12, i64 %436)
  call void @exit(i32 -1)
  br label %438

438:                                              ; preds = %435, %407
  br label %439

439:                                              ; preds = %438, %370
  br label %440

440:                                              ; preds = %439, %343
  br label %441

441:                                              ; preds = %440, %329
  br label %442

442:                                              ; preds = %441, %315
  br label %443

443:                                              ; preds = %442, %307
  br label %444

444:                                              ; preds = %443, %275
  br label %445

445:                                              ; preds = %444, %258
  br label %446

446:                                              ; preds = %445, %223
  br label %447

447:                                              ; preds = %446, %74
  br label %448

448:                                              ; preds = %447, %41
  br label %449

449:                                              ; preds = %448, %18
  br label %913

450:                                              ; preds = %913
  %451 = load i64, ptr @expr_type, align 8
  store i64 %451, ptr %4, align 8
  %452 = load i64, ptr @token, align 8
  %453 = icmp eq i64 %452, 142
  %454 = zext i1 %453 to i32
  %455 = icmp ne i32 %454, 0
  br i1 %455, label %456, label %483

456:                                              ; preds = %450
  call void @match(i64 142)
  %457 = load ptr, ptr @text, align 8
  %458 = load i64, ptr %457, align 8
  %459 = icmp eq i64 %458, 9
  br i1 %459, label %464, label %460

460:                                              ; preds = %456
  %461 = load ptr, ptr @text, align 8
  %462 = load i64, ptr %461, align 8
  %463 = icmp eq i64 %462, 10
  br label %464

464:                                              ; preds = %460, %456
  %465 = phi i1 [ %459, %456 ], [ %463, %460 ]
  %466 = zext i1 %465 to i32
  %467 = icmp ne i32 %466, 0
  br i1 %467, label %468, label %470

468:                                              ; preds = %464
  %469 = load ptr, ptr @text, align 8
  store i64 13, ptr %469, align 8
  br label %473

470:                                              ; preds = %464
  %471 = load i64, ptr @line, align 8
  %472 = call i32 (ptr, ...) @printf(ptr @.cstr.13, i64 %471)
  call void @exit(i32 -1)
  br label %473

473:                                              ; preds = %470, %468
  call void @expression(i64 142)
  %474 = load i64, ptr %4, align 8
  store i64 %474, ptr @expr_type, align 8
  %475 = load ptr, ptr @text, align 8
  %476 = getelementptr inbounds i64, ptr %475, i64 1
  store ptr %476, ptr @text, align 8
  %477 = load i64, ptr @expr_type, align 8
  %478 = icmp eq i64 %477, 0
  %479 = zext i1 %478 to i32
  %480 = icmp ne i32 %479, 0
  %481 = select i1 %480, i32 12, i32 11
  %482 = sext i32 %481 to i64
  store i64 %482, ptr %476, align 8
  br label %912

483:                                              ; preds = %450
  %484 = load i64, ptr @token, align 8
  %485 = icmp eq i64 %484, 143
  %486 = zext i1 %485 to i32
  %487 = icmp ne i32 %486, 0
  br i1 %487, label %488, label %514

488:                                              ; preds = %483
  call void @match(i64 143)
  %489 = load ptr, ptr @text, align 8
  %490 = getelementptr inbounds i64, ptr %489, i64 1
  store ptr %490, ptr @text, align 8
  store i64 4, ptr %490, align 8
  %491 = load ptr, ptr @text, align 8
  %492 = getelementptr inbounds i64, ptr %491, i64 1
  store ptr %492, ptr @text, align 8
  store ptr %492, ptr %5, align 8
  call void @expression(i64 142)
  %493 = load i64, ptr @token, align 8
  %494 = icmp eq i64 %493, 58
  %495 = zext i1 %494 to i32
  %496 = icmp ne i32 %495, 0
  br i1 %496, label %497, label %498

497:                                              ; preds = %488
  call void @match(i64 58)
  br label %501

498:                                              ; preds = %488
  %499 = load i64, ptr @line, align 8
  %500 = call i32 (ptr, ...) @printf(ptr @.cstr.14, i64 %499)
  call void @exit(i32 -1)
  br label %501

501:                                              ; preds = %498, %497
  %502 = load ptr, ptr %5, align 8
  %503 = load ptr, ptr @text, align 8
  %504 = getelementptr inbounds i64, ptr %503, i32 3
  %505 = ptrtoint ptr %504 to i64
  store i64 %505, ptr %502, align 8
  %506 = load ptr, ptr @text, align 8
  %507 = getelementptr inbounds i64, ptr %506, i64 1
  store ptr %507, ptr @text, align 8
  store i64 2, ptr %507, align 8
  %508 = load ptr, ptr @text, align 8
  %509 = getelementptr inbounds i64, ptr %508, i64 1
  store ptr %509, ptr @text, align 8
  store ptr %509, ptr %5, align 8
  call void @expression(i64 143)
  %510 = load ptr, ptr %5, align 8
  %511 = load ptr, ptr @text, align 8
  %512 = getelementptr inbounds i64, ptr %511, i32 1
  %513 = ptrtoint ptr %512 to i64
  store i64 %513, ptr %510, align 8
  br label %911

514:                                              ; preds = %483
  %515 = load i64, ptr @token, align 8
  %516 = icmp eq i64 %515, 144
  %517 = zext i1 %516 to i32
  %518 = icmp ne i32 %517, 0
  br i1 %518, label %519, label %528

519:                                              ; preds = %514
  call void @match(i64 144)
  %520 = load ptr, ptr @text, align 8
  %521 = getelementptr inbounds i64, ptr %520, i64 1
  store ptr %521, ptr @text, align 8
  store i64 5, ptr %521, align 8
  %522 = load ptr, ptr @text, align 8
  %523 = getelementptr inbounds i64, ptr %522, i64 1
  store ptr %523, ptr @text, align 8
  store ptr %523, ptr %5, align 8
  call void @expression(i64 145)
  %524 = load ptr, ptr %5, align 8
  %525 = load ptr, ptr @text, align 8
  %526 = getelementptr inbounds i64, ptr %525, i32 1
  %527 = ptrtoint ptr %526 to i64
  store i64 %527, ptr %524, align 8
  store i64 1, ptr @expr_type, align 8
  br label %910

528:                                              ; preds = %514
  %529 = load i64, ptr @token, align 8
  %530 = icmp eq i64 %529, 145
  %531 = zext i1 %530 to i32
  %532 = icmp ne i32 %531, 0
  br i1 %532, label %533, label %542

533:                                              ; preds = %528
  call void @match(i64 145)
  %534 = load ptr, ptr @text, align 8
  %535 = getelementptr inbounds i64, ptr %534, i64 1
  store ptr %535, ptr @text, align 8
  store i64 4, ptr %535, align 8
  %536 = load ptr, ptr @text, align 8
  %537 = getelementptr inbounds i64, ptr %536, i64 1
  store ptr %537, ptr @text, align 8
  store ptr %537, ptr %5, align 8
  call void @expression(i64 146)
  %538 = load ptr, ptr %5, align 8
  %539 = load ptr, ptr @text, align 8
  %540 = getelementptr inbounds i64, ptr %539, i32 1
  %541 = ptrtoint ptr %540 to i64
  store i64 %541, ptr %538, align 8
  store i64 1, ptr @expr_type, align 8
  br label %909

542:                                              ; preds = %528
  %543 = load i64, ptr @token, align 8
  %544 = icmp eq i64 %543, 146
  %545 = zext i1 %544 to i32
  %546 = icmp ne i32 %545, 0
  br i1 %546, label %547, label %552

547:                                              ; preds = %542
  call void @match(i64 146)
  %548 = load ptr, ptr @text, align 8
  %549 = getelementptr inbounds i64, ptr %548, i64 1
  store ptr %549, ptr @text, align 8
  store i64 13, ptr %549, align 8
  call void @expression(i64 147)
  %550 = load ptr, ptr @text, align 8
  %551 = getelementptr inbounds i64, ptr %550, i64 1
  store ptr %551, ptr @text, align 8
  store i64 14, ptr %551, align 8
  store i64 1, ptr @expr_type, align 8
  br label %908

552:                                              ; preds = %542
  %553 = load i64, ptr @token, align 8
  %554 = icmp eq i64 %553, 147
  %555 = zext i1 %554 to i32
  %556 = icmp ne i32 %555, 0
  br i1 %556, label %557, label %562

557:                                              ; preds = %552
  call void @match(i64 147)
  %558 = load ptr, ptr @text, align 8
  %559 = getelementptr inbounds i64, ptr %558, i64 1
  store ptr %559, ptr @text, align 8
  store i64 13, ptr %559, align 8
  call void @expression(i64 148)
  %560 = load ptr, ptr @text, align 8
  %561 = getelementptr inbounds i64, ptr %560, i64 1
  store ptr %561, ptr @text, align 8
  store i64 15, ptr %561, align 8
  store i64 1, ptr @expr_type, align 8
  br label %907

562:                                              ; preds = %552
  %563 = load i64, ptr @token, align 8
  %564 = icmp eq i64 %563, 148
  %565 = zext i1 %564 to i32
  %566 = icmp ne i32 %565, 0
  br i1 %566, label %567, label %572

567:                                              ; preds = %562
  call void @match(i64 148)
  %568 = load ptr, ptr @text, align 8
  %569 = getelementptr inbounds i64, ptr %568, i64 1
  store ptr %569, ptr @text, align 8
  store i64 13, ptr %569, align 8
  call void @expression(i64 149)
  %570 = load ptr, ptr @text, align 8
  %571 = getelementptr inbounds i64, ptr %570, i64 1
  store ptr %571, ptr @text, align 8
  store i64 16, ptr %571, align 8
  store i64 1, ptr @expr_type, align 8
  br label %906

572:                                              ; preds = %562
  %573 = load i64, ptr @token, align 8
  %574 = icmp eq i64 %573, 149
  %575 = zext i1 %574 to i32
  %576 = icmp ne i32 %575, 0
  br i1 %576, label %577, label %582

577:                                              ; preds = %572
  call void @match(i64 149)
  %578 = load ptr, ptr @text, align 8
  %579 = getelementptr inbounds i64, ptr %578, i64 1
  store ptr %579, ptr @text, align 8
  store i64 13, ptr %579, align 8
  call void @expression(i64 150)
  %580 = load ptr, ptr @text, align 8
  %581 = getelementptr inbounds i64, ptr %580, i64 1
  store ptr %581, ptr @text, align 8
  store i64 17, ptr %581, align 8
  store i64 1, ptr @expr_type, align 8
  br label %905

582:                                              ; preds = %572
  %583 = load i64, ptr @token, align 8
  %584 = icmp eq i64 %583, 150
  %585 = zext i1 %584 to i32
  %586 = icmp ne i32 %585, 0
  br i1 %586, label %587, label %592

587:                                              ; preds = %582
  call void @match(i64 150)
  %588 = load ptr, ptr @text, align 8
  %589 = getelementptr inbounds i64, ptr %588, i64 1
  store ptr %589, ptr @text, align 8
  store i64 13, ptr %589, align 8
  call void @expression(i64 151)
  %590 = load ptr, ptr @text, align 8
  %591 = getelementptr inbounds i64, ptr %590, i64 1
  store ptr %591, ptr @text, align 8
  store i64 18, ptr %591, align 8
  store i64 1, ptr @expr_type, align 8
  br label %904

592:                                              ; preds = %582
  %593 = load i64, ptr @token, align 8
  %594 = icmp eq i64 %593, 151
  %595 = zext i1 %594 to i32
  %596 = icmp ne i32 %595, 0
  br i1 %596, label %597, label %602

597:                                              ; preds = %592
  call void @match(i64 151)
  %598 = load ptr, ptr @text, align 8
  %599 = getelementptr inbounds i64, ptr %598, i64 1
  store ptr %599, ptr @text, align 8
  store i64 13, ptr %599, align 8
  call void @expression(i64 155)
  %600 = load ptr, ptr @text, align 8
  %601 = getelementptr inbounds i64, ptr %600, i64 1
  store ptr %601, ptr @text, align 8
  store i64 19, ptr %601, align 8
  store i64 1, ptr @expr_type, align 8
  br label %903

602:                                              ; preds = %592
  %603 = load i64, ptr @token, align 8
  %604 = icmp eq i64 %603, 152
  %605 = zext i1 %604 to i32
  %606 = icmp ne i32 %605, 0
  br i1 %606, label %607, label %612

607:                                              ; preds = %602
  call void @match(i64 152)
  %608 = load ptr, ptr @text, align 8
  %609 = getelementptr inbounds i64, ptr %608, i64 1
  store ptr %609, ptr @text, align 8
  store i64 13, ptr %609, align 8
  call void @expression(i64 155)
  %610 = load ptr, ptr @text, align 8
  %611 = getelementptr inbounds i64, ptr %610, i64 1
  store ptr %611, ptr @text, align 8
  store i64 20, ptr %611, align 8
  store i64 1, ptr @expr_type, align 8
  br label %902

612:                                              ; preds = %602
  %613 = load i64, ptr @token, align 8
  %614 = icmp eq i64 %613, 153
  %615 = zext i1 %614 to i32
  %616 = icmp ne i32 %615, 0
  br i1 %616, label %617, label %622

617:                                              ; preds = %612
  call void @match(i64 153)
  %618 = load ptr, ptr @text, align 8
  %619 = getelementptr inbounds i64, ptr %618, i64 1
  store ptr %619, ptr @text, align 8
  store i64 13, ptr %619, align 8
  call void @expression(i64 155)
  %620 = load ptr, ptr @text, align 8
  %621 = getelementptr inbounds i64, ptr %620, i64 1
  store ptr %621, ptr @text, align 8
  store i64 21, ptr %621, align 8
  store i64 1, ptr @expr_type, align 8
  br label %901

622:                                              ; preds = %612
  %623 = load i64, ptr @token, align 8
  %624 = icmp eq i64 %623, 154
  %625 = zext i1 %624 to i32
  %626 = icmp ne i32 %625, 0
  br i1 %626, label %627, label %632

627:                                              ; preds = %622
  call void @match(i64 154)
  %628 = load ptr, ptr @text, align 8
  %629 = getelementptr inbounds i64, ptr %628, i64 1
  store ptr %629, ptr @text, align 8
  store i64 13, ptr %629, align 8
  call void @expression(i64 155)
  %630 = load ptr, ptr @text, align 8
  %631 = getelementptr inbounds i64, ptr %630, i64 1
  store ptr %631, ptr @text, align 8
  store i64 22, ptr %631, align 8
  store i64 1, ptr @expr_type, align 8
  br label %900

632:                                              ; preds = %622
  %633 = load i64, ptr @token, align 8
  %634 = icmp eq i64 %633, 155
  %635 = zext i1 %634 to i32
  %636 = icmp ne i32 %635, 0
  br i1 %636, label %637, label %642

637:                                              ; preds = %632
  call void @match(i64 155)
  %638 = load ptr, ptr @text, align 8
  %639 = getelementptr inbounds i64, ptr %638, i64 1
  store ptr %639, ptr @text, align 8
  store i64 13, ptr %639, align 8
  call void @expression(i64 157)
  %640 = load ptr, ptr @text, align 8
  %641 = getelementptr inbounds i64, ptr %640, i64 1
  store ptr %641, ptr @text, align 8
  store i64 23, ptr %641, align 8
  store i64 1, ptr @expr_type, align 8
  br label %899

642:                                              ; preds = %632
  %643 = load i64, ptr @token, align 8
  %644 = icmp eq i64 %643, 156
  %645 = zext i1 %644 to i32
  %646 = icmp ne i32 %645, 0
  br i1 %646, label %647, label %652

647:                                              ; preds = %642
  call void @match(i64 156)
  %648 = load ptr, ptr @text, align 8
  %649 = getelementptr inbounds i64, ptr %648, i64 1
  store ptr %649, ptr @text, align 8
  store i64 13, ptr %649, align 8
  call void @expression(i64 157)
  %650 = load ptr, ptr @text, align 8
  %651 = getelementptr inbounds i64, ptr %650, i64 1
  store ptr %651, ptr @text, align 8
  store i64 24, ptr %651, align 8
  store i64 1, ptr @expr_type, align 8
  br label %898

652:                                              ; preds = %642
  %653 = load i64, ptr @token, align 8
  %654 = icmp eq i64 %653, 157
  %655 = zext i1 %654 to i32
  %656 = icmp ne i32 %655, 0
  br i1 %656, label %657, label %677

657:                                              ; preds = %652
  call void @match(i64 157)
  %658 = load ptr, ptr @text, align 8
  %659 = getelementptr inbounds i64, ptr %658, i64 1
  store ptr %659, ptr @text, align 8
  store i64 13, ptr %659, align 8
  call void @expression(i64 159)
  %660 = load i64, ptr %4, align 8
  store i64 %660, ptr @expr_type, align 8
  %661 = load i64, ptr @expr_type, align 8
  %662 = icmp sgt i64 %661, 2
  %663 = zext i1 %662 to i32
  %664 = icmp ne i32 %663, 0
  br i1 %664, label %665, label %674

665:                                              ; preds = %657
  %666 = load ptr, ptr @text, align 8
  %667 = getelementptr inbounds i64, ptr %666, i64 1
  store ptr %667, ptr @text, align 8
  store i64 13, ptr %667, align 8
  %668 = load ptr, ptr @text, align 8
  %669 = getelementptr inbounds i64, ptr %668, i64 1
  store ptr %669, ptr @text, align 8
  store i64 1, ptr %669, align 8
  %670 = load ptr, ptr @text, align 8
  %671 = getelementptr inbounds i64, ptr %670, i64 1
  store ptr %671, ptr @text, align 8
  store i64 8, ptr %671, align 8
  %672 = load ptr, ptr @text, align 8
  %673 = getelementptr inbounds i64, ptr %672, i64 1
  store ptr %673, ptr @text, align 8
  store i64 27, ptr %673, align 8
  br label %674

674:                                              ; preds = %665, %657
  %675 = load ptr, ptr @text, align 8
  %676 = getelementptr inbounds i64, ptr %675, i64 1
  store ptr %676, ptr @text, align 8
  store i64 25, ptr %676, align 8
  br label %897

677:                                              ; preds = %652
  %678 = load i64, ptr @token, align 8
  %679 = icmp eq i64 %678, 158
  %680 = zext i1 %679 to i32
  %681 = icmp ne i32 %680, 0
  br i1 %681, label %682, label %729

682:                                              ; preds = %677
  call void @match(i64 158)
  %683 = load ptr, ptr @text, align 8
  %684 = getelementptr inbounds i64, ptr %683, i64 1
  store ptr %684, ptr @text, align 8
  store i64 13, ptr %684, align 8
  call void @expression(i64 159)
  %685 = load i64, ptr %4, align 8
  %686 = icmp sgt i64 %685, 2
  br i1 %686, label %687, label %691

687:                                              ; preds = %682
  %688 = load i64, ptr %4, align 8
  %689 = load i64, ptr @expr_type, align 8
  %690 = icmp eq i64 %688, %689
  br label %691

691:                                              ; preds = %687, %682
  %692 = phi i1 [ %686, %682 ], [ %690, %687 ]
  %693 = zext i1 %692 to i32
  %694 = icmp ne i32 %693, 0
  br i1 %694, label %695, label %706

695:                                              ; preds = %691
  %696 = load ptr, ptr @text, align 8
  %697 = getelementptr inbounds i64, ptr %696, i64 1
  store ptr %697, ptr @text, align 8
  store i64 26, ptr %697, align 8
  %698 = load ptr, ptr @text, align 8
  %699 = getelementptr inbounds i64, ptr %698, i64 1
  store ptr %699, ptr @text, align 8
  store i64 13, ptr %699, align 8
  %700 = load ptr, ptr @text, align 8
  %701 = getelementptr inbounds i64, ptr %700, i64 1
  store ptr %701, ptr @text, align 8
  store i64 1, ptr %701, align 8
  %702 = load ptr, ptr @text, align 8
  %703 = getelementptr inbounds i64, ptr %702, i64 1
  store ptr %703, ptr @text, align 8
  store i64 8, ptr %703, align 8
  %704 = load ptr, ptr @text, align 8
  %705 = getelementptr inbounds i64, ptr %704, i64 1
  store ptr %705, ptr @text, align 8
  store i64 28, ptr %705, align 8
  store i64 1, ptr @expr_type, align 8
  br label %728

706:                                              ; preds = %691
  %707 = load i64, ptr %4, align 8
  %708 = icmp sgt i64 %707, 2
  %709 = zext i1 %708 to i32
  %710 = icmp ne i32 %709, 0
  br i1 %710, label %711, label %723

711:                                              ; preds = %706
  %712 = load ptr, ptr @text, align 8
  %713 = getelementptr inbounds i64, ptr %712, i64 1
  store ptr %713, ptr @text, align 8
  store i64 13, ptr %713, align 8
  %714 = load ptr, ptr @text, align 8
  %715 = getelementptr inbounds i64, ptr %714, i64 1
  store ptr %715, ptr @text, align 8
  store i64 1, ptr %715, align 8
  %716 = load ptr, ptr @text, align 8
  %717 = getelementptr inbounds i64, ptr %716, i64 1
  store ptr %717, ptr @text, align 8
  store i64 8, ptr %717, align 8
  %718 = load ptr, ptr @text, align 8
  %719 = getelementptr inbounds i64, ptr %718, i64 1
  store ptr %719, ptr @text, align 8
  store i64 27, ptr %719, align 8
  %720 = load ptr, ptr @text, align 8
  %721 = getelementptr inbounds i64, ptr %720, i64 1
  store ptr %721, ptr @text, align 8
  store i64 26, ptr %721, align 8
  %722 = load i64, ptr %4, align 8
  store i64 %722, ptr @expr_type, align 8
  br label %727

723:                                              ; preds = %706
  %724 = load ptr, ptr @text, align 8
  %725 = getelementptr inbounds i64, ptr %724, i64 1
  store ptr %725, ptr @text, align 8
  store i64 26, ptr %725, align 8
  %726 = load i64, ptr %4, align 8
  store i64 %726, ptr @expr_type, align 8
  br label %727

727:                                              ; preds = %723, %711
  br label %728

728:                                              ; preds = %727, %695
  br label %896

729:                                              ; preds = %677
  %730 = load i64, ptr @token, align 8
  %731 = icmp eq i64 %730, 159
  %732 = zext i1 %731 to i32
  %733 = icmp ne i32 %732, 0
  br i1 %733, label %734, label %740

734:                                              ; preds = %729
  call void @match(i64 159)
  %735 = load ptr, ptr @text, align 8
  %736 = getelementptr inbounds i64, ptr %735, i64 1
  store ptr %736, ptr @text, align 8
  store i64 13, ptr %736, align 8
  call void @expression(i64 162)
  %737 = load ptr, ptr @text, align 8
  %738 = getelementptr inbounds i64, ptr %737, i64 1
  store ptr %738, ptr @text, align 8
  store i64 27, ptr %738, align 8
  %739 = load i64, ptr %4, align 8
  store i64 %739, ptr @expr_type, align 8
  br label %895

740:                                              ; preds = %729
  %741 = load i64, ptr @token, align 8
  %742 = icmp eq i64 %741, 160
  %743 = zext i1 %742 to i32
  %744 = icmp ne i32 %743, 0
  br i1 %744, label %745, label %751

745:                                              ; preds = %740
  call void @match(i64 160)
  %746 = load ptr, ptr @text, align 8
  %747 = getelementptr inbounds i64, ptr %746, i64 1
  store ptr %747, ptr @text, align 8
  store i64 13, ptr %747, align 8
  call void @expression(i64 162)
  %748 = load ptr, ptr @text, align 8
  %749 = getelementptr inbounds i64, ptr %748, i64 1
  store ptr %749, ptr @text, align 8
  store i64 28, ptr %749, align 8
  %750 = load i64, ptr %4, align 8
  store i64 %750, ptr @expr_type, align 8
  br label %894

751:                                              ; preds = %740
  %752 = load i64, ptr @token, align 8
  %753 = icmp eq i64 %752, 161
  %754 = zext i1 %753 to i32
  %755 = icmp ne i32 %754, 0
  br i1 %755, label %756, label %762

756:                                              ; preds = %751
  call void @match(i64 161)
  %757 = load ptr, ptr @text, align 8
  %758 = getelementptr inbounds i64, ptr %757, i64 1
  store ptr %758, ptr @text, align 8
  store i64 13, ptr %758, align 8
  call void @expression(i64 162)
  %759 = load ptr, ptr @text, align 8
  %760 = getelementptr inbounds i64, ptr %759, i64 1
  store ptr %760, ptr @text, align 8
  store i64 29, ptr %760, align 8
  %761 = load i64, ptr %4, align 8
  store i64 %761, ptr @expr_type, align 8
  br label %893

762:                                              ; preds = %751
  %763 = load i64, ptr @token, align 8
  %764 = icmp eq i64 %763, 163
  br i1 %764, label %768, label %765

765:                                              ; preds = %762
  %766 = load i64, ptr @token, align 8
  %767 = icmp eq i64 %766, 162
  br label %768

768:                                              ; preds = %765, %762
  %769 = phi i1 [ %764, %762 ], [ %767, %765 ]
  %770 = zext i1 %769 to i32
  %771 = icmp ne i32 %770, 0
  br i1 %771, label %772, label %844

772:                                              ; preds = %768
  %773 = load ptr, ptr @text, align 8
  %774 = load i64, ptr %773, align 8
  %775 = icmp eq i64 %774, 9
  %776 = zext i1 %775 to i32
  %777 = icmp ne i32 %776, 0
  br i1 %777, label %778, label %782

778:                                              ; preds = %772
  %779 = load ptr, ptr @text, align 8
  store i64 13, ptr %779, align 8
  %780 = load ptr, ptr @text, align 8
  %781 = getelementptr inbounds i64, ptr %780, i64 1
  store ptr %781, ptr @text, align 8
  store i64 9, ptr %781, align 8
  br label %796

782:                                              ; preds = %772
  %783 = load ptr, ptr @text, align 8
  %784 = load i64, ptr %783, align 8
  %785 = icmp eq i64 %784, 10
  %786 = zext i1 %785 to i32
  %787 = icmp ne i32 %786, 0
  br i1 %787, label %788, label %792

788:                                              ; preds = %782
  %789 = load ptr, ptr @text, align 8
  store i64 13, ptr %789, align 8
  %790 = load ptr, ptr @text, align 8
  %791 = getelementptr inbounds i64, ptr %790, i64 1
  store ptr %791, ptr @text, align 8
  store i64 10, ptr %791, align 8
  br label %795

792:                                              ; preds = %782
  %793 = load i64, ptr @line, align 8
  %794 = call i32 (ptr, ...) @printf(ptr @.cstr.15, i64 %793)
  call void @exit(i32 -1)
  br label %795

795:                                              ; preds = %792, %788
  br label %796

796:                                              ; preds = %795, %778
  %797 = load ptr, ptr @text, align 8
  %798 = getelementptr inbounds i64, ptr %797, i64 1
  store ptr %798, ptr @text, align 8
  store i64 13, ptr %798, align 8
  %799 = load ptr, ptr @text, align 8
  %800 = getelementptr inbounds i64, ptr %799, i64 1
  store ptr %800, ptr @text, align 8
  store i64 1, ptr %800, align 8
  %801 = load ptr, ptr @text, align 8
  %802 = getelementptr inbounds i64, ptr %801, i64 1
  store ptr %802, ptr @text, align 8
  %803 = load i64, ptr @expr_type, align 8
  %804 = icmp sgt i64 %803, 2
  %805 = zext i1 %804 to i32
  %806 = icmp ne i32 %805, 0
  %807 = select i1 %806, i64 8, i64 1
  store i64 %807, ptr %802, align 8
  %808 = load ptr, ptr @text, align 8
  %809 = getelementptr inbounds i64, ptr %808, i64 1
  store ptr %809, ptr @text, align 8
  %810 = load i64, ptr @token, align 8
  %811 = icmp eq i64 %810, 162
  %812 = zext i1 %811 to i32
  %813 = icmp ne i32 %812, 0
  %814 = select i1 %813, i32 25, i32 26
  %815 = sext i32 %814 to i64
  store i64 %815, ptr %809, align 8
  %816 = load ptr, ptr @text, align 8
  %817 = getelementptr inbounds i64, ptr %816, i64 1
  store ptr %817, ptr @text, align 8
  %818 = load i64, ptr @expr_type, align 8
  %819 = icmp eq i64 %818, 0
  %820 = zext i1 %819 to i32
  %821 = icmp ne i32 %820, 0
  %822 = select i1 %821, i32 12, i32 11
  %823 = sext i32 %822 to i64
  store i64 %823, ptr %817, align 8
  %824 = load ptr, ptr @text, align 8
  %825 = getelementptr inbounds i64, ptr %824, i64 1
  store ptr %825, ptr @text, align 8
  store i64 13, ptr %825, align 8
  %826 = load ptr, ptr @text, align 8
  %827 = getelementptr inbounds i64, ptr %826, i64 1
  store ptr %827, ptr @text, align 8
  store i64 1, ptr %827, align 8
  %828 = load ptr, ptr @text, align 8
  %829 = getelementptr inbounds i64, ptr %828, i64 1
  store ptr %829, ptr @text, align 8
  %830 = load i64, ptr @expr_type, align 8
  %831 = icmp sgt i64 %830, 2
  %832 = zext i1 %831 to i32
  %833 = icmp ne i32 %832, 0
  %834 = select i1 %833, i64 8, i64 1
  store i64 %834, ptr %829, align 8
  %835 = load ptr, ptr @text, align 8
  %836 = getelementptr inbounds i64, ptr %835, i64 1
  store ptr %836, ptr @text, align 8
  %837 = load i64, ptr @token, align 8
  %838 = icmp eq i64 %837, 162
  %839 = zext i1 %838 to i32
  %840 = icmp ne i32 %839, 0
  %841 = select i1 %840, i32 26, i32 25
  %842 = sext i32 %841 to i64
  store i64 %842, ptr %836, align 8
  %843 = load i64, ptr @token, align 8
  call void @match(i64 %843)
  br label %892

844:                                              ; preds = %768
  %845 = load i64, ptr @token, align 8
  %846 = icmp eq i64 %845, 164
  %847 = zext i1 %846 to i32
  %848 = icmp ne i32 %847, 0
  br i1 %848, label %849, label %885

849:                                              ; preds = %844
  call void @match(i64 164)
  %850 = load ptr, ptr @text, align 8
  %851 = getelementptr inbounds i64, ptr %850, i64 1
  store ptr %851, ptr @text, align 8
  store i64 13, ptr %851, align 8
  call void @expression(i64 142)
  call void @match(i64 93)
  %852 = load i64, ptr %4, align 8
  %853 = icmp sgt i64 %852, 2
  %854 = zext i1 %853 to i32
  %855 = icmp ne i32 %854, 0
  br i1 %855, label %856, label %863

856:                                              ; preds = %849
  %857 = load ptr, ptr @text, align 8
  %858 = getelementptr inbounds i64, ptr %857, i64 1
  store ptr %858, ptr @text, align 8
  store i64 1, ptr %858, align 8
  %859 = load ptr, ptr @text, align 8
  %860 = getelementptr inbounds i64, ptr %859, i64 1
  store ptr %860, ptr @text, align 8
  store i64 8, ptr %860, align 8
  %861 = load ptr, ptr @text, align 8
  %862 = getelementptr inbounds i64, ptr %861, i64 1
  store ptr %862, ptr @text, align 8
  store i64 27, ptr %862, align 8
  br label %872

863:                                              ; preds = %849
  %864 = load i64, ptr %4, align 8
  %865 = icmp slt i64 %864, 2
  %866 = zext i1 %865 to i32
  %867 = icmp ne i32 %866, 0
  br i1 %867, label %868, label %871

868:                                              ; preds = %863
  %869 = load i64, ptr @line, align 8
  %870 = call i32 (ptr, ...) @printf(ptr @.cstr.16, i64 %869)
  call void @exit(i32 -1)
  br label %871

871:                                              ; preds = %868, %863
  br label %872

872:                                              ; preds = %871, %856
  %873 = load i64, ptr %4, align 8
  %874 = sub nsw i64 %873, 2
  store i64 %874, ptr @expr_type, align 8
  %875 = load ptr, ptr @text, align 8
  %876 = getelementptr inbounds i64, ptr %875, i64 1
  store ptr %876, ptr @text, align 8
  store i64 25, ptr %876, align 8
  %877 = load ptr, ptr @text, align 8
  %878 = getelementptr inbounds i64, ptr %877, i64 1
  store ptr %878, ptr @text, align 8
  %879 = load i64, ptr @expr_type, align 8
  %880 = icmp eq i64 %879, 0
  %881 = zext i1 %880 to i32
  %882 = icmp ne i32 %881, 0
  %883 = select i1 %882, i32 10, i32 9
  %884 = sext i32 %883 to i64
  store i64 %884, ptr %878, align 8
  br label %891

885:                                              ; preds = %844
  %886 = load ptr, ptr @text, align 8
  %887 = getelementptr inbounds i64, ptr %886, i64 1
  store ptr %887, ptr @text, align 8
  store i64 13, ptr %887, align 8
  %888 = load i64, ptr @line, align 8
  %889 = load i64, ptr @token, align 8
  %890 = call i32 (ptr, ...) @printf(ptr @.cstr.17, i64 %888, i64 %889)
  call void @exit(i32 -1)
  br label %891

891:                                              ; preds = %885, %872
  br label %892

892:                                              ; preds = %891, %796
  br label %893

893:                                              ; preds = %892, %756
  br label %894

894:                                              ; preds = %893, %745
  br label %895

895:                                              ; preds = %894, %734
  br label %896

896:                                              ; preds = %895, %728
  br label %897

897:                                              ; preds = %896, %674
  br label %898

898:                                              ; preds = %897, %647
  br label %899

899:                                              ; preds = %898, %637
  br label %900

900:                                              ; preds = %899, %627
  br label %901

901:                                              ; preds = %900, %617
  br label %902

902:                                              ; preds = %901, %607
  br label %903

903:                                              ; preds = %902, %597
  br label %904

904:                                              ; preds = %903, %587
  br label %905

905:                                              ; preds = %904, %577
  br label %906

906:                                              ; preds = %905, %567
  br label %907

907:                                              ; preds = %906, %557
  br label %908

908:                                              ; preds = %907, %547
  br label %909

909:                                              ; preds = %908, %533
  br label %910

910:                                              ; preds = %909, %519
  br label %911

911:                                              ; preds = %910, %501
  br label %912

912:                                              ; preds = %911, %473
  br label %913

913:                                              ; preds = %912, %449
  %914 = load i64, ptr @token, align 8
  %915 = load i64, ptr %2, align 8
  %916 = icmp sge i64 %914, %915
  %917 = zext i1 %916 to i32
  %918 = icmp ne i32 %917, 0
  br i1 %918, label %450, label %919

919:                                              ; preds = %913
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
  br i1 %6, label %7, label %30

7:                                                ; preds = %0
  call void @match(i64 137)
  call void @match(i64 40)
  call void @expression(i64 142)
  call void @match(i64 41)
  %8 = load ptr, ptr @text, align 8
  %9 = getelementptr inbounds i64, ptr %8, i64 1
  store ptr %9, ptr @text, align 8
  store i64 4, ptr %9, align 8
  %10 = load ptr, ptr @text, align 8
  %11 = getelementptr inbounds i64, ptr %10, i64 1
  store ptr %11, ptr @text, align 8
  store ptr %11, ptr %2, align 8
  call void (...) @statement()
  %12 = load i64, ptr @token, align 8
  %13 = icmp eq i64 %12, 135
  %14 = zext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %16, label %25

16:                                               ; preds = %7
  call void @match(i64 135)
  %17 = load ptr, ptr %2, align 8
  %18 = load ptr, ptr @text, align 8
  %19 = getelementptr inbounds i64, ptr %18, i32 3
  %20 = ptrtoint ptr %19 to i64
  store i64 %20, ptr %17, align 8
  %21 = load ptr, ptr @text, align 8
  %22 = getelementptr inbounds i64, ptr %21, i64 1
  store ptr %22, ptr @text, align 8
  store i64 2, ptr %22, align 8
  %23 = load ptr, ptr @text, align 8
  %24 = getelementptr inbounds i64, ptr %23, i64 1
  store ptr %24, ptr @text, align 8
  store ptr %24, ptr %2, align 8
  call void (...) @statement()
  br label %25

25:                                               ; preds = %16, %7
  %26 = load ptr, ptr %2, align 8
  %27 = load ptr, ptr @text, align 8
  %28 = getelementptr inbounds i64, ptr %27, i32 1
  %29 = ptrtoint ptr %28 to i64
  store i64 %29, ptr %26, align 8
  br label %90

30:                                               ; preds = %0
  %31 = load i64, ptr @token, align 8
  %32 = icmp eq i64 %31, 141
  %33 = zext i1 %32 to i32
  %34 = icmp ne i32 %33, 0
  br i1 %34, label %35, label %52

35:                                               ; preds = %30
  call void @match(i64 141)
  %36 = load ptr, ptr @text, align 8
  %37 = getelementptr inbounds i64, ptr %36, i32 1
  store ptr %37, ptr %1, align 8
  call void @match(i64 40)
  call void @expression(i64 142)
  call void @match(i64 41)
  %38 = load ptr, ptr @text, align 8
  %39 = getelementptr inbounds i64, ptr %38, i64 1
  store ptr %39, ptr @text, align 8
  store i64 4, ptr %39, align 8
  %40 = load ptr, ptr @text, align 8
  %41 = getelementptr inbounds i64, ptr %40, i64 1
  store ptr %41, ptr @text, align 8
  store ptr %41, ptr %2, align 8
  call void (...) @statement()
  %42 = load ptr, ptr @text, align 8
  %43 = getelementptr inbounds i64, ptr %42, i64 1
  store ptr %43, ptr @text, align 8
  store i64 2, ptr %43, align 8
  %44 = load ptr, ptr @text, align 8
  %45 = getelementptr inbounds i64, ptr %44, i64 1
  store ptr %45, ptr @text, align 8
  %46 = load ptr, ptr %1, align 8
  %47 = ptrtoint ptr %46 to i64
  store i64 %47, ptr %45, align 8
  %48 = load ptr, ptr %2, align 8
  %49 = load ptr, ptr @text, align 8
  %50 = getelementptr inbounds i64, ptr %49, i32 1
  %51 = ptrtoint ptr %50 to i64
  store i64 %51, ptr %48, align 8
  br label %89

52:                                               ; preds = %30
  %53 = load i64, ptr @token, align 8
  %54 = icmp eq i64 %53, 123
  %55 = zext i1 %54 to i32
  %56 = icmp ne i32 %55, 0
  br i1 %56, label %57, label %65

57:                                               ; preds = %52
  call void @match(i64 123)
  br label %59

58:                                               ; preds = %59
  call void (...) @statement()
  br label %59

59:                                               ; preds = %58, %57
  %60 = load i64, ptr @token, align 8
  %61 = icmp ne i64 %60, 125
  %62 = zext i1 %61 to i32
  %63 = icmp ne i32 %62, 0
  br i1 %63, label %58, label %64

64:                                               ; preds = %59
  call void @match(i64 125)
  br label %88

65:                                               ; preds = %52
  %66 = load i64, ptr @token, align 8
  %67 = icmp eq i64 %66, 139
  %68 = zext i1 %67 to i32
  %69 = icmp ne i32 %68, 0
  br i1 %69, label %70, label %79

70:                                               ; preds = %65
  call void @match(i64 139)
  %71 = load i64, ptr @token, align 8
  %72 = icmp ne i64 %71, 59
  %73 = zext i1 %72 to i32
  %74 = icmp ne i32 %73, 0
  br i1 %74, label %75, label %76

75:                                               ; preds = %70
  call void @expression(i64 142)
  br label %76

76:                                               ; preds = %75, %70
  call void @match(i64 59)
  %77 = load ptr, ptr @text, align 8
  %78 = getelementptr inbounds i64, ptr %77, i64 1
  store ptr %78, ptr @text, align 8
  store i64 8, ptr %78, align 8
  br label %87

79:                                               ; preds = %65
  %80 = load i64, ptr @token, align 8
  %81 = icmp eq i64 %80, 59
  %82 = zext i1 %81 to i32
  %83 = icmp ne i32 %82, 0
  br i1 %83, label %84, label %85

84:                                               ; preds = %79
  call void @match(i64 59)
  br label %86

85:                                               ; preds = %79
  call void @expression(i64 142)
  call void @match(i64 59)
  br label %86

86:                                               ; preds = %85, %84
  br label %87

87:                                               ; preds = %86, %76
  br label %88

88:                                               ; preds = %87, %64
  br label %89

89:                                               ; preds = %88, %35
  br label %90

90:                                               ; preds = %89, %25
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @enum_declaration(...) #0 {
  %1 = alloca i64, align 8
  store i64 0, ptr %1, align 8
  br label %41

2:                                                ; preds = %41
  %3 = load i64, ptr @token, align 8
  %4 = icmp ne i64 %3, 133
  %5 = zext i1 %4 to i32
  %6 = icmp ne i32 %5, 0
  br i1 %6, label %7, label %11

7:                                                ; preds = %2
  %8 = load i64, ptr @line, align 8
  %9 = load i64, ptr @token, align 8
  %10 = call i32 (ptr, ...) @printf(ptr @.cstr.18, i64 %8, i64 %9)
  call void @exit(i32 -1)
  br label %11

11:                                               ; preds = %7, %2
  call void (...) @next()
  %12 = load i64, ptr @token, align 8
  %13 = icmp eq i64 %12, 142
  %14 = zext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %16, label %26

16:                                               ; preds = %11
  call void (...) @next()
  %17 = load i64, ptr @token, align 8
  %18 = icmp ne i64 %17, 128
  %19 = zext i1 %18 to i32
  %20 = icmp ne i32 %19, 0
  br i1 %20, label %21, label %24

21:                                               ; preds = %16
  %22 = load i64, ptr @line, align 8
  %23 = call i32 (ptr, ...) @printf(ptr @.cstr.19, i64 %22)
  call void @exit(i32 -1)
  br label %24

24:                                               ; preds = %21, %16
  %25 = load i64, ptr @token_val, align 8
  store i64 %25, ptr %1, align 8
  call void (...) @next()
  br label %26

26:                                               ; preds = %24, %11
  %27 = load ptr, ptr @current_id, align 8
  %28 = getelementptr inbounds i64, ptr %27, i32 4
  store i64 128, ptr %28, align 8
  %29 = load ptr, ptr @current_id, align 8
  %30 = getelementptr inbounds i64, ptr %29, i32 3
  store i64 1, ptr %30, align 8
  %31 = load ptr, ptr @current_id, align 8
  %32 = getelementptr inbounds i64, ptr %31, i32 5
  %33 = load i64, ptr %1, align 8
  %34 = add i64 %33, 1
  store i64 %34, ptr %1, align 8
  store i64 %33, ptr %32, align 8
  %35 = load i64, ptr @token, align 8
  %36 = icmp eq i64 %35, 44
  %37 = zext i1 %36 to i32
  %38 = icmp ne i32 %37, 0
  br i1 %38, label %39, label %40

39:                                               ; preds = %26
  call void (...) @next()
  br label %40

40:                                               ; preds = %39, %26
  br label %41

41:                                               ; preds = %40, %0
  %42 = load i64, ptr @token, align 8
  %43 = icmp ne i64 %42, 125
  %44 = zext i1 %43 to i32
  %45 = icmp ne i32 %44, 0
  br i1 %45, label %2, label %46

46:                                               ; preds = %41
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @function_parameter(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  store i64 0, ptr %2, align 8
  br label %74

3:                                                ; preds = %74
  store i64 1, ptr %1, align 8
  %4 = load i64, ptr @token, align 8
  %5 = icmp eq i64 %4, 138
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %8, label %9

8:                                                ; preds = %3
  call void @match(i64 138)
  br label %16

9:                                                ; preds = %3
  %10 = load i64, ptr @token, align 8
  %11 = icmp eq i64 %10, 134
  %12 = zext i1 %11 to i32
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %14, label %15

14:                                               ; preds = %9
  store i64 0, ptr %1, align 8
  call void @match(i64 134)
  br label %15

15:                                               ; preds = %14, %9
  br label %16

16:                                               ; preds = %15, %8
  br label %20

17:                                               ; preds = %20
  call void @match(i64 159)
  %18 = load i64, ptr %1, align 8
  %19 = add nsw i64 %18, 2
  store i64 %19, ptr %1, align 8
  br label %20

20:                                               ; preds = %17, %16
  %21 = load i64, ptr @token, align 8
  %22 = icmp eq i64 %21, 159
  %23 = zext i1 %22 to i32
  %24 = icmp ne i32 %23, 0
  br i1 %24, label %17, label %25

25:                                               ; preds = %20
  %26 = load i64, ptr @token, align 8
  %27 = icmp ne i64 %26, 133
  %28 = zext i1 %27 to i32
  %29 = icmp ne i32 %28, 0
  br i1 %29, label %30, label %33

30:                                               ; preds = %25
  %31 = load i64, ptr @line, align 8
  %32 = call i32 (ptr, ...) @printf(ptr @.cstr.20, i64 %31)
  call void @exit(i32 -1)
  br label %33

33:                                               ; preds = %30, %25
  %34 = load ptr, ptr @current_id, align 8
  %35 = getelementptr inbounds i64, ptr %34, i32 4
  %36 = load i64, ptr %35, align 8
  %37 = icmp eq i64 %36, 132
  %38 = zext i1 %37 to i32
  %39 = icmp ne i32 %38, 0
  br i1 %39, label %40, label %43

40:                                               ; preds = %33
  %41 = load i64, ptr @line, align 8
  %42 = call i32 (ptr, ...) @printf(ptr @.cstr.21, i64 %41)
  call void @exit(i32 -1)
  br label %43

43:                                               ; preds = %40, %33
  call void @match(i64 133)
  %44 = load ptr, ptr @current_id, align 8
  %45 = getelementptr inbounds i64, ptr %44, i32 7
  %46 = load ptr, ptr @current_id, align 8
  %47 = getelementptr inbounds i64, ptr %46, i32 4
  %48 = load i64, ptr %47, align 8
  store i64 %48, ptr %45, align 8
  %49 = load ptr, ptr @current_id, align 8
  %50 = getelementptr inbounds i64, ptr %49, i32 4
  store i64 132, ptr %50, align 8
  %51 = load ptr, ptr @current_id, align 8
  %52 = getelementptr inbounds i64, ptr %51, i32 6
  %53 = load ptr, ptr @current_id, align 8
  %54 = getelementptr inbounds i64, ptr %53, i32 3
  %55 = load i64, ptr %54, align 8
  store i64 %55, ptr %52, align 8
  %56 = load ptr, ptr @current_id, align 8
  %57 = getelementptr inbounds i64, ptr %56, i32 3
  %58 = load i64, ptr %1, align 8
  store i64 %58, ptr %57, align 8
  %59 = load ptr, ptr @current_id, align 8
  %60 = getelementptr inbounds i64, ptr %59, i32 8
  %61 = load ptr, ptr @current_id, align 8
  %62 = getelementptr inbounds i64, ptr %61, i32 5
  %63 = load i64, ptr %62, align 8
  store i64 %63, ptr %60, align 8
  %64 = load ptr, ptr @current_id, align 8
  %65 = getelementptr inbounds i64, ptr %64, i32 5
  %66 = load i64, ptr %2, align 8
  %67 = add i64 %66, 1
  store i64 %67, ptr %2, align 8
  store i64 %66, ptr %65, align 8
  %68 = load i64, ptr @token, align 8
  %69 = icmp eq i64 %68, 44
  %70 = zext i1 %69 to i32
  %71 = icmp ne i32 %70, 0
  br i1 %71, label %72, label %73

72:                                               ; preds = %43
  call void @match(i64 44)
  br label %73

73:                                               ; preds = %72, %43
  br label %74

74:                                               ; preds = %73, %0
  %75 = load i64, ptr @token, align 8
  %76 = icmp ne i64 %75, 41
  %77 = zext i1 %76 to i32
  %78 = icmp ne i32 %77, 0
  br i1 %78, label %3, label %79

79:                                               ; preds = %74
  %80 = load i64, ptr %2, align 8
  %81 = add nsw i64 %80, 1
  store i64 %81, ptr @index_of_bp, align 8
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @function_body(...) #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = load i64, ptr @index_of_bp, align 8
  store i64 %3, ptr %1, align 8
  br label %77

4:                                                ; preds = %83
  %5 = load i64, ptr @token, align 8
  %6 = icmp eq i64 %5, 138
  %7 = zext i1 %6 to i32
  %8 = icmp ne i32 %7, 0
  %9 = select i1 %8, i32 1, i32 0
  %10 = sext i32 %9 to i64
  store i64 %10, ptr @basetype, align 8
  %11 = load i64, ptr @token, align 8
  call void @match(i64 %11)
  br label %71

12:                                               ; preds = %71
  %13 = load i64, ptr @basetype, align 8
  store i64 %13, ptr %2, align 8
  br label %17

14:                                               ; preds = %17
  call void @match(i64 159)
  %15 = load i64, ptr %2, align 8
  %16 = add nsw i64 %15, 2
  store i64 %16, ptr %2, align 8
  br label %17

17:                                               ; preds = %14, %12
  %18 = load i64, ptr @token, align 8
  %19 = icmp eq i64 %18, 159
  %20 = zext i1 %19 to i32
  %21 = icmp ne i32 %20, 0
  br i1 %21, label %14, label %22

22:                                               ; preds = %17
  %23 = load i64, ptr @token, align 8
  %24 = icmp ne i64 %23, 133
  %25 = zext i1 %24 to i32
  %26 = icmp ne i32 %25, 0
  br i1 %26, label %27, label %30

27:                                               ; preds = %22
  %28 = load i64, ptr @line, align 8
  %29 = call i32 (ptr, ...) @printf(ptr @.cstr.22, i64 %28)
  call void @exit(i32 -1)
  br label %30

30:                                               ; preds = %27, %22
  %31 = load ptr, ptr @current_id, align 8
  %32 = getelementptr inbounds i64, ptr %31, i32 4
  %33 = load i64, ptr %32, align 8
  %34 = icmp eq i64 %33, 132
  %35 = zext i1 %34 to i32
  %36 = icmp ne i32 %35, 0
  br i1 %36, label %37, label %40

37:                                               ; preds = %30
  %38 = load i64, ptr @line, align 8
  %39 = call i32 (ptr, ...) @printf(ptr @.cstr.23, i64 %38)
  call void @exit(i32 -1)
  br label %40

40:                                               ; preds = %37, %30
  call void @match(i64 133)
  %41 = load ptr, ptr @current_id, align 8
  %42 = getelementptr inbounds i64, ptr %41, i32 7
  %43 = load ptr, ptr @current_id, align 8
  %44 = getelementptr inbounds i64, ptr %43, i32 4
  %45 = load i64, ptr %44, align 8
  store i64 %45, ptr %42, align 8
  %46 = load ptr, ptr @current_id, align 8
  %47 = getelementptr inbounds i64, ptr %46, i32 4
  store i64 132, ptr %47, align 8
  %48 = load ptr, ptr @current_id, align 8
  %49 = getelementptr inbounds i64, ptr %48, i32 6
  %50 = load ptr, ptr @current_id, align 8
  %51 = getelementptr inbounds i64, ptr %50, i32 3
  %52 = load i64, ptr %51, align 8
  store i64 %52, ptr %49, align 8
  %53 = load ptr, ptr @current_id, align 8
  %54 = getelementptr inbounds i64, ptr %53, i32 3
  %55 = load i64, ptr %2, align 8
  store i64 %55, ptr %54, align 8
  %56 = load ptr, ptr @current_id, align 8
  %57 = getelementptr inbounds i64, ptr %56, i32 8
  %58 = load ptr, ptr @current_id, align 8
  %59 = getelementptr inbounds i64, ptr %58, i32 5
  %60 = load i64, ptr %59, align 8
  store i64 %60, ptr %57, align 8
  %61 = load ptr, ptr @current_id, align 8
  %62 = getelementptr inbounds i64, ptr %61, i32 5
  %63 = load i64, ptr %1, align 8
  %64 = add nsw i64 %63, 1
  store i64 %64, ptr %1, align 8
  store i64 %64, ptr %62, align 8
  %65 = load i64, ptr @token, align 8
  %66 = icmp eq i64 %65, 44
  %67 = zext i1 %66 to i32
  %68 = icmp ne i32 %67, 0
  br i1 %68, label %69, label %70

69:                                               ; preds = %40
  call void @match(i64 44)
  br label %70

70:                                               ; preds = %69, %40
  br label %71

71:                                               ; preds = %70, %4
  %72 = load i64, ptr @token, align 8
  %73 = icmp ne i64 %72, 59
  %74 = zext i1 %73 to i32
  %75 = icmp ne i32 %74, 0
  br i1 %75, label %12, label %76

76:                                               ; preds = %71
  call void @match(i64 59)
  br label %77

77:                                               ; preds = %76, %0
  %78 = load i64, ptr @token, align 8
  %79 = icmp eq i64 %78, 134
  br i1 %79, label %83, label %80

80:                                               ; preds = %77
  %81 = load i64, ptr @token, align 8
  %82 = icmp eq i64 %81, 138
  br label %83

83:                                               ; preds = %80, %77
  %84 = phi i1 [ %79, %77 ], [ %82, %80 ]
  %85 = zext i1 %84 to i32
  %86 = icmp ne i32 %85, 0
  br i1 %86, label %4, label %87

87:                                               ; preds = %83
  %88 = load ptr, ptr @text, align 8
  %89 = getelementptr inbounds i64, ptr %88, i64 1
  store ptr %89, ptr @text, align 8
  store i64 6, ptr %89, align 8
  %90 = load ptr, ptr @text, align 8
  %91 = getelementptr inbounds i64, ptr %90, i64 1
  store ptr %91, ptr @text, align 8
  %92 = load i64, ptr %1, align 8
  %93 = load i64, ptr @index_of_bp, align 8
  %94 = sub nsw i64 %92, %93
  store i64 %94, ptr %91, align 8
  br label %96

95:                                               ; preds = %96
  call void (...) @statement()
  br label %96

96:                                               ; preds = %95, %87
  %97 = load i64, ptr @token, align 8
  %98 = icmp ne i64 %97, 125
  %99 = zext i1 %98 to i32
  %100 = icmp ne i32 %99, 0
  br i1 %100, label %95, label %101

101:                                              ; preds = %96
  %102 = load ptr, ptr @text, align 8
  %103 = getelementptr inbounds i64, ptr %102, i64 1
  store ptr %103, ptr @text, align 8
  store i64 8, ptr %103, align 8
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
  br label %28

2:                                                ; preds = %28
  %3 = load ptr, ptr @current_id, align 8
  %4 = getelementptr inbounds i64, ptr %3, i32 4
  %5 = load i64, ptr %4, align 8
  %6 = icmp eq i64 %5, 132
  %7 = zext i1 %6 to i32
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %9, label %25

9:                                                ; preds = %2
  %10 = load ptr, ptr @current_id, align 8
  %11 = getelementptr inbounds i64, ptr %10, i32 4
  %12 = load ptr, ptr @current_id, align 8
  %13 = getelementptr inbounds i64, ptr %12, i32 7
  %14 = load i64, ptr %13, align 8
  store i64 %14, ptr %11, align 8
  %15 = load ptr, ptr @current_id, align 8
  %16 = getelementptr inbounds i64, ptr %15, i32 3
  %17 = load ptr, ptr @current_id, align 8
  %18 = getelementptr inbounds i64, ptr %17, i32 6
  %19 = load i64, ptr %18, align 8
  store i64 %19, ptr %16, align 8
  %20 = load ptr, ptr @current_id, align 8
  %21 = getelementptr inbounds i64, ptr %20, i32 5
  %22 = load ptr, ptr @current_id, align 8
  %23 = getelementptr inbounds i64, ptr %22, i32 8
  %24 = load i64, ptr %23, align 8
  store i64 %24, ptr %21, align 8
  br label %25

25:                                               ; preds = %9, %2
  %26 = load ptr, ptr @current_id, align 8
  %27 = getelementptr inbounds i64, ptr %26, i32 9
  store ptr %27, ptr @current_id, align 8
  br label %28

28:                                               ; preds = %25, %0
  %29 = load ptr, ptr @current_id, align 8
  %30 = getelementptr inbounds i64, ptr %29, i32 0
  %31 = load i64, ptr %30, align 8
  %32 = icmp ne i64 %31, 0
  br i1 %32, label %2, label %33

33:                                               ; preds = %28
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
  br i1 %6, label %7, label %20

7:                                                ; preds = %0
  call void @match(i64 136)
  %8 = load i64, ptr @token, align 8
  %9 = icmp ne i64 %8, 123
  %10 = zext i1 %9 to i32
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %12, label %13

12:                                               ; preds = %7
  call void @match(i64 133)
  br label %13

13:                                               ; preds = %12, %7
  %14 = load i64, ptr @token, align 8
  %15 = icmp eq i64 %14, 123
  %16 = zext i1 %15 to i32
  %17 = icmp ne i32 %16, 0
  br i1 %17, label %18, label %19

18:                                               ; preds = %13
  call void @match(i64 123)
  call void (...) @enum_declaration()
  call void @match(i64 125)
  br label %19

19:                                               ; preds = %18, %13
  call void @match(i64 59)
  ret void

20:                                               ; preds = %0
  %21 = load i64, ptr @token, align 8
  %22 = icmp eq i64 %21, 138
  %23 = zext i1 %22 to i32
  %24 = icmp ne i32 %23, 0
  br i1 %24, label %25, label %26

25:                                               ; preds = %20
  call void @match(i64 138)
  br label %33

26:                                               ; preds = %20
  %27 = load i64, ptr @token, align 8
  %28 = icmp eq i64 %27, 134
  %29 = zext i1 %28 to i32
  %30 = icmp ne i32 %29, 0
  br i1 %30, label %31, label %32

31:                                               ; preds = %26
  call void @match(i64 134)
  store i64 0, ptr @basetype, align 8
  br label %32

32:                                               ; preds = %31, %26
  br label %33

33:                                               ; preds = %32, %25
  br label %92

34:                                               ; preds = %98
  %35 = load i64, ptr @basetype, align 8
  store i64 %35, ptr %1, align 8
  br label %39

36:                                               ; preds = %39
  call void @match(i64 159)
  %37 = load i64, ptr %1, align 8
  %38 = add nsw i64 %37, 2
  store i64 %38, ptr %1, align 8
  br label %39

39:                                               ; preds = %36, %34
  %40 = load i64, ptr @token, align 8
  %41 = icmp eq i64 %40, 159
  %42 = zext i1 %41 to i32
  %43 = icmp ne i32 %42, 0
  br i1 %43, label %36, label %44

44:                                               ; preds = %39
  %45 = load i64, ptr @token, align 8
  %46 = icmp ne i64 %45, 133
  %47 = zext i1 %46 to i32
  %48 = icmp ne i32 %47, 0
  br i1 %48, label %49, label %52

49:                                               ; preds = %44
  %50 = load i64, ptr @line, align 8
  %51 = call i32 (ptr, ...) @printf(ptr @.cstr.24, i64 %50)
  call void @exit(i32 -1)
  br label %52

52:                                               ; preds = %49, %44
  %53 = load ptr, ptr @current_id, align 8
  %54 = getelementptr inbounds i64, ptr %53, i32 4
  %55 = load i64, ptr %54, align 8
  %56 = icmp ne i64 %55, 0
  br i1 %56, label %57, label %60

57:                                               ; preds = %52
  %58 = load i64, ptr @line, align 8
  %59 = call i32 (ptr, ...) @printf(ptr @.cstr.25, i64 %58)
  call void @exit(i32 -1)
  br label %60

60:                                               ; preds = %57, %52
  call void @match(i64 133)
  %61 = load ptr, ptr @current_id, align 8
  %62 = getelementptr inbounds i64, ptr %61, i32 3
  %63 = load i64, ptr %1, align 8
  store i64 %63, ptr %62, align 8
  %64 = load i64, ptr @token, align 8
  %65 = icmp eq i64 %64, 40
  %66 = zext i1 %65 to i32
  %67 = icmp ne i32 %66, 0
  br i1 %67, label %68, label %76

68:                                               ; preds = %60
  %69 = load ptr, ptr @current_id, align 8
  %70 = getelementptr inbounds i64, ptr %69, i32 4
  store i64 129, ptr %70, align 8
  %71 = load ptr, ptr @current_id, align 8
  %72 = getelementptr inbounds i64, ptr %71, i32 5
  %73 = load ptr, ptr @text, align 8
  %74 = getelementptr inbounds i64, ptr %73, i32 1
  %75 = ptrtoint ptr %74 to i64
  store i64 %75, ptr %72, align 8
  call void (...) @function_declaration()
  br label %85

76:                                               ; preds = %60
  %77 = load ptr, ptr @current_id, align 8
  %78 = getelementptr inbounds i64, ptr %77, i32 4
  store i64 131, ptr %78, align 8
  %79 = load ptr, ptr @current_id, align 8
  %80 = getelementptr inbounds i64, ptr %79, i32 5
  %81 = load ptr, ptr @data, align 8
  %82 = ptrtoint ptr %81 to i64
  store i64 %82, ptr %80, align 8
  %83 = load ptr, ptr @data, align 8
  %84 = getelementptr inbounds i8, ptr %83, i64 8
  store ptr %84, ptr @data, align 8
  br label %85

85:                                               ; preds = %76, %68
  %86 = load i64, ptr @token, align 8
  %87 = icmp eq i64 %86, 44
  %88 = zext i1 %87 to i32
  %89 = icmp ne i32 %88, 0
  br i1 %89, label %90, label %91

90:                                               ; preds = %85
  call void @match(i64 44)
  br label %91

91:                                               ; preds = %90, %85
  br label %92

92:                                               ; preds = %91, %33
  %93 = load i64, ptr @token, align 8
  %94 = icmp ne i64 %93, 59
  br i1 %94, label %95, label %98

95:                                               ; preds = %92
  %96 = load i64, ptr @token, align 8
  %97 = icmp ne i64 %96, 125
  br label %98

98:                                               ; preds = %95, %92
  %99 = phi i1 [ %94, %92 ], [ %97, %95 ]
  %100 = zext i1 %99 to i32
  %101 = icmp ne i32 %100, 0
  br i1 %101, label %34, label %102

102:                                              ; preds = %98
  call void (...) @next()
  ret void
}

; Function Attrs: nounwind optsize
define dso_local void @program(...) #0 {
  call void (...) @next()
  br label %2

1:                                                ; preds = %2
  call void (...) @global_declaration()
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
  br label %563

3:                                                ; preds = %563
  %4 = load i64, ptr @cycle, align 8
  %5 = add i64 %4, 1
  store i64 %5, ptr @cycle, align 8
  %6 = load ptr, ptr @pc, align 8
  %7 = getelementptr inbounds i64, ptr %6, i16 1
  store ptr %7, ptr @pc, align 8
  %8 = load i64, ptr %6, align 8
  store i64 %8, ptr %1, align 8
  %9 = load i64, ptr @debug, align 8
  %10 = icmp ne i64 %9, 0
  br i1 %10, label %11, label %28

11:                                               ; preds = %3
  %12 = load i64, ptr @cycle, align 8
  %13 = load i64, ptr %1, align 8
  %14 = mul nsw i64 %13, 5
  %15 = getelementptr inbounds i8, ptr @.cstr.2, i64 %14
  %16 = call i32 (ptr, ...) @printf(ptr @.cstr.26, i64 %12, ptr %15)
  %17 = load i64, ptr %1, align 8
  %18 = icmp sle i64 %17, 7
  %19 = zext i1 %18 to i32
  %20 = icmp ne i32 %19, 0
  br i1 %20, label %21, label %25

21:                                               ; preds = %11
  %22 = load ptr, ptr @pc, align 8
  %23 = load i64, ptr %22, align 8
  %24 = call i32 (ptr, ...) @printf(ptr @.cstr.27, i64 %23)
  br label %27

25:                                               ; preds = %11
  %26 = call i32 (ptr, ...) @printf(ptr @.cstr.28)
  br label %27

27:                                               ; preds = %25, %21
  br label %28

28:                                               ; preds = %27, %3
  %29 = load i64, ptr %1, align 8
  %30 = icmp eq i64 %29, 1
  %31 = zext i1 %30 to i32
  %32 = icmp ne i32 %31, 0
  br i1 %32, label %33, label %37

33:                                               ; preds = %28
  %34 = load ptr, ptr @pc, align 8
  %35 = getelementptr inbounds i64, ptr %34, i16 1
  store ptr %35, ptr @pc, align 8
  %36 = load i64, ptr %34, align 8
  store i64 %36, ptr @ax, align 8
  br label %562

37:                                               ; preds = %28
  %38 = load i64, ptr %1, align 8
  %39 = icmp eq i64 %38, 10
  %40 = zext i1 %39 to i32
  %41 = icmp ne i32 %40, 0
  br i1 %41, label %42, label %47

42:                                               ; preds = %37
  %43 = load i64, ptr @ax, align 8
  %44 = inttoptr i64 %43 to ptr
  %45 = load i8, ptr %44, align 1
  %46 = sext i8 %45 to i64
  store i64 %46, ptr @ax, align 8
  br label %561

47:                                               ; preds = %37
  %48 = load i64, ptr %1, align 8
  %49 = icmp eq i64 %48, 9
  %50 = zext i1 %49 to i32
  %51 = icmp ne i32 %50, 0
  br i1 %51, label %52, label %56

52:                                               ; preds = %47
  %53 = load i64, ptr @ax, align 8
  %54 = inttoptr i64 %53 to ptr
  %55 = load i64, ptr %54, align 8
  store i64 %55, ptr @ax, align 8
  br label %560

56:                                               ; preds = %47
  %57 = load i64, ptr %1, align 8
  %58 = icmp eq i64 %57, 12
  %59 = zext i1 %58 to i32
  %60 = icmp ne i32 %59, 0
  br i1 %60, label %61, label %69

61:                                               ; preds = %56
  %62 = load ptr, ptr @sp, align 8
  %63 = getelementptr inbounds i64, ptr %62, i16 1
  store ptr %63, ptr @sp, align 8
  %64 = load i64, ptr %62, align 8
  %65 = inttoptr i64 %64 to ptr
  %66 = load i64, ptr @ax, align 8
  %67 = trunc i64 %66 to i8
  store i8 %67, ptr %65, align 1
  %68 = sext i8 %67 to i64
  store i64 %68, ptr @ax, align 8
  br label %559

69:                                               ; preds = %56
  %70 = load i64, ptr %1, align 8
  %71 = icmp eq i64 %70, 11
  %72 = zext i1 %71 to i32
  %73 = icmp ne i32 %72, 0
  br i1 %73, label %74, label %80

74:                                               ; preds = %69
  %75 = load ptr, ptr @sp, align 8
  %76 = getelementptr inbounds i64, ptr %75, i16 1
  store ptr %76, ptr @sp, align 8
  %77 = load i64, ptr %75, align 8
  %78 = inttoptr i64 %77 to ptr
  %79 = load i64, ptr @ax, align 8
  store i64 %79, ptr %78, align 8
  br label %558

80:                                               ; preds = %69
  %81 = load i64, ptr %1, align 8
  %82 = icmp eq i64 %81, 13
  %83 = zext i1 %82 to i32
  %84 = icmp ne i32 %83, 0
  br i1 %84, label %85, label %89

85:                                               ; preds = %80
  %86 = load ptr, ptr @sp, align 8
  %87 = getelementptr inbounds i64, ptr %86, i64 -1
  store ptr %87, ptr @sp, align 8
  %88 = load i64, ptr @ax, align 8
  store i64 %88, ptr %87, align 8
  br label %557

89:                                               ; preds = %80
  %90 = load i64, ptr %1, align 8
  %91 = icmp eq i64 %90, 2
  %92 = zext i1 %91 to i32
  %93 = icmp ne i32 %92, 0
  br i1 %93, label %94, label %98

94:                                               ; preds = %89
  %95 = load ptr, ptr @pc, align 8
  %96 = load i64, ptr %95, align 8
  %97 = inttoptr i64 %96 to ptr
  store ptr %97, ptr @pc, align 8
  br label %556

98:                                               ; preds = %89
  %99 = load i64, ptr %1, align 8
  %100 = icmp eq i64 %99, 4
  %101 = zext i1 %100 to i32
  %102 = icmp ne i32 %101, 0
  br i1 %102, label %103, label %115

103:                                              ; preds = %98
  %104 = load i64, ptr @ax, align 8
  %105 = icmp ne i64 %104, 0
  br i1 %105, label %106, label %109

106:                                              ; preds = %103
  %107 = load ptr, ptr @pc, align 8
  %108 = getelementptr inbounds i64, ptr %107, i32 1
  br label %113

109:                                              ; preds = %103
  %110 = load ptr, ptr @pc, align 8
  %111 = load i64, ptr %110, align 8
  %112 = inttoptr i64 %111 to ptr
  br label %113

113:                                              ; preds = %109, %106
  %114 = phi ptr [ %108, %106 ], [ %112, %109 ]
  store ptr %114, ptr @pc, align 8
  br label %555

115:                                              ; preds = %98
  %116 = load i64, ptr %1, align 8
  %117 = icmp eq i64 %116, 5
  %118 = zext i1 %117 to i32
  %119 = icmp ne i32 %118, 0
  br i1 %119, label %120, label %132

120:                                              ; preds = %115
  %121 = load i64, ptr @ax, align 8
  %122 = icmp ne i64 %121, 0
  br i1 %122, label %123, label %127

123:                                              ; preds = %120
  %124 = load ptr, ptr @pc, align 8
  %125 = load i64, ptr %124, align 8
  %126 = inttoptr i64 %125 to ptr
  br label %130

127:                                              ; preds = %120
  %128 = load ptr, ptr @pc, align 8
  %129 = getelementptr inbounds i64, ptr %128, i32 1
  br label %130

130:                                              ; preds = %127, %123
  %131 = phi ptr [ %126, %123 ], [ %129, %127 ]
  store ptr %131, ptr @pc, align 8
  br label %554

132:                                              ; preds = %115
  %133 = load i64, ptr %1, align 8
  %134 = icmp eq i64 %133, 3
  %135 = zext i1 %134 to i32
  %136 = icmp ne i32 %135, 0
  br i1 %136, label %137, label %146

137:                                              ; preds = %132
  %138 = load ptr, ptr @sp, align 8
  %139 = getelementptr inbounds i64, ptr %138, i64 -1
  store ptr %139, ptr @sp, align 8
  %140 = load ptr, ptr @pc, align 8
  %141 = getelementptr inbounds i64, ptr %140, i32 1
  %142 = ptrtoint ptr %141 to i64
  store i64 %142, ptr %139, align 8
  %143 = load ptr, ptr @pc, align 8
  %144 = load i64, ptr %143, align 8
  %145 = inttoptr i64 %144 to ptr
  store ptr %145, ptr @pc, align 8
  br label %553

146:                                              ; preds = %132
  %147 = load i64, ptr %1, align 8
  %148 = icmp eq i64 %147, 6
  %149 = zext i1 %148 to i32
  %150 = icmp ne i32 %149, 0
  br i1 %150, label %151, label %163

151:                                              ; preds = %146
  %152 = load ptr, ptr @sp, align 8
  %153 = getelementptr inbounds i64, ptr %152, i64 -1
  store ptr %153, ptr @sp, align 8
  %154 = load ptr, ptr @bp, align 8
  %155 = ptrtoint ptr %154 to i64
  store i64 %155, ptr %153, align 8
  %156 = load ptr, ptr @sp, align 8
  store ptr %156, ptr @bp, align 8
  %157 = load ptr, ptr @sp, align 8
  %158 = load ptr, ptr @pc, align 8
  %159 = getelementptr inbounds i64, ptr %158, i16 1
  store ptr %159, ptr @pc, align 8
  %160 = load i64, ptr %158, align 8
  %161 = sub i64 0, %160
  %162 = getelementptr inbounds i64, ptr %157, i64 %161
  store ptr %162, ptr @sp, align 8
  br label %552

163:                                              ; preds = %146
  %164 = load i64, ptr %1, align 8
  %165 = icmp eq i64 %164, 7
  %166 = zext i1 %165 to i32
  %167 = icmp ne i32 %166, 0
  br i1 %167, label %168, label %174

168:                                              ; preds = %163
  %169 = load ptr, ptr @sp, align 8
  %170 = load ptr, ptr @pc, align 8
  %171 = getelementptr inbounds i64, ptr %170, i16 1
  store ptr %171, ptr @pc, align 8
  %172 = load i64, ptr %170, align 8
  %173 = getelementptr inbounds i64, ptr %169, i64 %172
  store ptr %173, ptr @sp, align 8
  br label %551

174:                                              ; preds = %163
  %175 = load i64, ptr %1, align 8
  %176 = icmp eq i64 %175, 8
  %177 = zext i1 %176 to i32
  %178 = icmp ne i32 %177, 0
  br i1 %178, label %179, label %189

179:                                              ; preds = %174
  %180 = load ptr, ptr @bp, align 8
  store ptr %180, ptr @sp, align 8
  %181 = load ptr, ptr @sp, align 8
  %182 = getelementptr inbounds i64, ptr %181, i16 1
  store ptr %182, ptr @sp, align 8
  %183 = load i64, ptr %181, align 8
  %184 = inttoptr i64 %183 to ptr
  store ptr %184, ptr @bp, align 8
  %185 = load ptr, ptr @sp, align 8
  %186 = getelementptr inbounds i64, ptr %185, i16 1
  store ptr %186, ptr @sp, align 8
  %187 = load i64, ptr %185, align 8
  %188 = inttoptr i64 %187 to ptr
  store ptr %188, ptr @pc, align 8
  br label %550

189:                                              ; preds = %174
  %190 = load i64, ptr %1, align 8
  %191 = icmp eq i64 %190, 0
  %192 = zext i1 %191 to i32
  %193 = icmp ne i32 %192, 0
  br i1 %193, label %194, label %201

194:                                              ; preds = %189
  %195 = load ptr, ptr @bp, align 8
  %196 = load ptr, ptr @pc, align 8
  %197 = getelementptr inbounds i64, ptr %196, i16 1
  store ptr %197, ptr @pc, align 8
  %198 = load i64, ptr %196, align 8
  %199 = getelementptr inbounds i64, ptr %195, i64 %198
  %200 = ptrtoint ptr %199 to i64
  store i64 %200, ptr @ax, align 8
  br label %549

201:                                              ; preds = %189
  %202 = load i64, ptr %1, align 8
  %203 = icmp eq i64 %202, 14
  %204 = zext i1 %203 to i32
  %205 = icmp ne i32 %204, 0
  br i1 %205, label %206, label %212

206:                                              ; preds = %201
  %207 = load ptr, ptr @sp, align 8
  %208 = getelementptr inbounds i64, ptr %207, i16 1
  store ptr %208, ptr @sp, align 8
  %209 = load i64, ptr %207, align 8
  %210 = load i64, ptr @ax, align 8
  %211 = or i64 %209, %210
  store i64 %211, ptr @ax, align 8
  br label %548

212:                                              ; preds = %201
  %213 = load i64, ptr %1, align 8
  %214 = icmp eq i64 %213, 15
  %215 = zext i1 %214 to i32
  %216 = icmp ne i32 %215, 0
  br i1 %216, label %217, label %223

217:                                              ; preds = %212
  %218 = load ptr, ptr @sp, align 8
  %219 = getelementptr inbounds i64, ptr %218, i16 1
  store ptr %219, ptr @sp, align 8
  %220 = load i64, ptr %218, align 8
  %221 = load i64, ptr @ax, align 8
  %222 = xor i64 %220, %221
  store i64 %222, ptr @ax, align 8
  br label %547

223:                                              ; preds = %212
  %224 = load i64, ptr %1, align 8
  %225 = icmp eq i64 %224, 16
  %226 = zext i1 %225 to i32
  %227 = icmp ne i32 %226, 0
  br i1 %227, label %228, label %234

228:                                              ; preds = %223
  %229 = load ptr, ptr @sp, align 8
  %230 = getelementptr inbounds i64, ptr %229, i16 1
  store ptr %230, ptr @sp, align 8
  %231 = load i64, ptr %229, align 8
  %232 = load i64, ptr @ax, align 8
  %233 = and i64 %231, %232
  store i64 %233, ptr @ax, align 8
  br label %546

234:                                              ; preds = %223
  %235 = load i64, ptr %1, align 8
  %236 = icmp eq i64 %235, 17
  %237 = zext i1 %236 to i32
  %238 = icmp ne i32 %237, 0
  br i1 %238, label %239, label %247

239:                                              ; preds = %234
  %240 = load ptr, ptr @sp, align 8
  %241 = getelementptr inbounds i64, ptr %240, i16 1
  store ptr %241, ptr @sp, align 8
  %242 = load i64, ptr %240, align 8
  %243 = load i64, ptr @ax, align 8
  %244 = icmp eq i64 %242, %243
  %245 = zext i1 %244 to i32
  %246 = sext i32 %245 to i64
  store i64 %246, ptr @ax, align 8
  br label %545

247:                                              ; preds = %234
  %248 = load i64, ptr %1, align 8
  %249 = icmp eq i64 %248, 18
  %250 = zext i1 %249 to i32
  %251 = icmp ne i32 %250, 0
  br i1 %251, label %252, label %260

252:                                              ; preds = %247
  %253 = load ptr, ptr @sp, align 8
  %254 = getelementptr inbounds i64, ptr %253, i16 1
  store ptr %254, ptr @sp, align 8
  %255 = load i64, ptr %253, align 8
  %256 = load i64, ptr @ax, align 8
  %257 = icmp ne i64 %255, %256
  %258 = zext i1 %257 to i32
  %259 = sext i32 %258 to i64
  store i64 %259, ptr @ax, align 8
  br label %544

260:                                              ; preds = %247
  %261 = load i64, ptr %1, align 8
  %262 = icmp eq i64 %261, 19
  %263 = zext i1 %262 to i32
  %264 = icmp ne i32 %263, 0
  br i1 %264, label %265, label %273

265:                                              ; preds = %260
  %266 = load ptr, ptr @sp, align 8
  %267 = getelementptr inbounds i64, ptr %266, i16 1
  store ptr %267, ptr @sp, align 8
  %268 = load i64, ptr %266, align 8
  %269 = load i64, ptr @ax, align 8
  %270 = icmp slt i64 %268, %269
  %271 = zext i1 %270 to i32
  %272 = sext i32 %271 to i64
  store i64 %272, ptr @ax, align 8
  br label %543

273:                                              ; preds = %260
  %274 = load i64, ptr %1, align 8
  %275 = icmp eq i64 %274, 21
  %276 = zext i1 %275 to i32
  %277 = icmp ne i32 %276, 0
  br i1 %277, label %278, label %286

278:                                              ; preds = %273
  %279 = load ptr, ptr @sp, align 8
  %280 = getelementptr inbounds i64, ptr %279, i16 1
  store ptr %280, ptr @sp, align 8
  %281 = load i64, ptr %279, align 8
  %282 = load i64, ptr @ax, align 8
  %283 = icmp sle i64 %281, %282
  %284 = zext i1 %283 to i32
  %285 = sext i32 %284 to i64
  store i64 %285, ptr @ax, align 8
  br label %542

286:                                              ; preds = %273
  %287 = load i64, ptr %1, align 8
  %288 = icmp eq i64 %287, 20
  %289 = zext i1 %288 to i32
  %290 = icmp ne i32 %289, 0
  br i1 %290, label %291, label %299

291:                                              ; preds = %286
  %292 = load ptr, ptr @sp, align 8
  %293 = getelementptr inbounds i64, ptr %292, i16 1
  store ptr %293, ptr @sp, align 8
  %294 = load i64, ptr %292, align 8
  %295 = load i64, ptr @ax, align 8
  %296 = icmp sgt i64 %294, %295
  %297 = zext i1 %296 to i32
  %298 = sext i32 %297 to i64
  store i64 %298, ptr @ax, align 8
  br label %541

299:                                              ; preds = %286
  %300 = load i64, ptr %1, align 8
  %301 = icmp eq i64 %300, 22
  %302 = zext i1 %301 to i32
  %303 = icmp ne i32 %302, 0
  br i1 %303, label %304, label %312

304:                                              ; preds = %299
  %305 = load ptr, ptr @sp, align 8
  %306 = getelementptr inbounds i64, ptr %305, i16 1
  store ptr %306, ptr @sp, align 8
  %307 = load i64, ptr %305, align 8
  %308 = load i64, ptr @ax, align 8
  %309 = icmp sge i64 %307, %308
  %310 = zext i1 %309 to i32
  %311 = sext i32 %310 to i64
  store i64 %311, ptr @ax, align 8
  br label %540

312:                                              ; preds = %299
  %313 = load i64, ptr %1, align 8
  %314 = icmp eq i64 %313, 23
  %315 = zext i1 %314 to i32
  %316 = icmp ne i32 %315, 0
  br i1 %316, label %317, label %323

317:                                              ; preds = %312
  %318 = load ptr, ptr @sp, align 8
  %319 = getelementptr inbounds i64, ptr %318, i16 1
  store ptr %319, ptr @sp, align 8
  %320 = load i64, ptr %318, align 8
  %321 = load i64, ptr @ax, align 8
  %322 = shl i64 %320, %321
  store i64 %322, ptr @ax, align 8
  br label %539

323:                                              ; preds = %312
  %324 = load i64, ptr %1, align 8
  %325 = icmp eq i64 %324, 24
  %326 = zext i1 %325 to i32
  %327 = icmp ne i32 %326, 0
  br i1 %327, label %328, label %334

328:                                              ; preds = %323
  %329 = load ptr, ptr @sp, align 8
  %330 = getelementptr inbounds i64, ptr %329, i16 1
  store ptr %330, ptr @sp, align 8
  %331 = load i64, ptr %329, align 8
  %332 = load i64, ptr @ax, align 8
  %333 = ashr i64 %331, %332
  store i64 %333, ptr @ax, align 8
  br label %538

334:                                              ; preds = %323
  %335 = load i64, ptr %1, align 8
  %336 = icmp eq i64 %335, 25
  %337 = zext i1 %336 to i32
  %338 = icmp ne i32 %337, 0
  br i1 %338, label %339, label %345

339:                                              ; preds = %334
  %340 = load ptr, ptr @sp, align 8
  %341 = getelementptr inbounds i64, ptr %340, i16 1
  store ptr %341, ptr @sp, align 8
  %342 = load i64, ptr %340, align 8
  %343 = load i64, ptr @ax, align 8
  %344 = add nsw i64 %342, %343
  store i64 %344, ptr @ax, align 8
  br label %537

345:                                              ; preds = %334
  %346 = load i64, ptr %1, align 8
  %347 = icmp eq i64 %346, 26
  %348 = zext i1 %347 to i32
  %349 = icmp ne i32 %348, 0
  br i1 %349, label %350, label %356

350:                                              ; preds = %345
  %351 = load ptr, ptr @sp, align 8
  %352 = getelementptr inbounds i64, ptr %351, i16 1
  store ptr %352, ptr @sp, align 8
  %353 = load i64, ptr %351, align 8
  %354 = load i64, ptr @ax, align 8
  %355 = sub nsw i64 %353, %354
  store i64 %355, ptr @ax, align 8
  br label %536

356:                                              ; preds = %345
  %357 = load i64, ptr %1, align 8
  %358 = icmp eq i64 %357, 27
  %359 = zext i1 %358 to i32
  %360 = icmp ne i32 %359, 0
  br i1 %360, label %361, label %367

361:                                              ; preds = %356
  %362 = load ptr, ptr @sp, align 8
  %363 = getelementptr inbounds i64, ptr %362, i16 1
  store ptr %363, ptr @sp, align 8
  %364 = load i64, ptr %362, align 8
  %365 = load i64, ptr @ax, align 8
  %366 = mul nsw i64 %364, %365
  store i64 %366, ptr @ax, align 8
  br label %535

367:                                              ; preds = %356
  %368 = load i64, ptr %1, align 8
  %369 = icmp eq i64 %368, 28
  %370 = zext i1 %369 to i32
  %371 = icmp ne i32 %370, 0
  br i1 %371, label %372, label %378

372:                                              ; preds = %367
  %373 = load ptr, ptr @sp, align 8
  %374 = getelementptr inbounds i64, ptr %373, i16 1
  store ptr %374, ptr @sp, align 8
  %375 = load i64, ptr %373, align 8
  %376 = load i64, ptr @ax, align 8
  %377 = sdiv i64 %375, %376
  store i64 %377, ptr @ax, align 8
  br label %534

378:                                              ; preds = %367
  %379 = load i64, ptr %1, align 8
  %380 = icmp eq i64 %379, 29
  %381 = zext i1 %380 to i32
  %382 = icmp ne i32 %381, 0
  br i1 %382, label %383, label %389

383:                                              ; preds = %378
  %384 = load ptr, ptr @sp, align 8
  %385 = getelementptr inbounds i64, ptr %384, i16 1
  store ptr %385, ptr @sp, align 8
  %386 = load i64, ptr %384, align 8
  %387 = load i64, ptr @ax, align 8
  %388 = srem i64 %386, %387
  store i64 %388, ptr @ax, align 8
  br label %533

389:                                              ; preds = %378
  %390 = load i64, ptr %1, align 8
  %391 = icmp eq i64 %390, 37
  %392 = zext i1 %391 to i32
  %393 = icmp ne i32 %392, 0
  br i1 %393, label %394, label %400

394:                                              ; preds = %389
  %395 = load ptr, ptr @sp, align 8
  %396 = load i64, ptr %395, align 8
  %397 = call i32 (ptr, ...) @printf(ptr @.cstr.29, i64 %396)
  %398 = load ptr, ptr @sp, align 8
  %399 = load i64, ptr %398, align 8
  ret i64 %399

400:                                              ; preds = %389
  %401 = load i64, ptr %1, align 8
  %402 = icmp eq i64 %401, 30
  %403 = zext i1 %402 to i32
  %404 = icmp ne i32 %403, 0
  br i1 %404, label %405, label %416

405:                                              ; preds = %400
  %406 = load ptr, ptr @sp, align 8
  %407 = getelementptr inbounds i64, ptr %406, i32 1
  %408 = load i64, ptr %407, align 8
  %409 = inttoptr i64 %408 to ptr
  %410 = load ptr, ptr @sp, align 8
  %411 = getelementptr inbounds i64, ptr %410, i32 0
  %412 = load i64, ptr %411, align 8
  %413 = trunc i64 %412 to i32
  %414 = call i32 (ptr, i32, ...) @open(ptr %409, i32 %413)
  %415 = sext i32 %414 to i64
  store i64 %415, ptr @ax, align 8
  br label %531

416:                                              ; preds = %400
  %417 = load i64, ptr %1, align 8
  %418 = icmp eq i64 %417, 32
  %419 = zext i1 %418 to i32
  %420 = icmp ne i32 %419, 0
  br i1 %420, label %421, label %427

421:                                              ; preds = %416
  %422 = load ptr, ptr @sp, align 8
  %423 = load i64, ptr %422, align 8
  %424 = trunc i64 %423 to i32
  %425 = call i32 @close(i32 %424)
  %426 = sext i32 %425 to i64
  store i64 %426, ptr @ax, align 8
  br label %530

427:                                              ; preds = %416
  %428 = load i64, ptr %1, align 8
  %429 = icmp eq i64 %428, 31
  %430 = zext i1 %429 to i32
  %431 = icmp ne i32 %430, 0
  br i1 %431, label %432, label %444

432:                                              ; preds = %427
  %433 = load ptr, ptr @sp, align 8
  %434 = getelementptr inbounds i64, ptr %433, i32 2
  %435 = load i64, ptr %434, align 8
  %436 = trunc i64 %435 to i32
  %437 = load ptr, ptr @sp, align 8
  %438 = getelementptr inbounds i64, ptr %437, i32 1
  %439 = load i64, ptr %438, align 8
  %440 = inttoptr i64 %439 to ptr
  %441 = load ptr, ptr @sp, align 8
  %442 = load i64, ptr %441, align 8
  %443 = call i64 @read(i32 %436, ptr %440, i64 %442)
  store i64 %443, ptr @ax, align 8
  br label %529

444:                                              ; preds = %427
  %445 = load i64, ptr %1, align 8
  %446 = icmp eq i64 %445, 33
  %447 = zext i1 %446 to i32
  %448 = icmp ne i32 %447, 0
  br i1 %448, label %449, label %476

449:                                              ; preds = %444
  %450 = load ptr, ptr @sp, align 8
  %451 = load ptr, ptr @pc, align 8
  %452 = getelementptr inbounds i64, ptr %451, i32 1
  %453 = load i64, ptr %452, align 8
  %454 = getelementptr inbounds i64, ptr %450, i64 %453
  store ptr %454, ptr %2, align 8
  %455 = load ptr, ptr %2, align 8
  %456 = getelementptr inbounds i64, ptr %455, i32 -1
  %457 = load i64, ptr %456, align 8
  %458 = inttoptr i64 %457 to ptr
  %459 = load ptr, ptr %2, align 8
  %460 = getelementptr inbounds i64, ptr %459, i32 -2
  %461 = load i64, ptr %460, align 8
  %462 = load ptr, ptr %2, align 8
  %463 = getelementptr inbounds i64, ptr %462, i32 -3
  %464 = load i64, ptr %463, align 8
  %465 = load ptr, ptr %2, align 8
  %466 = getelementptr inbounds i64, ptr %465, i32 -4
  %467 = load i64, ptr %466, align 8
  %468 = load ptr, ptr %2, align 8
  %469 = getelementptr inbounds i64, ptr %468, i32 -5
  %470 = load i64, ptr %469, align 8
  %471 = load ptr, ptr %2, align 8
  %472 = getelementptr inbounds i64, ptr %471, i32 -6
  %473 = load i64, ptr %472, align 8
  %474 = call i32 (ptr, ...) @printf(ptr %458, i64 %461, i64 %464, i64 %467, i64 %470, i64 %473)
  %475 = sext i32 %474 to i64
  store i64 %475, ptr @ax, align 8
  br label %528

476:                                              ; preds = %444
  %477 = load i64, ptr %1, align 8
  %478 = icmp eq i64 %477, 34
  %479 = zext i1 %478 to i32
  %480 = icmp ne i32 %479, 0
  br i1 %480, label %481, label %486

481:                                              ; preds = %476
  %482 = load ptr, ptr @sp, align 8
  %483 = load i64, ptr %482, align 8
  %484 = call ptr @malloc(i64 %483)
  %485 = ptrtoint ptr %484 to i64
  store i64 %485, ptr @ax, align 8
  br label %527

486:                                              ; preds = %476
  %487 = load i64, ptr %1, align 8
  %488 = icmp eq i64 %487, 35
  %489 = zext i1 %488 to i32
  %490 = icmp ne i32 %489, 0
  br i1 %490, label %491, label %504

491:                                              ; preds = %486
  %492 = load ptr, ptr @sp, align 8
  %493 = getelementptr inbounds i64, ptr %492, i32 2
  %494 = load i64, ptr %493, align 8
  %495 = inttoptr i64 %494 to ptr
  %496 = load ptr, ptr @sp, align 8
  %497 = getelementptr inbounds i64, ptr %496, i32 1
  %498 = load i64, ptr %497, align 8
  %499 = trunc i64 %498 to i32
  %500 = load ptr, ptr @sp, align 8
  %501 = load i64, ptr %500, align 8
  %502 = call ptr @memset(ptr %495, i32 %499, i64 %501)
  %503 = ptrtoint ptr %502 to i64
  store i64 %503, ptr @ax, align 8
  br label %526

504:                                              ; preds = %486
  %505 = load i64, ptr %1, align 8
  %506 = icmp eq i64 %505, 36
  %507 = zext i1 %506 to i32
  %508 = icmp ne i32 %507, 0
  br i1 %508, label %509, label %522

509:                                              ; preds = %504
  %510 = load ptr, ptr @sp, align 8
  %511 = getelementptr inbounds i64, ptr %510, i32 2
  %512 = load i64, ptr %511, align 8
  %513 = inttoptr i64 %512 to ptr
  %514 = load ptr, ptr @sp, align 8
  %515 = getelementptr inbounds i64, ptr %514, i32 1
  %516 = load i64, ptr %515, align 8
  %517 = inttoptr i64 %516 to ptr
  %518 = load ptr, ptr @sp, align 8
  %519 = load i64, ptr %518, align 8
  %520 = call i32 @memcmp(ptr %513, ptr %517, i64 %519)
  %521 = sext i32 %520 to i64
  store i64 %521, ptr @ax, align 8
  br label %525

522:                                              ; preds = %504
  %523 = load i64, ptr %1, align 8
  %524 = call i32 (ptr, ...) @printf(ptr @.cstr.30, i64 %523)
  ret i64 -1

525:                                              ; preds = %509
  br label %526

526:                                              ; preds = %525, %491
  br label %527

527:                                              ; preds = %526, %481
  br label %528

528:                                              ; preds = %527, %449
  br label %529

529:                                              ; preds = %528, %432
  br label %530

530:                                              ; preds = %529, %421
  br label %531

531:                                              ; preds = %530, %405
  br label %532

532:                                              ; preds = %531
  br label %533

533:                                              ; preds = %532, %383
  br label %534

534:                                              ; preds = %533, %372
  br label %535

535:                                              ; preds = %534, %361
  br label %536

536:                                              ; preds = %535, %350
  br label %537

537:                                              ; preds = %536, %339
  br label %538

538:                                              ; preds = %537, %328
  br label %539

539:                                              ; preds = %538, %317
  br label %540

540:                                              ; preds = %539, %304
  br label %541

541:                                              ; preds = %540, %291
  br label %542

542:                                              ; preds = %541, %278
  br label %543

543:                                              ; preds = %542, %265
  br label %544

544:                                              ; preds = %543, %252
  br label %545

545:                                              ; preds = %544, %239
  br label %546

546:                                              ; preds = %545, %228
  br label %547

547:                                              ; preds = %546, %217
  br label %548

548:                                              ; preds = %547, %206
  br label %549

549:                                              ; preds = %548, %194
  br label %550

550:                                              ; preds = %549, %179
  br label %551

551:                                              ; preds = %550, %168
  br label %552

552:                                              ; preds = %551, %151
  br label %553

553:                                              ; preds = %552, %137
  br label %554

554:                                              ; preds = %553, %130
  br label %555

555:                                              ; preds = %554, %113
  br label %556

556:                                              ; preds = %555, %94
  br label %557

557:                                              ; preds = %556, %85
  br label %558

558:                                              ; preds = %557, %74
  br label %559

559:                                              ; preds = %558, %61
  br label %560

560:                                              ; preds = %559, %52
  br label %561

561:                                              ; preds = %560, %42
  br label %562

562:                                              ; preds = %561, %33
  br label %563

563:                                              ; preds = %562, %0
  br i1 true, label %3, label %564

564:                                              ; preds = %563
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
  %11 = getelementptr inbounds ptr, ptr %10, i16 1
  store ptr %11, ptr %4, align 8
  %12 = load i32, ptr %3, align 4
  %13 = icmp sgt i32 %12, 0
  br i1 %13, label %14, label %20

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8
  %16 = load ptr, ptr %15, align 8
  %17 = load i8, ptr %16, align 1
  %18 = sext i8 %17 to i32
  %19 = icmp eq i32 %18, 45
  br label %20

20:                                               ; preds = %14, %2
  %21 = phi i1 [ %13, %2 ], [ %19, %14 ]
  %22 = icmp ne i1 %21, false
  br i1 %22, label %23, label %30

23:                                               ; preds = %20
  %24 = load ptr, ptr %4, align 8
  %25 = load ptr, ptr %24, align 8
  %26 = getelementptr inbounds i8, ptr %25, i32 1
  %27 = load i8, ptr %26, align 1
  %28 = sext i8 %27 to i32
  %29 = icmp eq i32 %28, 115
  br label %30

30:                                               ; preds = %23, %20
  %31 = phi i1 [ %22, %20 ], [ %29, %23 ]
  %32 = zext i1 %31 to i32
  %33 = icmp ne i32 %32, 0
  br i1 %33, label %34, label %39

34:                                               ; preds = %30
  store i64 1, ptr @assembly, align 8
  %35 = load i32, ptr %3, align 4
  %36 = sub nsw i32 %35, 1
  store i32 %36, ptr %3, align 4
  %37 = load ptr, ptr %4, align 8
  %38 = getelementptr inbounds ptr, ptr %37, i64 1
  store ptr %38, ptr %4, align 8
  br label %39

39:                                               ; preds = %34, %30
  %40 = load i32, ptr %3, align 4
  %41 = icmp sgt i32 %40, 0
  br i1 %41, label %42, label %48

42:                                               ; preds = %39
  %43 = load ptr, ptr %4, align 8
  %44 = load ptr, ptr %43, align 8
  %45 = load i8, ptr %44, align 1
  %46 = sext i8 %45 to i32
  %47 = icmp eq i32 %46, 45
  br label %48

48:                                               ; preds = %42, %39
  %49 = phi i1 [ %41, %39 ], [ %47, %42 ]
  %50 = icmp ne i1 %49, false
  br i1 %50, label %51, label %58

51:                                               ; preds = %48
  %52 = load ptr, ptr %4, align 8
  %53 = load ptr, ptr %52, align 8
  %54 = getelementptr inbounds i8, ptr %53, i32 1
  %55 = load i8, ptr %54, align 1
  %56 = sext i8 %55 to i32
  %57 = icmp eq i32 %56, 100
  br label %58

58:                                               ; preds = %51, %48
  %59 = phi i1 [ %50, %48 ], [ %57, %51 ]
  %60 = zext i1 %59 to i32
  %61 = icmp ne i32 %60, 0
  br i1 %61, label %62, label %67

62:                                               ; preds = %58
  store i64 1, ptr @debug, align 8
  %63 = load i32, ptr %3, align 4
  %64 = sub nsw i32 %63, 1
  store i32 %64, ptr %3, align 4
  %65 = load ptr, ptr %4, align 8
  %66 = getelementptr inbounds ptr, ptr %65, i64 1
  store ptr %66, ptr %4, align 8
  br label %67

67:                                               ; preds = %62, %58
  %68 = load i32, ptr %3, align 4
  %69 = icmp slt i32 %68, 1
  %70 = zext i1 %69 to i32
  %71 = icmp ne i32 %70, 0
  br i1 %71, label %72, label %74

72:                                               ; preds = %67
  %73 = call i32 (ptr, ...) @printf(ptr @.cstr.31)
  ret i32 -1

74:                                               ; preds = %67
  %75 = load ptr, ptr %4, align 8
  %76 = load ptr, ptr %75, align 8
  %77 = call i32 (ptr, i32, ...) @open(ptr %76, i32 0)
  %78 = sext i32 %77 to i64
  store i64 %78, ptr %6, align 8
  %79 = icmp slt i64 %78, 0
  %80 = zext i1 %79 to i32
  %81 = icmp ne i32 %80, 0
  br i1 %81, label %82, label %86

82:                                               ; preds = %74
  %83 = load ptr, ptr %4, align 8
  %84 = load ptr, ptr %83, align 8
  %85 = call i32 (ptr, ...) @printf(ptr @.cstr.32, ptr %84)
  ret i32 -1

86:                                               ; preds = %74
  store i64 262144, ptr @poolsize, align 8
  store i64 1, ptr @line, align 8
  %87 = load i64, ptr @poolsize, align 8
  %88 = call ptr @malloc(i64 %87)
  store ptr %88, ptr @text, align 8
  %89 = icmp eq ptr %88, null
  %90 = zext i1 %89 to i32
  %91 = icmp ne i32 %90, 0
  br i1 %91, label %92, label %95

92:                                               ; preds = %86
  %93 = load i64, ptr @poolsize, align 8
  %94 = call i32 (ptr, ...) @printf(ptr @.cstr.33, i64 %93)
  ret i32 -1

95:                                               ; preds = %86
  %96 = load i64, ptr @poolsize, align 8
  %97 = call ptr @malloc(i64 %96)
  store ptr %97, ptr @data, align 8
  %98 = icmp eq ptr %97, null
  %99 = zext i1 %98 to i32
  %100 = icmp ne i32 %99, 0
  br i1 %100, label %101, label %104

101:                                              ; preds = %95
  %102 = load i64, ptr @poolsize, align 8
  %103 = call i32 (ptr, ...) @printf(ptr @.cstr.34, i64 %102)
  ret i32 -1

104:                                              ; preds = %95
  %105 = load i64, ptr @poolsize, align 8
  %106 = call ptr @malloc(i64 %105)
  store ptr %106, ptr @stack, align 8
  %107 = icmp eq ptr %106, null
  %108 = zext i1 %107 to i32
  %109 = icmp ne i32 %108, 0
  br i1 %109, label %110, label %113

110:                                              ; preds = %104
  %111 = load i64, ptr @poolsize, align 8
  %112 = call i32 (ptr, ...) @printf(ptr @.cstr.35, i64 %111)
  ret i32 -1

113:                                              ; preds = %104
  %114 = load i64, ptr @poolsize, align 8
  %115 = call ptr @malloc(i64 %114)
  store ptr %115, ptr @symbols, align 8
  %116 = icmp eq ptr %115, null
  %117 = zext i1 %116 to i32
  %118 = icmp ne i32 %117, 0
  br i1 %118, label %119, label %122

119:                                              ; preds = %113
  %120 = load i64, ptr @poolsize, align 8
  %121 = call i32 (ptr, ...) @printf(ptr @.cstr.36, i64 %120)
  ret i32 -1

122:                                              ; preds = %113
  %123 = load ptr, ptr @text, align 8
  %124 = load i64, ptr @poolsize, align 8
  %125 = call ptr @memset(ptr %123, i32 0, i64 %124)
  %126 = load ptr, ptr @data, align 8
  %127 = load i64, ptr @poolsize, align 8
  %128 = call ptr @memset(ptr %126, i32 0, i64 %127)
  %129 = load ptr, ptr @stack, align 8
  %130 = load i64, ptr @poolsize, align 8
  %131 = call ptr @memset(ptr %129, i32 0, i64 %130)
  %132 = load ptr, ptr @symbols, align 8
  %133 = load i64, ptr @poolsize, align 8
  %134 = call ptr @memset(ptr %132, i32 0, i64 %133)
  %135 = load ptr, ptr @text, align 8
  store ptr %135, ptr @old_text, align 8
  store ptr @.cstr.37, ptr @src, align 8
  store i64 134, ptr %5, align 8
  br label %141

136:                                              ; preds = %141
  call void (...) @next()
  %137 = load ptr, ptr @current_id, align 8
  %138 = getelementptr inbounds i64, ptr %137, i32 0
  %139 = load i64, ptr %5, align 8
  %140 = add i64 %139, 1
  store i64 %140, ptr %5, align 8
  store i64 %139, ptr %138, align 8
  br label %141

141:                                              ; preds = %136, %122
  %142 = load i64, ptr %5, align 8
  %143 = icmp sle i64 %142, 141
  %144 = zext i1 %143 to i32
  %145 = icmp ne i32 %144, 0
  br i1 %145, label %136, label %146

146:                                              ; preds = %141
  store i64 30, ptr %5, align 8
  br label %156

147:                                              ; preds = %156
  call void (...) @next()
  %148 = load ptr, ptr @current_id, align 8
  %149 = getelementptr inbounds i64, ptr %148, i32 4
  store i64 130, ptr %149, align 8
  %150 = load ptr, ptr @current_id, align 8
  %151 = getelementptr inbounds i64, ptr %150, i32 3
  store i64 1, ptr %151, align 8
  %152 = load ptr, ptr @current_id, align 8
  %153 = getelementptr inbounds i64, ptr %152, i32 5
  %154 = load i64, ptr %5, align 8
  %155 = add i64 %154, 1
  store i64 %155, ptr %5, align 8
  store i64 %154, ptr %153, align 8
  br label %156

156:                                              ; preds = %147, %146
  %157 = load i64, ptr %5, align 8
  %158 = icmp sle i64 %157, 37
  %159 = zext i1 %158 to i32
  %160 = icmp ne i32 %159, 0
  br i1 %160, label %147, label %161

161:                                              ; preds = %156
  call void (...) @next()
  %162 = load ptr, ptr @current_id, align 8
  %163 = getelementptr inbounds i64, ptr %162, i32 0
  store i64 134, ptr %163, align 8
  call void (...) @next()
  %164 = load ptr, ptr @current_id, align 8
  store ptr %164, ptr @idmain, align 8
  %165 = load i64, ptr @poolsize, align 8
  %166 = call ptr @malloc(i64 %165)
  store ptr %166, ptr @old_src, align 8
  store ptr %166, ptr @src, align 8
  %167 = icmp eq ptr %166, null
  %168 = zext i1 %167 to i32
  %169 = icmp ne i32 %168, 0
  br i1 %169, label %170, label %173

170:                                              ; preds = %161
  %171 = load i64, ptr @poolsize, align 8
  %172 = call i32 (ptr, ...) @printf(ptr @.cstr.38, i64 %171)
  ret i32 -1

173:                                              ; preds = %161
  %174 = load i64, ptr %6, align 8
  %175 = trunc i64 %174 to i32
  %176 = load ptr, ptr @src, align 8
  %177 = load i64, ptr @poolsize, align 8
  %178 = sub nsw i64 %177, 1
  %179 = call i64 @read(i32 %175, ptr %176, i64 %178)
  store i64 %179, ptr %5, align 8
  %180 = icmp sle i64 %179, 0
  %181 = zext i1 %180 to i32
  %182 = icmp ne i32 %181, 0
  br i1 %182, label %183, label %186

183:                                              ; preds = %173
  %184 = load i64, ptr %5, align 8
  %185 = call i32 (ptr, ...) @printf(ptr @.cstr.39, i64 %184)
  ret i32 -1

186:                                              ; preds = %173
  %187 = load ptr, ptr @src, align 8
  %188 = load i64, ptr %5, align 8
  %189 = getelementptr inbounds i8, ptr %187, i64 %188
  store i8 0, ptr %189, align 1
  %190 = load i64, ptr %6, align 8
  %191 = trunc i64 %190 to i32
  %192 = call i32 @close(i32 %191)
  call void (...) @program()
  %193 = load ptr, ptr @idmain, align 8
  %194 = getelementptr inbounds i64, ptr %193, i32 5
  %195 = load i64, ptr %194, align 8
  %196 = inttoptr i64 %195 to ptr
  store ptr %196, ptr @pc, align 8
  %197 = icmp eq ptr %196, null
  %198 = zext i1 %197 to i32
  %199 = icmp ne i32 %198, 0
  br i1 %199, label %200, label %202

200:                                              ; preds = %186
  %201 = call i32 (ptr, ...) @printf(ptr @.cstr.40)
  ret i32 -1

202:                                              ; preds = %186
  %203 = load i64, ptr @assembly, align 8
  %204 = icmp ne i64 %203, 0
  br i1 %204, label %205, label %206

205:                                              ; preds = %202
  ret i32 0

206:                                              ; preds = %202
  %207 = load ptr, ptr @stack, align 8
  %208 = ptrtoint ptr %207 to i64
  %209 = load i64, ptr @poolsize, align 8
  %210 = add nsw i64 %208, %209
  %211 = inttoptr i64 %210 to ptr
  store ptr %211, ptr @sp, align 8
  %212 = load ptr, ptr @sp, align 8
  %213 = getelementptr inbounds i64, ptr %212, i64 -1
  store ptr %213, ptr @sp, align 8
  store i64 37, ptr %213, align 8
  %214 = load ptr, ptr @sp, align 8
  %215 = getelementptr inbounds i64, ptr %214, i64 -1
  store ptr %215, ptr @sp, align 8
  store i64 13, ptr %215, align 8
  %216 = load ptr, ptr @sp, align 8
  store ptr %216, ptr %7, align 8
  %217 = load ptr, ptr @sp, align 8
  %218 = getelementptr inbounds i64, ptr %217, i64 -1
  store ptr %218, ptr @sp, align 8
  %219 = load i32, ptr %3, align 4
  %220 = sext i32 %219 to i64
  store i64 %220, ptr %218, align 8
  %221 = load ptr, ptr @sp, align 8
  %222 = getelementptr inbounds i64, ptr %221, i64 -1
  store ptr %222, ptr @sp, align 8
  %223 = load ptr, ptr %4, align 8
  %224 = ptrtoint ptr %223 to i64
  store i64 %224, ptr %222, align 8
  %225 = load ptr, ptr @sp, align 8
  %226 = getelementptr inbounds i64, ptr %225, i64 -1
  store ptr %226, ptr @sp, align 8
  %227 = load ptr, ptr %7, align 8
  %228 = ptrtoint ptr %227 to i64
  store i64 %228, ptr %226, align 8
  %229 = call i64 (...) @eval()
  %230 = trunc i64 %229 to i32
  ret i32 %230
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"XCC version alpha (https://github.com/ianfun/xcc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
