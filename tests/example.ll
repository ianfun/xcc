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
@0 = private unnamed_addr constant [9 x i8] c"%d: %.*s\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"%8.4s\00", align 1
@2 = private unnamed_addr constant [190 x i8] c"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT\00", align 1
@3 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1
@4 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@5 = private unnamed_addr constant [24 x i8] c"%d: expected token: %d\0A\00", align 1
@6 = private unnamed_addr constant [40 x i8] c"%d: unexpected token EOF of expression\0A\00", align 1
@7 = private unnamed_addr constant [23 x i8] c"%d: bad function call\0A\00", align 1
@8 = private unnamed_addr constant [24 x i8] c"%d: undefined variable\0A\00", align 1
@9 = private unnamed_addr constant [21 x i8] c"%d: bad dereference\0A\00", align 1
@10 = private unnamed_addr constant [20 x i8] c"%d: bad address of\0A\00", align 1
@11 = private unnamed_addr constant [33 x i8] c"%d: bad lvalue of pre-increment\0A\00", align 1
@12 = private unnamed_addr constant [20 x i8] c"%d: bad expression\0A\00", align 1
@13 = private unnamed_addr constant [30 x i8] c"%d: bad lvalue in assignment\0A\00", align 1
@14 = private unnamed_addr constant [34 x i8] c"%d: missing colon in conditional\0A\00", align 1
@15 = private unnamed_addr constant [28 x i8] c"%d: bad value in increment\0A\00", align 1
@16 = private unnamed_addr constant [27 x i8] c"%d: pointer type expected\0A\00", align 1
@17 = private unnamed_addr constant [32 x i8] c"%d: compiler error, token = %d\0A\00", align 1
@18 = private unnamed_addr constant [28 x i8] c"%d: bad enum identifier %d\0A\00", align 1
@19 = private unnamed_addr constant [26 x i8] c"%d: bad enum initializer\0A\00", align 1
@20 = private unnamed_addr constant [31 x i8] c"%d: bad parameter declaration\0A\00", align 1
@21 = private unnamed_addr constant [37 x i8] c"%d: duplicate parameter declaration\0A\00", align 1
@22 = private unnamed_addr constant [27 x i8] c"%d: bad local declaration\0A\00", align 1
@23 = private unnamed_addr constant [33 x i8] c"%d: duplicate local declaration\0A\00", align 1
@24 = private unnamed_addr constant [28 x i8] c"%d: bad global declaration\0A\00", align 1
@25 = private unnamed_addr constant [34 x i8] c"%d: duplicate global declaration\0A\00", align 1
@26 = private unnamed_addr constant [9 x i8] c"%d> %.4s\00", align 1
@27 = private unnamed_addr constant [190 x i8] c"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT\00", align 1
@28 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1
@29 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@30 = private unnamed_addr constant [9 x i8] c"exit(%d)\00", align 1
@31 = private unnamed_addr constant [24 x i8] c"unknown instruction:%d\0A\00", align 1
@32 = private unnamed_addr constant [30 x i8] c"usage: xc [-s] [-d] file ...\0A\00", align 1
@33 = private unnamed_addr constant [20 x i8] c"could not open(%s)\0A\00", align 1
@34 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for text area\0A\00", align 1
@35 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for data area\0A\00", align 1
@36 = private unnamed_addr constant [37 x i8] c"could not malloc(%d) for stack area\0A\00", align 1
@37 = private unnamed_addr constant [39 x i8] c"could not malloc(%d) for symbol table\0A\00", align 1
@38 = private unnamed_addr constant [101 x i8] c"char else enum if int return sizeof while open read close printf malloc memset memcmp exit void main\00", align 1
@39 = private unnamed_addr constant [38 x i8] c"could not malloc(%d) for source area\0A\00", align 1
@40 = private unnamed_addr constant [20 x i8] c"read() returned %d\0A\00", align 1
@41 = private unnamed_addr constant [20 x i8] c"main() not defined\0A\00", align 1

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
  br label %838

3:                                                ; preds = %838
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
  %21 = call i32 (ptr, ...) @printf(ptr @0, i64 %14, i64 %19, ptr %20)
  %22 = load ptr, ptr @src, align 8
  store ptr %22, ptr @old_src, align 8
  br label %43

23:                                               ; preds = %43
  %24 = load ptr, ptr @old_text, align 8
  %25 = getelementptr inbounds i64, ptr %24, i64 1
  store ptr %25, ptr @old_text, align 8
  %26 = load i64, ptr %25, align 8
  %27 = mul nsw i64 %26, 5
  %28 = getelementptr inbounds [189 x i8], ptr @2, i32 0, i64 %27
  %29 = call i32 (ptr, ...) @printf(ptr @1, ptr %28)
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
  %39 = call i32 (ptr, ...) @printf(ptr @3, i64 %38)
  br label %42

40:                                               ; preds = %23
  %41 = call i32 (ptr, ...) @printf(ptr @4)
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
  br label %837

53:                                               ; preds = %3
  %54 = load i64, ptr @token, align 8
  %55 = icmp eq i64 %54, 35
  %56 = zext i1 %55 to i32
  %57 = icmp ne i32 %56, 0
  br i1 %57, label %58, label %81

58:                                               ; preds = %53
  br label %62

59:                                               ; preds = %76
  %60 = load ptr, ptr @src, align 8
  %61 = getelementptr inbounds i8, ptr %60, i32 1
  store ptr %61, ptr @src, align 8
  br label %62

62:                                               ; preds = %59, %58
  %63 = load ptr, ptr @src, align 8
  %64 = load i8, ptr %63, align 1
  %65 = sext i8 %64 to i32
  %66 = icmp ne i32 %65, 0
  %67 = zext i1 %66 to i32
  %68 = icmp ne i32 %67, 0
  br i1 %68, label %69, label %76

69:                                               ; preds = %62
  %70 = load ptr, ptr @src, align 8
  %71 = load i8, ptr %70, align 1
  %72 = sext i8 %71 to i32
  %73 = icmp ne i32 %72, 10
  %74 = zext i1 %73 to i32
  %75 = icmp ne i32 %74, 0
  br label %76

76:                                               ; preds = %69, %62
  %77 = phi i1 [ %68, %62 ], [ %75, %69 ]
  %78 = zext i1 %77 to i32
  %79 = icmp ne i32 %78, 0
  br i1 %79, label %59, label %80

80:                                               ; preds = %76
  br label %836

81:                                               ; preds = %53
  %82 = load i64, ptr @token, align 8
  %83 = icmp eq i64 %82, 95
  %84 = zext i1 %83 to i32
  %85 = icmp ne i32 %84, 0
  br i1 %85, label %118, label %86

86:                                               ; preds = %81
  %87 = load i64, ptr @token, align 8
  %88 = icmp sge i64 %87, 65
  %89 = zext i1 %88 to i32
  %90 = icmp ne i32 %89, 0
  br i1 %90, label %91, label %96

91:                                               ; preds = %86
  %92 = load i64, ptr @token, align 8
  %93 = icmp sle i64 %92, 90
  %94 = zext i1 %93 to i32
  %95 = icmp ne i32 %94, 0
  br label %96

96:                                               ; preds = %91, %86
  %97 = phi i1 [ %90, %86 ], [ %95, %91 ]
  %98 = zext i1 %97 to i32
  %99 = icmp ne i32 %98, 0
  br i1 %99, label %114, label %100

100:                                              ; preds = %96
  %101 = load i64, ptr @token, align 8
  %102 = icmp sge i64 %101, 97
  %103 = zext i1 %102 to i32
  %104 = icmp ne i32 %103, 0
  br i1 %104, label %105, label %110

105:                                              ; preds = %100
  %106 = load i64, ptr @token, align 8
  %107 = icmp sle i64 %106, 122
  %108 = zext i1 %107 to i32
  %109 = icmp ne i32 %108, 0
  br label %110

110:                                              ; preds = %105, %100
  %111 = phi i1 [ %104, %100 ], [ %109, %105 ]
  %112 = zext i1 %111 to i32
  %113 = icmp ne i32 %112, 0
  br label %114

114:                                              ; preds = %110, %96
  %115 = phi i1 [ %99, %96 ], [ %113, %110 ]
  %116 = zext i1 %115 to i32
  %117 = icmp ne i32 %116, 0
  br label %118

118:                                              ; preds = %114, %81
  %119 = phi i1 [ %85, %81 ], [ %117, %114 ]
  %120 = zext i1 %119 to i32
  %121 = icmp ne i32 %120, 0
  br i1 %121, label %122, label %260

122:                                              ; preds = %118
  %123 = load ptr, ptr @src, align 8
  %124 = getelementptr inbounds i8, ptr %123, i32 -1
  store ptr %124, ptr %1, align 8
  %125 = load i64, ptr @token, align 8
  store i64 %125, ptr %2, align 8
  br label %136

126:                                              ; preds = %205
  %127 = load i64, ptr %2, align 8
  %128 = mul nsw i64 %127, 147
  %129 = load ptr, ptr @src, align 8
  %130 = load i8, ptr %129, align 1
  %131 = sext i8 %130 to i32
  %132 = sext i32 %131 to i64
  %133 = add nsw i64 %128, %132
  store i64 %133, ptr %2, align 8
  %134 = load ptr, ptr @src, align 8
  %135 = getelementptr inbounds i8, ptr %134, i32 1
  store ptr %135, ptr @src, align 8
  br label %136

136:                                              ; preds = %126, %122
  %137 = load ptr, ptr @src, align 8
  %138 = load i8, ptr %137, align 1
  %139 = sext i8 %138 to i32
  %140 = icmp eq i32 %139, 95
  %141 = zext i1 %140 to i32
  %142 = icmp ne i32 %141, 0
  br i1 %142, label %205, label %143

143:                                              ; preds = %136
  %144 = load ptr, ptr @src, align 8
  %145 = load i8, ptr %144, align 1
  %146 = sext i8 %145 to i32
  %147 = icmp sge i32 %146, 48
  %148 = zext i1 %147 to i32
  %149 = icmp ne i32 %148, 0
  br i1 %149, label %150, label %157

150:                                              ; preds = %143
  %151 = load ptr, ptr @src, align 8
  %152 = load i8, ptr %151, align 1
  %153 = sext i8 %152 to i32
  %154 = icmp sle i32 %153, 57
  %155 = zext i1 %154 to i32
  %156 = icmp ne i32 %155, 0
  br label %157

157:                                              ; preds = %150, %143
  %158 = phi i1 [ %149, %143 ], [ %156, %150 ]
  %159 = zext i1 %158 to i32
  %160 = icmp ne i32 %159, 0
  br i1 %160, label %201, label %161

161:                                              ; preds = %157
  %162 = load ptr, ptr @src, align 8
  %163 = load i8, ptr %162, align 1
  %164 = sext i8 %163 to i32
  %165 = icmp sge i32 %164, 65
  %166 = zext i1 %165 to i32
  %167 = icmp ne i32 %166, 0
  br i1 %167, label %168, label %175

168:                                              ; preds = %161
  %169 = load ptr, ptr @src, align 8
  %170 = load i8, ptr %169, align 1
  %171 = sext i8 %170 to i32
  %172 = icmp sle i32 %171, 90
  %173 = zext i1 %172 to i32
  %174 = icmp ne i32 %173, 0
  br label %175

175:                                              ; preds = %168, %161
  %176 = phi i1 [ %167, %161 ], [ %174, %168 ]
  %177 = zext i1 %176 to i32
  %178 = icmp ne i32 %177, 0
  br i1 %178, label %197, label %179

179:                                              ; preds = %175
  %180 = load ptr, ptr @src, align 8
  %181 = load i8, ptr %180, align 1
  %182 = sext i8 %181 to i32
  %183 = icmp sge i32 %182, 97
  %184 = zext i1 %183 to i32
  %185 = icmp ne i32 %184, 0
  br i1 %185, label %186, label %193

186:                                              ; preds = %179
  %187 = load ptr, ptr @src, align 8
  %188 = load i8, ptr %187, align 1
  %189 = sext i8 %188 to i32
  %190 = icmp sle i32 %189, 122
  %191 = zext i1 %190 to i32
  %192 = icmp ne i32 %191, 0
  br label %193

193:                                              ; preds = %186, %179
  %194 = phi i1 [ %185, %179 ], [ %192, %186 ]
  %195 = zext i1 %194 to i32
  %196 = icmp ne i32 %195, 0
  br label %197

197:                                              ; preds = %193, %175
  %198 = phi i1 [ %178, %175 ], [ %196, %193 ]
  %199 = zext i1 %198 to i32
  %200 = icmp ne i32 %199, 0
  br label %201

201:                                              ; preds = %197, %157
  %202 = phi i1 [ %160, %157 ], [ %200, %197 ]
  %203 = zext i1 %202 to i32
  %204 = icmp ne i32 %203, 0
  br label %205

205:                                              ; preds = %201, %136
  %206 = phi i1 [ %142, %136 ], [ %204, %201 ]
  %207 = zext i1 %206 to i32
  %208 = icmp ne i32 %207, 0
  br i1 %208, label %126, label %209

209:                                              ; preds = %205
  %210 = load ptr, ptr @symbols, align 8
  store ptr %210, ptr @current_id, align 8
  br label %245

211:                                              ; preds = %245
  %212 = load ptr, ptr @current_id, align 8
  %213 = getelementptr inbounds i64, ptr %212, i32 1
  %214 = load i64, ptr %213, align 8
  %215 = load i64, ptr %2, align 8
  %216 = icmp eq i64 %214, %215
  %217 = zext i1 %216 to i32
  %218 = icmp ne i32 %217, 0
  br i1 %218, label %219, label %234

219:                                              ; preds = %211
  %220 = load ptr, ptr @current_id, align 8
  %221 = getelementptr inbounds i64, ptr %220, i32 2
  %222 = load i64, ptr %221, align 8
  %223 = inttoptr i64 %222 to ptr
  %224 = load ptr, ptr %1, align 8
  %225 = load ptr, ptr @src, align 8
  %226 = ptrtoint ptr %225 to i64
  %227 = load ptr, ptr %1, align 8
  %228 = ptrtoint ptr %227 to i64
  %229 = sub i64 %226, %228
  %230 = call i32 @memcmp(ptr %223, ptr %224, i64 %229)
  %231 = icmp eq i32 %230, 0
  %232 = zext i1 %231 to i32
  %233 = icmp ne i32 %232, 0
  br label %234

234:                                              ; preds = %219, %211
  %235 = phi i1 [ %218, %211 ], [ %233, %219 ]
  %236 = zext i1 %235 to i32
  %237 = icmp ne i32 %236, 0
  br i1 %237, label %238, label %242

238:                                              ; preds = %234
  %239 = load ptr, ptr @current_id, align 8
  %240 = getelementptr inbounds i64, ptr %239, i32 0
  %241 = load i64, ptr %240, align 8
  store i64 %241, ptr @token, align 8
  ret void

242:                                              ; preds = %234
  %243 = load ptr, ptr @current_id, align 8
  %244 = getelementptr inbounds i64, ptr %243, i32 9
  store ptr %244, ptr @current_id, align 8
  br label %245

245:                                              ; preds = %242, %209
  %246 = load ptr, ptr @current_id, align 8
  %247 = getelementptr inbounds i64, ptr %246, i32 0
  %248 = load i64, ptr %247, align 8
  %249 = icmp ne i64 %248, 0
  br i1 %249, label %211, label %250

250:                                              ; preds = %245
  %251 = load ptr, ptr @current_id, align 8
  %252 = getelementptr inbounds i64, ptr %251, i32 2
  %253 = load ptr, ptr %1, align 8
  %254 = ptrtoint ptr %253 to i64
  store i64 %254, ptr %252, align 8
  %255 = load ptr, ptr @current_id, align 8
  %256 = getelementptr inbounds i64, ptr %255, i32 1
  %257 = load i64, ptr %2, align 8
  store i64 %257, ptr %256, align 8
  %258 = load ptr, ptr @current_id, align 8
  %259 = getelementptr inbounds i64, ptr %258, i32 0
  store i64 133, ptr %259, align 8
  store i64 133, ptr @token, align 8
  ret void

260:                                              ; preds = %118
  %261 = load i64, ptr @token, align 8
  %262 = icmp sge i64 %261, 48
  %263 = zext i1 %262 to i32
  %264 = icmp ne i32 %263, 0
  br i1 %264, label %265, label %270

265:                                              ; preds = %260
  %266 = load i64, ptr @token, align 8
  %267 = icmp sle i64 %266, 57
  %268 = zext i1 %267 to i32
  %269 = icmp ne i32 %268, 0
  br label %270

270:                                              ; preds = %265, %260
  %271 = phi i1 [ %264, %260 ], [ %269, %265 ]
  %272 = zext i1 %271 to i32
  %273 = icmp ne i32 %272, 0
  br i1 %273, label %274, label %437

274:                                              ; preds = %270
  %275 = load i64, ptr @token, align 8
  %276 = sub nsw i64 %275, 48
  store i64 %276, ptr @token_val, align 8
  %277 = load i64, ptr @token_val, align 8
  %278 = icmp sgt i64 %277, 0
  %279 = zext i1 %278 to i32
  %280 = icmp ne i32 %279, 0
  br i1 %280, label %281, label %311

281:                                              ; preds = %274
  br label %292

282:                                              ; preds = %306
  %283 = load i64, ptr @token_val, align 8
  %284 = mul nsw i64 %283, 10
  %285 = load ptr, ptr @src, align 8
  %286 = getelementptr inbounds i8, ptr %285, i32 1
  store ptr %286, ptr @src, align 8
  %287 = load i8, ptr %285, align 1
  %288 = sext i8 %287 to i32
  %289 = sext i32 %288 to i64
  %290 = add nsw i64 %284, %289
  %291 = sub nsw i64 %290, 48
  store i64 %291, ptr @token_val, align 8
  br label %292

292:                                              ; preds = %282, %281
  %293 = load ptr, ptr @src, align 8
  %294 = load i8, ptr %293, align 1
  %295 = sext i8 %294 to i32
  %296 = icmp sge i32 %295, 48
  %297 = zext i1 %296 to i32
  %298 = icmp ne i32 %297, 0
  br i1 %298, label %299, label %306

299:                                              ; preds = %292
  %300 = load ptr, ptr @src, align 8
  %301 = load i8, ptr %300, align 1
  %302 = sext i8 %301 to i32
  %303 = icmp sle i32 %302, 57
  %304 = zext i1 %303 to i32
  %305 = icmp ne i32 %304, 0
  br label %306

306:                                              ; preds = %299, %292
  %307 = phi i1 [ %298, %292 ], [ %305, %299 ]
  %308 = zext i1 %307 to i32
  %309 = icmp ne i32 %308, 0
  br i1 %309, label %282, label %310

310:                                              ; preds = %306
  br label %436

311:                                              ; preds = %274
  %312 = load ptr, ptr @src, align 8
  %313 = load i8, ptr %312, align 1
  %314 = sext i8 %313 to i32
  %315 = icmp eq i32 %314, 88
  %316 = zext i1 %315 to i32
  %317 = icmp ne i32 %316, 0
  br i1 %317, label %325, label %318

318:                                              ; preds = %311
  %319 = load ptr, ptr @src, align 8
  %320 = load i8, ptr %319, align 1
  %321 = sext i8 %320 to i32
  %322 = icmp eq i32 %321, 120
  %323 = zext i1 %322 to i32
  %324 = icmp ne i32 %323, 0
  br label %325

325:                                              ; preds = %318, %311
  %326 = phi i1 [ %317, %311 ], [ %324, %318 ]
  %327 = zext i1 %326 to i32
  %328 = icmp ne i32 %327, 0
  br i1 %328, label %329, label %405

329:                                              ; preds = %325
  %330 = load ptr, ptr @src, align 8
  %331 = getelementptr inbounds i8, ptr %330, i64 1
  store ptr %331, ptr @src, align 8
  %332 = load i8, ptr %331, align 1
  %333 = sext i8 %332 to i64
  store i64 %333, ptr @token, align 8
  br label %354

334:                                              ; preds = %400
  %335 = load i64, ptr @token_val, align 8
  %336 = mul nsw i64 %335, 16
  %337 = load i64, ptr @token, align 8
  %338 = and i64 %337, 15
  %339 = add nsw i64 %336, %338
  %340 = load i64, ptr @token, align 8
  %341 = icmp sge i64 %340, 65
  %342 = zext i1 %341 to i32
  %343 = icmp ne i32 %342, 0
  br i1 %343, label %344, label %345

344:                                              ; preds = %334
  br label %346

345:                                              ; preds = %334
  br label %346

346:                                              ; preds = %345, %344
  %347 = phi i32 [ 9, %344 ], [ 0, %345 ]
  %348 = sext i32 %347 to i64
  %349 = add nsw i64 %339, %348
  store i64 %349, ptr @token_val, align 8
  %350 = load ptr, ptr @src, align 8
  %351 = getelementptr inbounds i8, ptr %350, i64 1
  store ptr %351, ptr @src, align 8
  %352 = load i8, ptr %351, align 1
  %353 = sext i8 %352 to i64
  store i64 %353, ptr @token, align 8
  br label %354

354:                                              ; preds = %346, %329
  %355 = load i64, ptr @token, align 8
  %356 = icmp sge i64 %355, 65
  %357 = zext i1 %356 to i32
  %358 = icmp ne i32 %357, 0
  br i1 %358, label %359, label %364

359:                                              ; preds = %354
  %360 = load i64, ptr @token, align 8
  %361 = icmp sle i64 %360, 70
  %362 = zext i1 %361 to i32
  %363 = icmp ne i32 %362, 0
  br label %364

364:                                              ; preds = %359, %354
  %365 = phi i1 [ %358, %354 ], [ %363, %359 ]
  %366 = zext i1 %365 to i32
  %367 = icmp ne i32 %366, 0
  br i1 %367, label %400, label %368

368:                                              ; preds = %364
  %369 = load i64, ptr @token, align 8
  %370 = icmp sge i64 %369, 97
  %371 = zext i1 %370 to i32
  %372 = icmp ne i32 %371, 0
  br i1 %372, label %373, label %378

373:                                              ; preds = %368
  %374 = load i64, ptr @token, align 8
  %375 = icmp sle i64 %374, 102
  %376 = zext i1 %375 to i32
  %377 = icmp ne i32 %376, 0
  br label %378

378:                                              ; preds = %373, %368
  %379 = phi i1 [ %372, %368 ], [ %377, %373 ]
  %380 = zext i1 %379 to i32
  %381 = icmp ne i32 %380, 0
  br i1 %381, label %396, label %382

382:                                              ; preds = %378
  %383 = load i64, ptr @token, align 8
  %384 = icmp sge i64 %383, 48
  %385 = zext i1 %384 to i32
  %386 = icmp ne i32 %385, 0
  br i1 %386, label %387, label %392

387:                                              ; preds = %382
  %388 = load i64, ptr @token, align 8
  %389 = icmp sle i64 %388, 57
  %390 = zext i1 %389 to i32
  %391 = icmp ne i32 %390, 0
  br label %392

392:                                              ; preds = %387, %382
  %393 = phi i1 [ %386, %382 ], [ %391, %387 ]
  %394 = zext i1 %393 to i32
  %395 = icmp ne i32 %394, 0
  br label %396

396:                                              ; preds = %392, %378
  %397 = phi i1 [ %381, %378 ], [ %395, %392 ]
  %398 = zext i1 %397 to i32
  %399 = icmp ne i32 %398, 0
  br label %400

400:                                              ; preds = %396, %364
  %401 = phi i1 [ %367, %364 ], [ %399, %396 ]
  %402 = zext i1 %401 to i32
  %403 = icmp ne i32 %402, 0
  br i1 %403, label %334, label %404

404:                                              ; preds = %400
  br label %435

405:                                              ; preds = %325
  br label %416

406:                                              ; preds = %430
  %407 = load i64, ptr @token_val, align 8
  %408 = mul nsw i64 %407, 8
  %409 = load ptr, ptr @src, align 8
  %410 = getelementptr inbounds i8, ptr %409, i32 1
  store ptr %410, ptr @src, align 8
  %411 = load i8, ptr %409, align 1
  %412 = sext i8 %411 to i32
  %413 = sext i32 %412 to i64
  %414 = add nsw i64 %408, %413
  %415 = sub nsw i64 %414, 48
  store i64 %415, ptr @token_val, align 8
  br label %416

416:                                              ; preds = %406, %405
  %417 = load ptr, ptr @src, align 8
  %418 = load i8, ptr %417, align 1
  %419 = sext i8 %418 to i32
  %420 = icmp sge i32 %419, 48
  %421 = zext i1 %420 to i32
  %422 = icmp ne i32 %421, 0
  br i1 %422, label %423, label %430

423:                                              ; preds = %416
  %424 = load ptr, ptr @src, align 8
  %425 = load i8, ptr %424, align 1
  %426 = sext i8 %425 to i32
  %427 = icmp sle i32 %426, 55
  %428 = zext i1 %427 to i32
  %429 = icmp ne i32 %428, 0
  br label %430

430:                                              ; preds = %423, %416
  %431 = phi i1 [ %422, %416 ], [ %429, %423 ]
  %432 = zext i1 %431 to i32
  %433 = icmp ne i32 %432, 0
  br i1 %433, label %406, label %434

434:                                              ; preds = %430
  br label %435

435:                                              ; preds = %434, %404
  br label %436

436:                                              ; preds = %435, %310
  store i64 128, ptr @token, align 8
  ret void

437:                                              ; preds = %270
  %438 = load i64, ptr @token, align 8
  %439 = icmp eq i64 %438, 47
  %440 = zext i1 %439 to i32
  %441 = icmp ne i32 %440, 0
  br i1 %441, label %442, label %474

442:                                              ; preds = %437
  %443 = load ptr, ptr @src, align 8
  %444 = load i8, ptr %443, align 1
  %445 = sext i8 %444 to i32
  %446 = icmp eq i32 %445, 47
  %447 = zext i1 %446 to i32
  %448 = icmp ne i32 %447, 0
  br i1 %448, label %449, label %472

449:                                              ; preds = %442
  br label %453

450:                                              ; preds = %467
  %451 = load ptr, ptr @src, align 8
  %452 = getelementptr inbounds i8, ptr %451, i64 1
  store ptr %452, ptr @src, align 8
  br label %453

453:                                              ; preds = %450, %449
  %454 = load ptr, ptr @src, align 8
  %455 = load i8, ptr %454, align 1
  %456 = sext i8 %455 to i32
  %457 = icmp ne i32 %456, 0
  %458 = zext i1 %457 to i32
  %459 = icmp ne i32 %458, 0
  br i1 %459, label %460, label %467

460:                                              ; preds = %453
  %461 = load ptr, ptr @src, align 8
  %462 = load i8, ptr %461, align 1
  %463 = sext i8 %462 to i32
  %464 = icmp ne i32 %463, 10
  %465 = zext i1 %464 to i32
  %466 = icmp ne i32 %465, 0
  br label %467

467:                                              ; preds = %460, %453
  %468 = phi i1 [ %459, %453 ], [ %466, %460 ]
  %469 = zext i1 %468 to i32
  %470 = icmp ne i32 %469, 0
  br i1 %470, label %450, label %471

471:                                              ; preds = %467
  br label %473

472:                                              ; preds = %442
  store i64 160, ptr @token, align 8
  ret void

473:                                              ; preds = %471
  br label %833

474:                                              ; preds = %437
  %475 = load i64, ptr @token, align 8
  %476 = icmp eq i64 %475, 39
  %477 = zext i1 %476 to i32
  %478 = icmp ne i32 %477, 0
  br i1 %478, label %484, label %479

479:                                              ; preds = %474
  %480 = load i64, ptr @token, align 8
  %481 = icmp eq i64 %480, 34
  %482 = zext i1 %481 to i32
  %483 = icmp ne i32 %482, 0
  br label %484

484:                                              ; preds = %479, %474
  %485 = phi i1 [ %478, %474 ], [ %483, %479 ]
  %486 = zext i1 %485 to i32
  %487 = icmp ne i32 %486, 0
  br i1 %487, label %488, label %553

488:                                              ; preds = %484
  %489 = load ptr, ptr @data, align 8
  store ptr %489, ptr %1, align 8
  br label %521

490:                                              ; preds = %537
  %491 = load ptr, ptr @src, align 8
  %492 = getelementptr inbounds i8, ptr %491, i32 1
  store ptr %492, ptr @src, align 8
  %493 = load i8, ptr %491, align 1
  %494 = sext i8 %493 to i64
  store i64 %494, ptr @token_val, align 8
  %495 = load i64, ptr @token_val, align 8
  %496 = icmp eq i64 %495, 92
  %497 = zext i1 %496 to i32
  %498 = icmp ne i32 %497, 0
  br i1 %498, label %499, label %510

499:                                              ; preds = %490
  %500 = load ptr, ptr @src, align 8
  %501 = getelementptr inbounds i8, ptr %500, i32 1
  store ptr %501, ptr @src, align 8
  %502 = load i8, ptr %500, align 1
  %503 = sext i8 %502 to i64
  store i64 %503, ptr @token_val, align 8
  %504 = load i64, ptr @token_val, align 8
  %505 = icmp eq i64 %504, 110
  %506 = zext i1 %505 to i32
  %507 = icmp ne i32 %506, 0
  br i1 %507, label %508, label %509

508:                                              ; preds = %499
  store i64 10, ptr @token_val, align 8
  br label %509

509:                                              ; preds = %508, %499
  br label %510

510:                                              ; preds = %509, %490
  %511 = load i64, ptr @token, align 8
  %512 = icmp eq i64 %511, 34
  %513 = zext i1 %512 to i32
  %514 = icmp ne i32 %513, 0
  br i1 %514, label %515, label %520

515:                                              ; preds = %510
  %516 = load ptr, ptr @data, align 8
  %517 = getelementptr inbounds i8, ptr %516, i32 1
  store ptr %517, ptr @data, align 8
  %518 = load i64, ptr @token_val, align 8
  %519 = trunc i64 %518 to i8
  store i8 %519, ptr %516, align 1
  br label %520

520:                                              ; preds = %515, %510
  br label %521

521:                                              ; preds = %520, %488
  %522 = load ptr, ptr @src, align 8
  %523 = load i8, ptr %522, align 1
  %524 = sext i8 %523 to i32
  %525 = icmp ne i32 %524, 0
  %526 = zext i1 %525 to i32
  %527 = icmp ne i32 %526, 0
  br i1 %527, label %528, label %537

528:                                              ; preds = %521
  %529 = load ptr, ptr @src, align 8
  %530 = load i8, ptr %529, align 1
  %531 = sext i8 %530 to i32
  %532 = sext i32 %531 to i64
  %533 = load i64, ptr @token, align 8
  %534 = icmp ne i64 %532, %533
  %535 = zext i1 %534 to i32
  %536 = icmp ne i32 %535, 0
  br label %537

537:                                              ; preds = %528, %521
  %538 = phi i1 [ %527, %521 ], [ %536, %528 ]
  %539 = zext i1 %538 to i32
  %540 = icmp ne i32 %539, 0
  br i1 %540, label %490, label %541

541:                                              ; preds = %537
  %542 = load ptr, ptr @src, align 8
  %543 = getelementptr inbounds i8, ptr %542, i32 1
  store ptr %543, ptr @src, align 8
  %544 = load i64, ptr @token, align 8
  %545 = icmp eq i64 %544, 34
  %546 = zext i1 %545 to i32
  %547 = icmp ne i32 %546, 0
  br i1 %547, label %548, label %551

548:                                              ; preds = %541
  %549 = load ptr, ptr %1, align 8
  %550 = ptrtoint ptr %549 to i64
  store i64 %550, ptr @token_val, align 8
  br label %552

551:                                              ; preds = %541
  store i64 128, ptr @token, align 8
  br label %552

552:                                              ; preds = %551, %548
  ret void

553:                                              ; preds = %484
  %554 = load i64, ptr @token, align 8
  %555 = icmp eq i64 %554, 61
  %556 = zext i1 %555 to i32
  %557 = icmp ne i32 %556, 0
  br i1 %557, label %558, label %570

558:                                              ; preds = %553
  %559 = load ptr, ptr @src, align 8
  %560 = load i8, ptr %559, align 1
  %561 = sext i8 %560 to i32
  %562 = icmp eq i32 %561, 61
  %563 = zext i1 %562 to i32
  %564 = icmp ne i32 %563, 0
  br i1 %564, label %565, label %568

565:                                              ; preds = %558
  %566 = load ptr, ptr @src, align 8
  %567 = getelementptr inbounds i8, ptr %566, i32 1
  store ptr %567, ptr @src, align 8
  store i64 149, ptr @token, align 8
  br label %569

568:                                              ; preds = %558
  store i64 142, ptr @token, align 8
  br label %569

569:                                              ; preds = %568, %565
  ret void

570:                                              ; preds = %553
  %571 = load i64, ptr @token, align 8
  %572 = icmp eq i64 %571, 43
  %573 = zext i1 %572 to i32
  %574 = icmp ne i32 %573, 0
  br i1 %574, label %575, label %587

575:                                              ; preds = %570
  %576 = load ptr, ptr @src, align 8
  %577 = load i8, ptr %576, align 1
  %578 = sext i8 %577 to i32
  %579 = icmp eq i32 %578, 43
  %580 = zext i1 %579 to i32
  %581 = icmp ne i32 %580, 0
  br i1 %581, label %582, label %585

582:                                              ; preds = %575
  %583 = load ptr, ptr @src, align 8
  %584 = getelementptr inbounds i8, ptr %583, i32 1
  store ptr %584, ptr @src, align 8
  store i64 162, ptr @token, align 8
  br label %586

585:                                              ; preds = %575
  store i64 157, ptr @token, align 8
  br label %586

586:                                              ; preds = %585, %582
  ret void

587:                                              ; preds = %570
  %588 = load i64, ptr @token, align 8
  %589 = icmp eq i64 %588, 45
  %590 = zext i1 %589 to i32
  %591 = icmp ne i32 %590, 0
  br i1 %591, label %592, label %604

592:                                              ; preds = %587
  %593 = load ptr, ptr @src, align 8
  %594 = load i8, ptr %593, align 1
  %595 = sext i8 %594 to i32
  %596 = icmp eq i32 %595, 45
  %597 = zext i1 %596 to i32
  %598 = icmp ne i32 %597, 0
  br i1 %598, label %599, label %602

599:                                              ; preds = %592
  %600 = load ptr, ptr @src, align 8
  %601 = getelementptr inbounds i8, ptr %600, i32 1
  store ptr %601, ptr @src, align 8
  store i64 163, ptr @token, align 8
  br label %603

602:                                              ; preds = %592
  store i64 158, ptr @token, align 8
  br label %603

603:                                              ; preds = %602, %599
  ret void

604:                                              ; preds = %587
  %605 = load i64, ptr @token, align 8
  %606 = icmp eq i64 %605, 33
  %607 = zext i1 %606 to i32
  %608 = icmp ne i32 %607, 0
  br i1 %608, label %609, label %620

609:                                              ; preds = %604
  %610 = load ptr, ptr @src, align 8
  %611 = load i8, ptr %610, align 1
  %612 = sext i8 %611 to i32
  %613 = icmp eq i32 %612, 61
  %614 = zext i1 %613 to i32
  %615 = icmp ne i32 %614, 0
  br i1 %615, label %616, label %619

616:                                              ; preds = %609
  %617 = load ptr, ptr @src, align 8
  %618 = getelementptr inbounds i8, ptr %617, i32 1
  store ptr %618, ptr @src, align 8
  store i64 150, ptr @token, align 8
  br label %619

619:                                              ; preds = %616, %609
  ret void

620:                                              ; preds = %604
  %621 = load i64, ptr @token, align 8
  %622 = icmp eq i64 %621, 60
  %623 = zext i1 %622 to i32
  %624 = icmp ne i32 %623, 0
  br i1 %624, label %625, label %648

625:                                              ; preds = %620
  %626 = load ptr, ptr @src, align 8
  %627 = load i8, ptr %626, align 1
  %628 = sext i8 %627 to i32
  %629 = icmp eq i32 %628, 61
  %630 = zext i1 %629 to i32
  %631 = icmp ne i32 %630, 0
  br i1 %631, label %632, label %635

632:                                              ; preds = %625
  %633 = load ptr, ptr @src, align 8
  %634 = getelementptr inbounds i8, ptr %633, i32 1
  store ptr %634, ptr @src, align 8
  store i64 153, ptr @token, align 8
  br label %647

635:                                              ; preds = %625
  %636 = load ptr, ptr @src, align 8
  %637 = load i8, ptr %636, align 1
  %638 = sext i8 %637 to i32
  %639 = icmp eq i32 %638, 60
  %640 = zext i1 %639 to i32
  %641 = icmp ne i32 %640, 0
  br i1 %641, label %642, label %645

642:                                              ; preds = %635
  %643 = load ptr, ptr @src, align 8
  %644 = getelementptr inbounds i8, ptr %643, i32 1
  store ptr %644, ptr @src, align 8
  store i64 155, ptr @token, align 8
  br label %646

645:                                              ; preds = %635
  store i64 151, ptr @token, align 8
  br label %646

646:                                              ; preds = %645, %642
  br label %647

647:                                              ; preds = %646, %632
  ret void

648:                                              ; preds = %620
  %649 = load i64, ptr @token, align 8
  %650 = icmp eq i64 %649, 62
  %651 = zext i1 %650 to i32
  %652 = icmp ne i32 %651, 0
  br i1 %652, label %653, label %676

653:                                              ; preds = %648
  %654 = load ptr, ptr @src, align 8
  %655 = load i8, ptr %654, align 1
  %656 = sext i8 %655 to i32
  %657 = icmp eq i32 %656, 61
  %658 = zext i1 %657 to i32
  %659 = icmp ne i32 %658, 0
  br i1 %659, label %660, label %663

660:                                              ; preds = %653
  %661 = load ptr, ptr @src, align 8
  %662 = getelementptr inbounds i8, ptr %661, i32 1
  store ptr %662, ptr @src, align 8
  store i64 154, ptr @token, align 8
  br label %675

663:                                              ; preds = %653
  %664 = load ptr, ptr @src, align 8
  %665 = load i8, ptr %664, align 1
  %666 = sext i8 %665 to i32
  %667 = icmp eq i32 %666, 62
  %668 = zext i1 %667 to i32
  %669 = icmp ne i32 %668, 0
  br i1 %669, label %670, label %673

670:                                              ; preds = %663
  %671 = load ptr, ptr @src, align 8
  %672 = getelementptr inbounds i8, ptr %671, i32 1
  store ptr %672, ptr @src, align 8
  store i64 156, ptr @token, align 8
  br label %674

673:                                              ; preds = %663
  store i64 152, ptr @token, align 8
  br label %674

674:                                              ; preds = %673, %670
  br label %675

675:                                              ; preds = %674, %660
  ret void

676:                                              ; preds = %648
  %677 = load i64, ptr @token, align 8
  %678 = icmp eq i64 %677, 124
  %679 = zext i1 %678 to i32
  %680 = icmp ne i32 %679, 0
  br i1 %680, label %681, label %693

681:                                              ; preds = %676
  %682 = load ptr, ptr @src, align 8
  %683 = load i8, ptr %682, align 1
  %684 = sext i8 %683 to i32
  %685 = icmp eq i32 %684, 124
  %686 = zext i1 %685 to i32
  %687 = icmp ne i32 %686, 0
  br i1 %687, label %688, label %691

688:                                              ; preds = %681
  %689 = load ptr, ptr @src, align 8
  %690 = getelementptr inbounds i8, ptr %689, i32 1
  store ptr %690, ptr @src, align 8
  store i64 144, ptr @token, align 8
  br label %692

691:                                              ; preds = %681
  store i64 146, ptr @token, align 8
  br label %692

692:                                              ; preds = %691, %688
  ret void

693:                                              ; preds = %676
  %694 = load i64, ptr @token, align 8
  %695 = icmp eq i64 %694, 38
  %696 = zext i1 %695 to i32
  %697 = icmp ne i32 %696, 0
  br i1 %697, label %698, label %710

698:                                              ; preds = %693
  %699 = load ptr, ptr @src, align 8
  %700 = load i8, ptr %699, align 1
  %701 = sext i8 %700 to i32
  %702 = icmp eq i32 %701, 38
  %703 = zext i1 %702 to i32
  %704 = icmp ne i32 %703, 0
  br i1 %704, label %705, label %708

705:                                              ; preds = %698
  %706 = load ptr, ptr @src, align 8
  %707 = getelementptr inbounds i8, ptr %706, i32 1
  store ptr %707, ptr @src, align 8
  store i64 145, ptr @token, align 8
  br label %709

708:                                              ; preds = %698
  store i64 148, ptr @token, align 8
  br label %709

709:                                              ; preds = %708, %705
  ret void

710:                                              ; preds = %693
  %711 = load i64, ptr @token, align 8
  %712 = icmp eq i64 %711, 94
  %713 = zext i1 %712 to i32
  %714 = icmp ne i32 %713, 0
  br i1 %714, label %715, label %716

715:                                              ; preds = %710
  store i64 147, ptr @token, align 8
  ret void

716:                                              ; preds = %710
  %717 = load i64, ptr @token, align 8
  %718 = icmp eq i64 %717, 37
  %719 = zext i1 %718 to i32
  %720 = icmp ne i32 %719, 0
  br i1 %720, label %721, label %722

721:                                              ; preds = %716
  store i64 161, ptr @token, align 8
  ret void

722:                                              ; preds = %716
  %723 = load i64, ptr @token, align 8
  %724 = icmp eq i64 %723, 42
  %725 = zext i1 %724 to i32
  %726 = icmp ne i32 %725, 0
  br i1 %726, label %727, label %728

727:                                              ; preds = %722
  store i64 159, ptr @token, align 8
  ret void

728:                                              ; preds = %722
  %729 = load i64, ptr @token, align 8
  %730 = icmp eq i64 %729, 91
  %731 = zext i1 %730 to i32
  %732 = icmp ne i32 %731, 0
  br i1 %732, label %733, label %734

733:                                              ; preds = %728
  store i64 164, ptr @token, align 8
  ret void

734:                                              ; preds = %728
  %735 = load i64, ptr @token, align 8
  %736 = icmp eq i64 %735, 63
  %737 = zext i1 %736 to i32
  %738 = icmp ne i32 %737, 0
  br i1 %738, label %739, label %740

739:                                              ; preds = %734
  store i64 143, ptr @token, align 8
  ret void

740:                                              ; preds = %734
  %741 = load i64, ptr @token, align 8
  %742 = icmp eq i64 %741, 58
  %743 = zext i1 %742 to i32
  %744 = icmp ne i32 %743, 0
  br i1 %744, label %813, label %745

745:                                              ; preds = %740
  %746 = load i64, ptr @token, align 8
  %747 = icmp eq i64 %746, 44
  %748 = zext i1 %747 to i32
  %749 = icmp ne i32 %748, 0
  br i1 %749, label %809, label %750

750:                                              ; preds = %745
  %751 = load i64, ptr @token, align 8
  %752 = icmp eq i64 %751, 93
  %753 = zext i1 %752 to i32
  %754 = icmp ne i32 %753, 0
  br i1 %754, label %805, label %755

755:                                              ; preds = %750
  %756 = load i64, ptr @token, align 8
  %757 = icmp eq i64 %756, 41
  %758 = zext i1 %757 to i32
  %759 = icmp ne i32 %758, 0
  br i1 %759, label %801, label %760

760:                                              ; preds = %755
  %761 = load i64, ptr @token, align 8
  %762 = icmp eq i64 %761, 40
  %763 = zext i1 %762 to i32
  %764 = icmp ne i32 %763, 0
  br i1 %764, label %797, label %765

765:                                              ; preds = %760
  %766 = load i64, ptr @token, align 8
  %767 = icmp eq i64 %766, 125
  %768 = zext i1 %767 to i32
  %769 = icmp ne i32 %768, 0
  br i1 %769, label %793, label %770

770:                                              ; preds = %765
  %771 = load i64, ptr @token, align 8
  %772 = icmp eq i64 %771, 123
  %773 = zext i1 %772 to i32
  %774 = icmp ne i32 %773, 0
  br i1 %774, label %789, label %775

775:                                              ; preds = %770
  %776 = load i64, ptr @token, align 8
  %777 = icmp eq i64 %776, 59
  %778 = zext i1 %777 to i32
  %779 = icmp ne i32 %778, 0
  br i1 %779, label %785, label %780

780:                                              ; preds = %775
  %781 = load i64, ptr @token, align 8
  %782 = icmp eq i64 %781, 126
  %783 = zext i1 %782 to i32
  %784 = icmp ne i32 %783, 0
  br label %785

785:                                              ; preds = %780, %775
  %786 = phi i1 [ %779, %775 ], [ %784, %780 ]
  %787 = zext i1 %786 to i32
  %788 = icmp ne i32 %787, 0
  br label %789

789:                                              ; preds = %785, %770
  %790 = phi i1 [ %774, %770 ], [ %788, %785 ]
  %791 = zext i1 %790 to i32
  %792 = icmp ne i32 %791, 0
  br label %793

793:                                              ; preds = %789, %765
  %794 = phi i1 [ %769, %765 ], [ %792, %789 ]
  %795 = zext i1 %794 to i32
  %796 = icmp ne i32 %795, 0
  br label %797

797:                                              ; preds = %793, %760
  %798 = phi i1 [ %764, %760 ], [ %796, %793 ]
  %799 = zext i1 %798 to i32
  %800 = icmp ne i32 %799, 0
  br label %801

801:                                              ; preds = %797, %755
  %802 = phi i1 [ %759, %755 ], [ %800, %797 ]
  %803 = zext i1 %802 to i32
  %804 = icmp ne i32 %803, 0
  br label %805

805:                                              ; preds = %801, %750
  %806 = phi i1 [ %754, %750 ], [ %804, %801 ]
  %807 = zext i1 %806 to i32
  %808 = icmp ne i32 %807, 0
  br label %809

809:                                              ; preds = %805, %745
  %810 = phi i1 [ %749, %745 ], [ %808, %805 ]
  %811 = zext i1 %810 to i32
  %812 = icmp ne i32 %811, 0
  br label %813

813:                                              ; preds = %809, %740
  %814 = phi i1 [ %744, %740 ], [ %812, %809 ]
  %815 = zext i1 %814 to i32
  %816 = icmp ne i32 %815, 0
  br i1 %816, label %817, label %818

817:                                              ; preds = %813
  ret void

818:                                              ; preds = %813
  br label %819

819:                                              ; preds = %818
  br label %820

820:                                              ; preds = %819
  br label %821

821:                                              ; preds = %820
  br label %822

822:                                              ; preds = %821
  br label %823

823:                                              ; preds = %822
  br label %824

824:                                              ; preds = %823
  br label %825

825:                                              ; preds = %824
  br label %826

826:                                              ; preds = %825
  br label %827

827:                                              ; preds = %826
  br label %828

828:                                              ; preds = %827
  br label %829

829:                                              ; preds = %828
  br label %830

830:                                              ; preds = %829
  br label %831

831:                                              ; preds = %830
  br label %832

832:                                              ; preds = %831
  br label %833

833:                                              ; preds = %832, %473
  br label %834

834:                                              ; preds = %833
  br label %835

835:                                              ; preds = %834
  br label %836

836:                                              ; preds = %835, %80
  br label %837

837:                                              ; preds = %836, %50
  br label %838

838:                                              ; preds = %837, %0
  %839 = load ptr, ptr @src, align 8
  %840 = load i8, ptr %839, align 1
  %841 = sext i8 %840 to i64
  store i64 %841, ptr @token, align 8
  %842 = icmp ne i64 %841, 0
  br i1 %842, label %3, label %843

843:                                              ; preds = %838
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
  %12 = call i32 (ptr, ...) @printf(ptr @5, i64 %10, i64 %11)
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
  %12 = call i32 (ptr, ...) @printf(ptr @6, i64 %11)
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
  br label %482

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
  %44 = add nsw i64 %43, 8
  %45 = and i64 %44, -8
  %46 = inttoptr i64 %45 to ptr
  store ptr %46, ptr @data, align 8
  store i64 2, ptr @expr_type, align 8
  br label %481

47:                                               ; preds = %24
  %48 = load i64, ptr @token, align 8
  %49 = icmp eq i64 %48, 140
  %50 = zext i1 %49 to i32
  %51 = icmp ne i32 %50, 0
  br i1 %51, label %52, label %87

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
  br i1 %82, label %83, label %84

83:                                               ; preds = %74
  br label %85

84:                                               ; preds = %74
  br label %85

85:                                               ; preds = %84, %83
  %86 = phi i64 [ 1, %83 ], [ 8, %84 ]
  store i64 %86, ptr %78, align 8
  store i64 1, ptr @expr_type, align 8
  br label %480

87:                                               ; preds = %47
  %88 = load i64, ptr @token, align 8
  %89 = icmp eq i64 %88, 133
  %90 = zext i1 %89 to i32
  %91 = icmp ne i32 %90, 0
  br i1 %91, label %92, label %230

92:                                               ; preds = %87
  call void @match(i64 133)
  %93 = load ptr, ptr @current_id, align 8
  store ptr %93, ptr %3, align 8
  %94 = load i64, ptr @token, align 8
  %95 = icmp eq i64 %94, 40
  %96 = zext i1 %95 to i32
  %97 = icmp ne i32 %96, 0
  br i1 %97, label %98, label %162

98:                                               ; preds = %92
  call void @match(i64 40)
  store i64 0, ptr %4, align 8
  br label %110

99:                                               ; preds = %110
  call void @expression(i64 142)
  %100 = load ptr, ptr @text, align 8
  %101 = getelementptr inbounds i64, ptr %100, i64 1
  store ptr %101, ptr @text, align 8
  store i64 13, ptr %101, align 8
  %102 = load i64, ptr %4, align 8
  %103 = add i64 %102, 1
  store i64 %103, ptr %4, align 8
  %104 = load i64, ptr @token, align 8
  %105 = icmp eq i64 %104, 44
  %106 = zext i1 %105 to i32
  %107 = icmp ne i32 %106, 0
  br i1 %107, label %108, label %109

108:                                              ; preds = %99
  call void @match(i64 44)
  br label %109

109:                                              ; preds = %108, %99
  br label %110

110:                                              ; preds = %109, %98
  %111 = load i64, ptr @token, align 8
  %112 = icmp ne i64 %111, 41
  %113 = zext i1 %112 to i32
  %114 = icmp ne i32 %113, 0
  br i1 %114, label %99, label %115

115:                                              ; preds = %110
  call void @match(i64 41)
  %116 = load ptr, ptr %3, align 8
  %117 = getelementptr inbounds i64, ptr %116, i32 4
  %118 = load i64, ptr %117, align 8
  %119 = icmp eq i64 %118, 130
  %120 = zext i1 %119 to i32
  %121 = icmp ne i32 %120, 0
  br i1 %121, label %122, label %128

122:                                              ; preds = %115
  %123 = load ptr, ptr @text, align 8
  %124 = getelementptr inbounds i64, ptr %123, i64 1
  store ptr %124, ptr @text, align 8
  %125 = load ptr, ptr %3, align 8
  %126 = getelementptr inbounds i64, ptr %125, i32 5
  %127 = load i64, ptr %126, align 8
  store i64 %127, ptr %124, align 8
  br label %147

128:                                              ; preds = %115
  %129 = load ptr, ptr %3, align 8
  %130 = getelementptr inbounds i64, ptr %129, i32 4
  %131 = load i64, ptr %130, align 8
  %132 = icmp eq i64 %131, 129
  %133 = zext i1 %132 to i32
  %134 = icmp ne i32 %133, 0
  br i1 %134, label %135, label %143

135:                                              ; preds = %128
  %136 = load ptr, ptr @text, align 8
  %137 = getelementptr inbounds i64, ptr %136, i64 1
  store ptr %137, ptr @text, align 8
  store i64 3, ptr %137, align 8
  %138 = load ptr, ptr @text, align 8
  %139 = getelementptr inbounds i64, ptr %138, i64 1
  store ptr %139, ptr @text, align 8
  %140 = load ptr, ptr %3, align 8
  %141 = getelementptr inbounds i64, ptr %140, i32 5
  %142 = load i64, ptr %141, align 8
  store i64 %142, ptr %139, align 8
  br label %146

143:                                              ; preds = %128
  %144 = load i64, ptr @line, align 8
  %145 = call i32 (ptr, ...) @printf(ptr @7, i64 %144)
  call void @exit(i32 -1)
  br label %146

146:                                              ; preds = %143, %135
  br label %147

147:                                              ; preds = %146, %122
  %148 = load i64, ptr %4, align 8
  %149 = icmp sgt i64 %148, 0
  %150 = zext i1 %149 to i32
  %151 = icmp ne i32 %150, 0
  br i1 %151, label %152, label %158

152:                                              ; preds = %147
  %153 = load ptr, ptr @text, align 8
  %154 = getelementptr inbounds i64, ptr %153, i64 1
  store ptr %154, ptr @text, align 8
  store i64 7, ptr %154, align 8
  %155 = load ptr, ptr @text, align 8
  %156 = getelementptr inbounds i64, ptr %155, i64 1
  store ptr %156, ptr @text, align 8
  %157 = load i64, ptr %4, align 8
  store i64 %157, ptr %156, align 8
  br label %158

158:                                              ; preds = %152, %147
  %159 = load ptr, ptr %3, align 8
  %160 = getelementptr inbounds i64, ptr %159, i32 3
  %161 = load i64, ptr %160, align 8
  store i64 %161, ptr @expr_type, align 8
  br label %229

162:                                              ; preds = %92
  %163 = load ptr, ptr %3, align 8
  %164 = getelementptr inbounds i64, ptr %163, i32 4
  %165 = load i64, ptr %164, align 8
  %166 = icmp eq i64 %165, 128
  %167 = zext i1 %166 to i32
  %168 = icmp ne i32 %167, 0
  br i1 %168, label %169, label %177

169:                                              ; preds = %162
  %170 = load ptr, ptr @text, align 8
  %171 = getelementptr inbounds i64, ptr %170, i64 1
  store ptr %171, ptr @text, align 8
  store i64 1, ptr %171, align 8
  %172 = load ptr, ptr @text, align 8
  %173 = getelementptr inbounds i64, ptr %172, i64 1
  store ptr %173, ptr @text, align 8
  %174 = load ptr, ptr %3, align 8
  %175 = getelementptr inbounds i64, ptr %174, i32 5
  %176 = load i64, ptr %175, align 8
  store i64 %176, ptr %173, align 8
  store i64 1, ptr @expr_type, align 8
  br label %228

177:                                              ; preds = %162
  %178 = load ptr, ptr %3, align 8
  %179 = getelementptr inbounds i64, ptr %178, i32 4
  %180 = load i64, ptr %179, align 8
  %181 = icmp eq i64 %180, 132
  %182 = zext i1 %181 to i32
  %183 = icmp ne i32 %182, 0
  br i1 %183, label %184, label %194

184:                                              ; preds = %177
  %185 = load ptr, ptr @text, align 8
  %186 = getelementptr inbounds i64, ptr %185, i64 1
  store ptr %186, ptr @text, align 8
  store i64 0, ptr %186, align 8
  %187 = load ptr, ptr @text, align 8
  %188 = getelementptr inbounds i64, ptr %187, i64 1
  store ptr %188, ptr @text, align 8
  %189 = load i64, ptr @index_of_bp, align 8
  %190 = load ptr, ptr %3, align 8
  %191 = getelementptr inbounds i64, ptr %190, i32 5
  %192 = load i64, ptr %191, align 8
  %193 = sub nsw i64 %189, %192
  store i64 %193, ptr %188, align 8
  br label %213

194:                                              ; preds = %177
  %195 = load ptr, ptr %3, align 8
  %196 = getelementptr inbounds i64, ptr %195, i32 4
  %197 = load i64, ptr %196, align 8
  %198 = icmp eq i64 %197, 131
  %199 = zext i1 %198 to i32
  %200 = icmp ne i32 %199, 0
  br i1 %200, label %201, label %209

201:                                              ; preds = %194
  %202 = load ptr, ptr @text, align 8
  %203 = getelementptr inbounds i64, ptr %202, i64 1
  store ptr %203, ptr @text, align 8
  store i64 1, ptr %203, align 8
  %204 = load ptr, ptr @text, align 8
  %205 = getelementptr inbounds i64, ptr %204, i64 1
  store ptr %205, ptr @text, align 8
  %206 = load ptr, ptr %3, align 8
  %207 = getelementptr inbounds i64, ptr %206, i32 5
  %208 = load i64, ptr %207, align 8
  store i64 %208, ptr %205, align 8
  br label %212

209:                                              ; preds = %194
  %210 = load i64, ptr @line, align 8
  %211 = call i32 (ptr, ...) @printf(ptr @8, i64 %210)
  call void @exit(i32 -1)
  br label %212

212:                                              ; preds = %209, %201
  br label %213

213:                                              ; preds = %212, %184
  %214 = load ptr, ptr %3, align 8
  %215 = getelementptr inbounds i64, ptr %214, i32 3
  %216 = load i64, ptr %215, align 8
  store i64 %216, ptr @expr_type, align 8
  %217 = load ptr, ptr @text, align 8
  %218 = getelementptr inbounds i64, ptr %217, i64 1
  store ptr %218, ptr @text, align 8
  %219 = load i64, ptr @expr_type, align 8
  %220 = icmp eq i64 %219, 0
  %221 = zext i1 %220 to i32
  %222 = icmp ne i32 %221, 0
  br i1 %222, label %223, label %224

223:                                              ; preds = %213
  br label %225

224:                                              ; preds = %213
  br label %225

225:                                              ; preds = %224, %223
  %226 = phi i32 [ 10, %223 ], [ 9, %224 ]
  %227 = sext i32 %226 to i64
  store i64 %227, ptr %218, align 8
  br label %228

228:                                              ; preds = %225, %169
  br label %229

229:                                              ; preds = %228, %158
  br label %479

230:                                              ; preds = %87
  %231 = load i64, ptr @token, align 8
  %232 = icmp eq i64 %231, 40
  %233 = zext i1 %232 to i32
  %234 = icmp ne i32 %233, 0
  br i1 %234, label %235, label %272

235:                                              ; preds = %230
  call void @match(i64 40)
  %236 = load i64, ptr @token, align 8
  %237 = icmp eq i64 %236, 134
  %238 = zext i1 %237 to i32
  %239 = icmp ne i32 %238, 0
  br i1 %239, label %245, label %240

240:                                              ; preds = %235
  %241 = load i64, ptr @token, align 8
  %242 = icmp eq i64 %241, 138
  %243 = zext i1 %242 to i32
  %244 = icmp ne i32 %243, 0
  br label %245

245:                                              ; preds = %240, %235
  %246 = phi i1 [ %239, %235 ], [ %244, %240 ]
  %247 = zext i1 %246 to i32
  %248 = icmp ne i32 %247, 0
  br i1 %248, label %249, label %270

249:                                              ; preds = %245
  %250 = load i64, ptr @token, align 8
  %251 = icmp eq i64 %250, 134
  %252 = zext i1 %251 to i32
  %253 = icmp ne i32 %252, 0
  br i1 %253, label %254, label %255

254:                                              ; preds = %249
  br label %256

255:                                              ; preds = %249
  br label %256

256:                                              ; preds = %255, %254
  %257 = phi i32 [ 0, %254 ], [ 1, %255 ]
  %258 = sext i32 %257 to i64
  store i64 %258, ptr %4, align 8
  %259 = load i64, ptr @token, align 8
  call void @match(i64 %259)
  br label %263

260:                                              ; preds = %263
  call void @match(i64 159)
  %261 = load i64, ptr %4, align 8
  %262 = add nsw i64 %261, 2
  store i64 %262, ptr %4, align 8
  br label %263

263:                                              ; preds = %260, %256
  %264 = load i64, ptr @token, align 8
  %265 = icmp eq i64 %264, 159
  %266 = zext i1 %265 to i32
  %267 = icmp ne i32 %266, 0
  br i1 %267, label %260, label %268

268:                                              ; preds = %263
  call void @match(i64 41)
  call void @expression(i64 162)
  %269 = load i64, ptr %4, align 8
  store i64 %269, ptr @expr_type, align 8
  br label %271

270:                                              ; preds = %245
  call void @expression(i64 142)
  call void @match(i64 41)
  br label %271

271:                                              ; preds = %270, %268
  br label %478

272:                                              ; preds = %230
  %273 = load i64, ptr @token, align 8
  %274 = icmp eq i64 %273, 159
  %275 = zext i1 %274 to i32
  %276 = icmp ne i32 %275, 0
  br i1 %276, label %277, label %300

277:                                              ; preds = %272
  call void @match(i64 159)
  call void @expression(i64 162)
  %278 = load i64, ptr @expr_type, align 8
  %279 = icmp sge i64 %278, 2
  %280 = zext i1 %279 to i32
  %281 = icmp ne i32 %280, 0
  br i1 %281, label %282, label %285

282:                                              ; preds = %277
  %283 = load i64, ptr @expr_type, align 8
  %284 = sub nsw i64 %283, 2
  store i64 %284, ptr @expr_type, align 8
  br label %288

285:                                              ; preds = %277
  %286 = load i64, ptr @line, align 8
  %287 = call i32 (ptr, ...) @printf(ptr @9, i64 %286)
  call void @exit(i32 -1)
  br label %288

288:                                              ; preds = %285, %282
  %289 = load ptr, ptr @text, align 8
  %290 = getelementptr inbounds i64, ptr %289, i64 1
  store ptr %290, ptr @text, align 8
  %291 = load i64, ptr @expr_type, align 8
  %292 = icmp eq i64 %291, 0
  %293 = zext i1 %292 to i32
  %294 = icmp ne i32 %293, 0
  br i1 %294, label %295, label %296

295:                                              ; preds = %288
  br label %297

296:                                              ; preds = %288
  br label %297

297:                                              ; preds = %296, %295
  %298 = phi i32 [ 10, %295 ], [ 9, %296 ]
  %299 = sext i32 %298 to i64
  store i64 %299, ptr %290, align 8
  br label %477

300:                                              ; preds = %272
  %301 = load i64, ptr @token, align 8
  %302 = icmp eq i64 %301, 148
  %303 = zext i1 %302 to i32
  %304 = icmp ne i32 %303, 0
  br i1 %304, label %305, label %330

305:                                              ; preds = %300
  call void @match(i64 148)
  call void @expression(i64 162)
  %306 = load ptr, ptr @text, align 8
  %307 = load i64, ptr %306, align 8
  %308 = icmp eq i64 %307, 9
  %309 = zext i1 %308 to i32
  %310 = icmp ne i32 %309, 0
  br i1 %310, label %317, label %311

311:                                              ; preds = %305
  %312 = load ptr, ptr @text, align 8
  %313 = load i64, ptr %312, align 8
  %314 = icmp eq i64 %313, 10
  %315 = zext i1 %314 to i32
  %316 = icmp ne i32 %315, 0
  br label %317

317:                                              ; preds = %311, %305
  %318 = phi i1 [ %310, %305 ], [ %316, %311 ]
  %319 = zext i1 %318 to i32
  %320 = icmp ne i32 %319, 0
  br i1 %320, label %321, label %324

321:                                              ; preds = %317
  %322 = load ptr, ptr @text, align 8
  %323 = getelementptr inbounds i64, ptr %322, i32 -1
  store ptr %323, ptr @text, align 8
  br label %327

324:                                              ; preds = %317
  %325 = load i64, ptr @line, align 8
  %326 = call i32 (ptr, ...) @printf(ptr @10, i64 %325)
  call void @exit(i32 -1)
  br label %327

327:                                              ; preds = %324, %321
  %328 = load i64, ptr @expr_type, align 8
  %329 = add nsw i64 %328, 2
  store i64 %329, ptr @expr_type, align 8
  br label %476

330:                                              ; preds = %300
  %331 = load i64, ptr @token, align 8
  %332 = icmp eq i64 %331, 33
  %333 = zext i1 %332 to i32
  %334 = icmp ne i32 %333, 0
  br i1 %334, label %335, label %344

335:                                              ; preds = %330
  call void @match(i64 33)
  call void @expression(i64 162)
  %336 = load ptr, ptr @text, align 8
  %337 = getelementptr inbounds i64, ptr %336, i64 1
  store ptr %337, ptr @text, align 8
  store i64 13, ptr %337, align 8
  %338 = load ptr, ptr @text, align 8
  %339 = getelementptr inbounds i64, ptr %338, i64 1
  store ptr %339, ptr @text, align 8
  store i64 1, ptr %339, align 8
  %340 = load ptr, ptr @text, align 8
  %341 = getelementptr inbounds i64, ptr %340, i64 1
  store ptr %341, ptr @text, align 8
  store i64 0, ptr %341, align 8
  %342 = load ptr, ptr @text, align 8
  %343 = getelementptr inbounds i64, ptr %342, i64 1
  store ptr %343, ptr @text, align 8
  store i64 17, ptr %343, align 8
  store i64 1, ptr @expr_type, align 8
  br label %475

344:                                              ; preds = %330
  %345 = load i64, ptr @token, align 8
  %346 = icmp eq i64 %345, 126
  %347 = zext i1 %346 to i32
  %348 = icmp ne i32 %347, 0
  br i1 %348, label %349, label %358

349:                                              ; preds = %344
  call void @match(i64 126)
  call void @expression(i64 162)
  %350 = load ptr, ptr @text, align 8
  %351 = getelementptr inbounds i64, ptr %350, i64 1
  store ptr %351, ptr @text, align 8
  store i64 13, ptr %351, align 8
  %352 = load ptr, ptr @text, align 8
  %353 = getelementptr inbounds i64, ptr %352, i64 1
  store ptr %353, ptr @text, align 8
  store i64 1, ptr %353, align 8
  %354 = load ptr, ptr @text, align 8
  %355 = getelementptr inbounds i64, ptr %354, i64 1
  store ptr %355, ptr @text, align 8
  store i64 -1, ptr %355, align 8
  %356 = load ptr, ptr @text, align 8
  %357 = getelementptr inbounds i64, ptr %356, i64 1
  store ptr %357, ptr @text, align 8
  store i64 15, ptr %357, align 8
  store i64 1, ptr @expr_type, align 8
  br label %474

358:                                              ; preds = %344
  %359 = load i64, ptr @token, align 8
  %360 = icmp eq i64 %359, 157
  %361 = zext i1 %360 to i32
  %362 = icmp ne i32 %361, 0
  br i1 %362, label %363, label %364

363:                                              ; preds = %358
  call void @match(i64 157)
  call void @expression(i64 162)
  store i64 1, ptr @expr_type, align 8
  br label %473

364:                                              ; preds = %358
  %365 = load i64, ptr @token, align 8
  %366 = icmp eq i64 %365, 158
  %367 = zext i1 %366 to i32
  %368 = icmp ne i32 %367, 0
  br i1 %368, label %369, label %391

369:                                              ; preds = %364
  call void @match(i64 158)
  %370 = load i64, ptr @token, align 8
  %371 = icmp eq i64 %370, 128
  %372 = zext i1 %371 to i32
  %373 = icmp ne i32 %372, 0
  br i1 %373, label %374, label %381

374:                                              ; preds = %369
  %375 = load ptr, ptr @text, align 8
  %376 = getelementptr inbounds i64, ptr %375, i64 1
  store ptr %376, ptr @text, align 8
  store i64 1, ptr %376, align 8
  %377 = load ptr, ptr @text, align 8
  %378 = getelementptr inbounds i64, ptr %377, i64 1
  store ptr %378, ptr @text, align 8
  %379 = load i64, ptr @token_val, align 8
  %380 = sub nsw i64 0, %379
  store i64 %380, ptr %378, align 8
  call void @match(i64 128)
  br label %390

381:                                              ; preds = %369
  %382 = load ptr, ptr @text, align 8
  %383 = getelementptr inbounds i64, ptr %382, i64 1
  store ptr %383, ptr @text, align 8
  store i64 1, ptr %383, align 8
  %384 = load ptr, ptr @text, align 8
  %385 = getelementptr inbounds i64, ptr %384, i64 1
  store ptr %385, ptr @text, align 8
  store i64 -1, ptr %385, align 8
  %386 = load ptr, ptr @text, align 8
  %387 = getelementptr inbounds i64, ptr %386, i64 1
  store ptr %387, ptr @text, align 8
  store i64 13, ptr %387, align 8
  call void @expression(i64 162)
  %388 = load ptr, ptr @text, align 8
  %389 = getelementptr inbounds i64, ptr %388, i64 1
  store ptr %389, ptr @text, align 8
  store i64 27, ptr %389, align 8
  br label %390

390:                                              ; preds = %381, %374
  store i64 1, ptr @expr_type, align 8
  br label %472

391:                                              ; preds = %364
  %392 = load i64, ptr @token, align 8
  %393 = icmp eq i64 %392, 163
  %394 = zext i1 %393 to i32
  %395 = icmp ne i32 %394, 0
  br i1 %395, label %401, label %396

396:                                              ; preds = %391
  %397 = load i64, ptr @token, align 8
  %398 = icmp eq i64 %397, 162
  %399 = zext i1 %398 to i32
  %400 = icmp ne i32 %399, 0
  br label %401

401:                                              ; preds = %396, %391
  %402 = phi i1 [ %395, %391 ], [ %400, %396 ]
  %403 = zext i1 %402 to i32
  %404 = icmp ne i32 %403, 0
  br i1 %404, label %405, label %468

405:                                              ; preds = %401
  %406 = load i64, ptr @token, align 8
  store i64 %406, ptr %4, align 8
  %407 = load i64, ptr @token, align 8
  call void @match(i64 %407)
  call void @expression(i64 162)
  %408 = load ptr, ptr @text, align 8
  %409 = load i64, ptr %408, align 8
  %410 = icmp eq i64 %409, 10
  %411 = zext i1 %410 to i32
  %412 = icmp ne i32 %411, 0
  br i1 %412, label %413, label %417

413:                                              ; preds = %405
  %414 = load ptr, ptr @text, align 8
  store i64 13, ptr %414, align 8
  %415 = load ptr, ptr @text, align 8
  %416 = getelementptr inbounds i64, ptr %415, i64 1
  store ptr %416, ptr @text, align 8
  store i64 10, ptr %416, align 8
  br label %431

417:                                              ; preds = %405
  %418 = load ptr, ptr @text, align 8
  %419 = load i64, ptr %418, align 8
  %420 = icmp eq i64 %419, 9
  %421 = zext i1 %420 to i32
  %422 = icmp ne i32 %421, 0
  br i1 %422, label %423, label %427

423:                                              ; preds = %417
  %424 = load ptr, ptr @text, align 8
  store i64 13, ptr %424, align 8
  %425 = load ptr, ptr @text, align 8
  %426 = getelementptr inbounds i64, ptr %425, i64 1
  store ptr %426, ptr @text, align 8
  store i64 9, ptr %426, align 8
  br label %430

427:                                              ; preds = %417
  %428 = load i64, ptr @line, align 8
  %429 = call i32 (ptr, ...) @printf(ptr @11, i64 %428)
  call void @exit(i32 -1)
  br label %430

430:                                              ; preds = %427, %423
  br label %431

431:                                              ; preds = %430, %413
  %432 = load ptr, ptr @text, align 8
  %433 = getelementptr inbounds i64, ptr %432, i64 1
  store ptr %433, ptr @text, align 8
  store i64 13, ptr %433, align 8
  %434 = load ptr, ptr @text, align 8
  %435 = getelementptr inbounds i64, ptr %434, i64 1
  store ptr %435, ptr @text, align 8
  store i64 1, ptr %435, align 8
  %436 = load ptr, ptr @text, align 8
  %437 = getelementptr inbounds i64, ptr %436, i64 1
  store ptr %437, ptr @text, align 8
  %438 = load i64, ptr @expr_type, align 8
  %439 = icmp sgt i64 %438, 2
  %440 = zext i1 %439 to i32
  %441 = icmp ne i32 %440, 0
  br i1 %441, label %442, label %443

442:                                              ; preds = %431
  br label %444

443:                                              ; preds = %431
  br label %444

444:                                              ; preds = %443, %442
  %445 = phi i64 [ 8, %442 ], [ 1, %443 ]
  store i64 %445, ptr %437, align 8
  %446 = load ptr, ptr @text, align 8
  %447 = getelementptr inbounds i64, ptr %446, i64 1
  store ptr %447, ptr @text, align 8
  %448 = load i64, ptr %4, align 8
  %449 = icmp eq i64 %448, 162
  %450 = zext i1 %449 to i32
  %451 = icmp ne i32 %450, 0
  br i1 %451, label %452, label %453

452:                                              ; preds = %444
  br label %454

453:                                              ; preds = %444
  br label %454

454:                                              ; preds = %453, %452
  %455 = phi i32 [ 25, %452 ], [ 26, %453 ]
  %456 = sext i32 %455 to i64
  store i64 %456, ptr %447, align 8
  %457 = load ptr, ptr @text, align 8
  %458 = getelementptr inbounds i64, ptr %457, i64 1
  store ptr %458, ptr @text, align 8
  %459 = load i64, ptr @expr_type, align 8
  %460 = icmp eq i64 %459, 0
  %461 = zext i1 %460 to i32
  %462 = icmp ne i32 %461, 0
  br i1 %462, label %463, label %464

463:                                              ; preds = %454
  br label %465

464:                                              ; preds = %454
  br label %465

465:                                              ; preds = %464, %463
  %466 = phi i32 [ 12, %463 ], [ 11, %464 ]
  %467 = sext i32 %466 to i64
  store i64 %467, ptr %458, align 8
  br label %471

468:                                              ; preds = %401
  %469 = load i64, ptr @line, align 8
  %470 = call i32 (ptr, ...) @printf(ptr @12, i64 %469)
  call void @exit(i32 -1)
  br label %471

471:                                              ; preds = %468, %465
  br label %472

472:                                              ; preds = %471, %390
  br label %473

473:                                              ; preds = %472, %363
  br label %474

474:                                              ; preds = %473, %349
  br label %475

475:                                              ; preds = %474, %335
  br label %476

476:                                              ; preds = %475, %327
  br label %477

477:                                              ; preds = %476, %297
  br label %478

478:                                              ; preds = %477, %271
  br label %479

479:                                              ; preds = %478, %229
  br label %480

480:                                              ; preds = %479, %85
  br label %481

481:                                              ; preds = %480, %41
  br label %482

482:                                              ; preds = %481, %18
  br label %979

483:                                              ; preds = %979
  %484 = load i64, ptr @expr_type, align 8
  store i64 %484, ptr %4, align 8
  %485 = load i64, ptr @token, align 8
  %486 = icmp eq i64 %485, 142
  %487 = zext i1 %486 to i32
  %488 = icmp ne i32 %487, 0
  br i1 %488, label %489, label %523

489:                                              ; preds = %483
  call void @match(i64 142)
  %490 = load ptr, ptr @text, align 8
  %491 = load i64, ptr %490, align 8
  %492 = icmp eq i64 %491, 9
  %493 = zext i1 %492 to i32
  %494 = icmp ne i32 %493, 0
  br i1 %494, label %501, label %495

495:                                              ; preds = %489
  %496 = load ptr, ptr @text, align 8
  %497 = load i64, ptr %496, align 8
  %498 = icmp eq i64 %497, 10
  %499 = zext i1 %498 to i32
  %500 = icmp ne i32 %499, 0
  br label %501

501:                                              ; preds = %495, %489
  %502 = phi i1 [ %494, %489 ], [ %500, %495 ]
  %503 = zext i1 %502 to i32
  %504 = icmp ne i32 %503, 0
  br i1 %504, label %505, label %507

505:                                              ; preds = %501
  %506 = load ptr, ptr @text, align 8
  store i64 13, ptr %506, align 8
  br label %510

507:                                              ; preds = %501
  %508 = load i64, ptr @line, align 8
  %509 = call i32 (ptr, ...) @printf(ptr @13, i64 %508)
  call void @exit(i32 -1)
  br label %510

510:                                              ; preds = %507, %505
  call void @expression(i64 142)
  %511 = load i64, ptr %4, align 8
  store i64 %511, ptr @expr_type, align 8
  %512 = load ptr, ptr @text, align 8
  %513 = getelementptr inbounds i64, ptr %512, i64 1
  store ptr %513, ptr @text, align 8
  %514 = load i64, ptr @expr_type, align 8
  %515 = icmp eq i64 %514, 0
  %516 = zext i1 %515 to i32
  %517 = icmp ne i32 %516, 0
  br i1 %517, label %518, label %519

518:                                              ; preds = %510
  br label %520

519:                                              ; preds = %510
  br label %520

520:                                              ; preds = %519, %518
  %521 = phi i32 [ 12, %518 ], [ 11, %519 ]
  %522 = sext i32 %521 to i64
  store i64 %522, ptr %513, align 8
  br label %978

523:                                              ; preds = %483
  %524 = load i64, ptr @token, align 8
  %525 = icmp eq i64 %524, 143
  %526 = zext i1 %525 to i32
  %527 = icmp ne i32 %526, 0
  br i1 %527, label %528, label %554

528:                                              ; preds = %523
  call void @match(i64 143)
  %529 = load ptr, ptr @text, align 8
  %530 = getelementptr inbounds i64, ptr %529, i64 1
  store ptr %530, ptr @text, align 8
  store i64 4, ptr %530, align 8
  %531 = load ptr, ptr @text, align 8
  %532 = getelementptr inbounds i64, ptr %531, i64 1
  store ptr %532, ptr @text, align 8
  store ptr %532, ptr %5, align 8
  call void @expression(i64 142)
  %533 = load i64, ptr @token, align 8
  %534 = icmp eq i64 %533, 58
  %535 = zext i1 %534 to i32
  %536 = icmp ne i32 %535, 0
  br i1 %536, label %537, label %538

537:                                              ; preds = %528
  call void @match(i64 58)
  br label %541

538:                                              ; preds = %528
  %539 = load i64, ptr @line, align 8
  %540 = call i32 (ptr, ...) @printf(ptr @14, i64 %539)
  call void @exit(i32 -1)
  br label %541

541:                                              ; preds = %538, %537
  %542 = load ptr, ptr %5, align 8
  %543 = load ptr, ptr @text, align 8
  %544 = getelementptr inbounds i64, ptr %543, i32 3
  %545 = ptrtoint ptr %544 to i64
  store i64 %545, ptr %542, align 8
  %546 = load ptr, ptr @text, align 8
  %547 = getelementptr inbounds i64, ptr %546, i64 1
  store ptr %547, ptr @text, align 8
  store i64 2, ptr %547, align 8
  %548 = load ptr, ptr @text, align 8
  %549 = getelementptr inbounds i64, ptr %548, i64 1
  store ptr %549, ptr @text, align 8
  store ptr %549, ptr %5, align 8
  call void @expression(i64 143)
  %550 = load ptr, ptr %5, align 8
  %551 = load ptr, ptr @text, align 8
  %552 = getelementptr inbounds i64, ptr %551, i32 1
  %553 = ptrtoint ptr %552 to i64
  store i64 %553, ptr %550, align 8
  br label %977

554:                                              ; preds = %523
  %555 = load i64, ptr @token, align 8
  %556 = icmp eq i64 %555, 144
  %557 = zext i1 %556 to i32
  %558 = icmp ne i32 %557, 0
  br i1 %558, label %559, label %568

559:                                              ; preds = %554
  call void @match(i64 144)
  %560 = load ptr, ptr @text, align 8
  %561 = getelementptr inbounds i64, ptr %560, i64 1
  store ptr %561, ptr @text, align 8
  store i64 5, ptr %561, align 8
  %562 = load ptr, ptr @text, align 8
  %563 = getelementptr inbounds i64, ptr %562, i64 1
  store ptr %563, ptr @text, align 8
  store ptr %563, ptr %5, align 8
  call void @expression(i64 145)
  %564 = load ptr, ptr %5, align 8
  %565 = load ptr, ptr @text, align 8
  %566 = getelementptr inbounds i64, ptr %565, i32 1
  %567 = ptrtoint ptr %566 to i64
  store i64 %567, ptr %564, align 8
  store i64 1, ptr @expr_type, align 8
  br label %976

568:                                              ; preds = %554
  %569 = load i64, ptr @token, align 8
  %570 = icmp eq i64 %569, 145
  %571 = zext i1 %570 to i32
  %572 = icmp ne i32 %571, 0
  br i1 %572, label %573, label %582

573:                                              ; preds = %568
  call void @match(i64 145)
  %574 = load ptr, ptr @text, align 8
  %575 = getelementptr inbounds i64, ptr %574, i64 1
  store ptr %575, ptr @text, align 8
  store i64 4, ptr %575, align 8
  %576 = load ptr, ptr @text, align 8
  %577 = getelementptr inbounds i64, ptr %576, i64 1
  store ptr %577, ptr @text, align 8
  store ptr %577, ptr %5, align 8
  call void @expression(i64 146)
  %578 = load ptr, ptr %5, align 8
  %579 = load ptr, ptr @text, align 8
  %580 = getelementptr inbounds i64, ptr %579, i32 1
  %581 = ptrtoint ptr %580 to i64
  store i64 %581, ptr %578, align 8
  store i64 1, ptr @expr_type, align 8
  br label %975

582:                                              ; preds = %568
  %583 = load i64, ptr @token, align 8
  %584 = icmp eq i64 %583, 146
  %585 = zext i1 %584 to i32
  %586 = icmp ne i32 %585, 0
  br i1 %586, label %587, label %592

587:                                              ; preds = %582
  call void @match(i64 146)
  %588 = load ptr, ptr @text, align 8
  %589 = getelementptr inbounds i64, ptr %588, i64 1
  store ptr %589, ptr @text, align 8
  store i64 13, ptr %589, align 8
  call void @expression(i64 147)
  %590 = load ptr, ptr @text, align 8
  %591 = getelementptr inbounds i64, ptr %590, i64 1
  store ptr %591, ptr @text, align 8
  store i64 14, ptr %591, align 8
  store i64 1, ptr @expr_type, align 8
  br label %974

592:                                              ; preds = %582
  %593 = load i64, ptr @token, align 8
  %594 = icmp eq i64 %593, 147
  %595 = zext i1 %594 to i32
  %596 = icmp ne i32 %595, 0
  br i1 %596, label %597, label %602

597:                                              ; preds = %592
  call void @match(i64 147)
  %598 = load ptr, ptr @text, align 8
  %599 = getelementptr inbounds i64, ptr %598, i64 1
  store ptr %599, ptr @text, align 8
  store i64 13, ptr %599, align 8
  call void @expression(i64 148)
  %600 = load ptr, ptr @text, align 8
  %601 = getelementptr inbounds i64, ptr %600, i64 1
  store ptr %601, ptr @text, align 8
  store i64 15, ptr %601, align 8
  store i64 1, ptr @expr_type, align 8
  br label %973

602:                                              ; preds = %592
  %603 = load i64, ptr @token, align 8
  %604 = icmp eq i64 %603, 148
  %605 = zext i1 %604 to i32
  %606 = icmp ne i32 %605, 0
  br i1 %606, label %607, label %612

607:                                              ; preds = %602
  call void @match(i64 148)
  %608 = load ptr, ptr @text, align 8
  %609 = getelementptr inbounds i64, ptr %608, i64 1
  store ptr %609, ptr @text, align 8
  store i64 13, ptr %609, align 8
  call void @expression(i64 149)
  %610 = load ptr, ptr @text, align 8
  %611 = getelementptr inbounds i64, ptr %610, i64 1
  store ptr %611, ptr @text, align 8
  store i64 16, ptr %611, align 8
  store i64 1, ptr @expr_type, align 8
  br label %972

612:                                              ; preds = %602
  %613 = load i64, ptr @token, align 8
  %614 = icmp eq i64 %613, 149
  %615 = zext i1 %614 to i32
  %616 = icmp ne i32 %615, 0
  br i1 %616, label %617, label %622

617:                                              ; preds = %612
  call void @match(i64 149)
  %618 = load ptr, ptr @text, align 8
  %619 = getelementptr inbounds i64, ptr %618, i64 1
  store ptr %619, ptr @text, align 8
  store i64 13, ptr %619, align 8
  call void @expression(i64 150)
  %620 = load ptr, ptr @text, align 8
  %621 = getelementptr inbounds i64, ptr %620, i64 1
  store ptr %621, ptr @text, align 8
  store i64 17, ptr %621, align 8
  store i64 1, ptr @expr_type, align 8
  br label %971

622:                                              ; preds = %612
  %623 = load i64, ptr @token, align 8
  %624 = icmp eq i64 %623, 150
  %625 = zext i1 %624 to i32
  %626 = icmp ne i32 %625, 0
  br i1 %626, label %627, label %632

627:                                              ; preds = %622
  call void @match(i64 150)
  %628 = load ptr, ptr @text, align 8
  %629 = getelementptr inbounds i64, ptr %628, i64 1
  store ptr %629, ptr @text, align 8
  store i64 13, ptr %629, align 8
  call void @expression(i64 151)
  %630 = load ptr, ptr @text, align 8
  %631 = getelementptr inbounds i64, ptr %630, i64 1
  store ptr %631, ptr @text, align 8
  store i64 18, ptr %631, align 8
  store i64 1, ptr @expr_type, align 8
  br label %970

632:                                              ; preds = %622
  %633 = load i64, ptr @token, align 8
  %634 = icmp eq i64 %633, 151
  %635 = zext i1 %634 to i32
  %636 = icmp ne i32 %635, 0
  br i1 %636, label %637, label %642

637:                                              ; preds = %632
  call void @match(i64 151)
  %638 = load ptr, ptr @text, align 8
  %639 = getelementptr inbounds i64, ptr %638, i64 1
  store ptr %639, ptr @text, align 8
  store i64 13, ptr %639, align 8
  call void @expression(i64 155)
  %640 = load ptr, ptr @text, align 8
  %641 = getelementptr inbounds i64, ptr %640, i64 1
  store ptr %641, ptr @text, align 8
  store i64 19, ptr %641, align 8
  store i64 1, ptr @expr_type, align 8
  br label %969

642:                                              ; preds = %632
  %643 = load i64, ptr @token, align 8
  %644 = icmp eq i64 %643, 152
  %645 = zext i1 %644 to i32
  %646 = icmp ne i32 %645, 0
  br i1 %646, label %647, label %652

647:                                              ; preds = %642
  call void @match(i64 152)
  %648 = load ptr, ptr @text, align 8
  %649 = getelementptr inbounds i64, ptr %648, i64 1
  store ptr %649, ptr @text, align 8
  store i64 13, ptr %649, align 8
  call void @expression(i64 155)
  %650 = load ptr, ptr @text, align 8
  %651 = getelementptr inbounds i64, ptr %650, i64 1
  store ptr %651, ptr @text, align 8
  store i64 20, ptr %651, align 8
  store i64 1, ptr @expr_type, align 8
  br label %968

652:                                              ; preds = %642
  %653 = load i64, ptr @token, align 8
  %654 = icmp eq i64 %653, 153
  %655 = zext i1 %654 to i32
  %656 = icmp ne i32 %655, 0
  br i1 %656, label %657, label %662

657:                                              ; preds = %652
  call void @match(i64 153)
  %658 = load ptr, ptr @text, align 8
  %659 = getelementptr inbounds i64, ptr %658, i64 1
  store ptr %659, ptr @text, align 8
  store i64 13, ptr %659, align 8
  call void @expression(i64 155)
  %660 = load ptr, ptr @text, align 8
  %661 = getelementptr inbounds i64, ptr %660, i64 1
  store ptr %661, ptr @text, align 8
  store i64 21, ptr %661, align 8
  store i64 1, ptr @expr_type, align 8
  br label %967

662:                                              ; preds = %652
  %663 = load i64, ptr @token, align 8
  %664 = icmp eq i64 %663, 154
  %665 = zext i1 %664 to i32
  %666 = icmp ne i32 %665, 0
  br i1 %666, label %667, label %672

667:                                              ; preds = %662
  call void @match(i64 154)
  %668 = load ptr, ptr @text, align 8
  %669 = getelementptr inbounds i64, ptr %668, i64 1
  store ptr %669, ptr @text, align 8
  store i64 13, ptr %669, align 8
  call void @expression(i64 155)
  %670 = load ptr, ptr @text, align 8
  %671 = getelementptr inbounds i64, ptr %670, i64 1
  store ptr %671, ptr @text, align 8
  store i64 22, ptr %671, align 8
  store i64 1, ptr @expr_type, align 8
  br label %966

672:                                              ; preds = %662
  %673 = load i64, ptr @token, align 8
  %674 = icmp eq i64 %673, 155
  %675 = zext i1 %674 to i32
  %676 = icmp ne i32 %675, 0
  br i1 %676, label %677, label %682

677:                                              ; preds = %672
  call void @match(i64 155)
  %678 = load ptr, ptr @text, align 8
  %679 = getelementptr inbounds i64, ptr %678, i64 1
  store ptr %679, ptr @text, align 8
  store i64 13, ptr %679, align 8
  call void @expression(i64 157)
  %680 = load ptr, ptr @text, align 8
  %681 = getelementptr inbounds i64, ptr %680, i64 1
  store ptr %681, ptr @text, align 8
  store i64 23, ptr %681, align 8
  store i64 1, ptr @expr_type, align 8
  br label %965

682:                                              ; preds = %672
  %683 = load i64, ptr @token, align 8
  %684 = icmp eq i64 %683, 156
  %685 = zext i1 %684 to i32
  %686 = icmp ne i32 %685, 0
  br i1 %686, label %687, label %692

687:                                              ; preds = %682
  call void @match(i64 156)
  %688 = load ptr, ptr @text, align 8
  %689 = getelementptr inbounds i64, ptr %688, i64 1
  store ptr %689, ptr @text, align 8
  store i64 13, ptr %689, align 8
  call void @expression(i64 157)
  %690 = load ptr, ptr @text, align 8
  %691 = getelementptr inbounds i64, ptr %690, i64 1
  store ptr %691, ptr @text, align 8
  store i64 24, ptr %691, align 8
  store i64 1, ptr @expr_type, align 8
  br label %964

692:                                              ; preds = %682
  %693 = load i64, ptr @token, align 8
  %694 = icmp eq i64 %693, 157
  %695 = zext i1 %694 to i32
  %696 = icmp ne i32 %695, 0
  br i1 %696, label %697, label %717

697:                                              ; preds = %692
  call void @match(i64 157)
  %698 = load ptr, ptr @text, align 8
  %699 = getelementptr inbounds i64, ptr %698, i64 1
  store ptr %699, ptr @text, align 8
  store i64 13, ptr %699, align 8
  call void @expression(i64 159)
  %700 = load i64, ptr %4, align 8
  store i64 %700, ptr @expr_type, align 8
  %701 = load i64, ptr @expr_type, align 8
  %702 = icmp sgt i64 %701, 2
  %703 = zext i1 %702 to i32
  %704 = icmp ne i32 %703, 0
  br i1 %704, label %705, label %714

705:                                              ; preds = %697
  %706 = load ptr, ptr @text, align 8
  %707 = getelementptr inbounds i64, ptr %706, i64 1
  store ptr %707, ptr @text, align 8
  store i64 13, ptr %707, align 8
  %708 = load ptr, ptr @text, align 8
  %709 = getelementptr inbounds i64, ptr %708, i64 1
  store ptr %709, ptr @text, align 8
  store i64 1, ptr %709, align 8
  %710 = load ptr, ptr @text, align 8
  %711 = getelementptr inbounds i64, ptr %710, i64 1
  store ptr %711, ptr @text, align 8
  store i64 8, ptr %711, align 8
  %712 = load ptr, ptr @text, align 8
  %713 = getelementptr inbounds i64, ptr %712, i64 1
  store ptr %713, ptr @text, align 8
  store i64 27, ptr %713, align 8
  br label %714

714:                                              ; preds = %705, %697
  %715 = load ptr, ptr @text, align 8
  %716 = getelementptr inbounds i64, ptr %715, i64 1
  store ptr %716, ptr @text, align 8
  store i64 25, ptr %716, align 8
  br label %963

717:                                              ; preds = %692
  %718 = load i64, ptr @token, align 8
  %719 = icmp eq i64 %718, 158
  %720 = zext i1 %719 to i32
  %721 = icmp ne i32 %720, 0
  br i1 %721, label %722, label %773

722:                                              ; preds = %717
  call void @match(i64 158)
  %723 = load ptr, ptr @text, align 8
  %724 = getelementptr inbounds i64, ptr %723, i64 1
  store ptr %724, ptr @text, align 8
  store i64 13, ptr %724, align 8
  call void @expression(i64 159)
  %725 = load i64, ptr %4, align 8
  %726 = icmp sgt i64 %725, 2
  %727 = zext i1 %726 to i32
  %728 = icmp ne i32 %727, 0
  br i1 %728, label %729, label %735

729:                                              ; preds = %722
  %730 = load i64, ptr %4, align 8
  %731 = load i64, ptr @expr_type, align 8
  %732 = icmp eq i64 %730, %731
  %733 = zext i1 %732 to i32
  %734 = icmp ne i32 %733, 0
  br label %735

735:                                              ; preds = %729, %722
  %736 = phi i1 [ %728, %722 ], [ %734, %729 ]
  %737 = zext i1 %736 to i32
  %738 = icmp ne i32 %737, 0
  br i1 %738, label %739, label %750

739:                                              ; preds = %735
  %740 = load ptr, ptr @text, align 8
  %741 = getelementptr inbounds i64, ptr %740, i64 1
  store ptr %741, ptr @text, align 8
  store i64 26, ptr %741, align 8
  %742 = load ptr, ptr @text, align 8
  %743 = getelementptr inbounds i64, ptr %742, i64 1
  store ptr %743, ptr @text, align 8
  store i64 13, ptr %743, align 8
  %744 = load ptr, ptr @text, align 8
  %745 = getelementptr inbounds i64, ptr %744, i64 1
  store ptr %745, ptr @text, align 8
  store i64 1, ptr %745, align 8
  %746 = load ptr, ptr @text, align 8
  %747 = getelementptr inbounds i64, ptr %746, i64 1
  store ptr %747, ptr @text, align 8
  store i64 8, ptr %747, align 8
  %748 = load ptr, ptr @text, align 8
  %749 = getelementptr inbounds i64, ptr %748, i64 1
  store ptr %749, ptr @text, align 8
  store i64 28, ptr %749, align 8
  store i64 1, ptr @expr_type, align 8
  br label %772

750:                                              ; preds = %735
  %751 = load i64, ptr %4, align 8
  %752 = icmp sgt i64 %751, 2
  %753 = zext i1 %752 to i32
  %754 = icmp ne i32 %753, 0
  br i1 %754, label %755, label %767

755:                                              ; preds = %750
  %756 = load ptr, ptr @text, align 8
  %757 = getelementptr inbounds i64, ptr %756, i64 1
  store ptr %757, ptr @text, align 8
  store i64 13, ptr %757, align 8
  %758 = load ptr, ptr @text, align 8
  %759 = getelementptr inbounds i64, ptr %758, i64 1
  store ptr %759, ptr @text, align 8
  store i64 1, ptr %759, align 8
  %760 = load ptr, ptr @text, align 8
  %761 = getelementptr inbounds i64, ptr %760, i64 1
  store ptr %761, ptr @text, align 8
  store i64 8, ptr %761, align 8
  %762 = load ptr, ptr @text, align 8
  %763 = getelementptr inbounds i64, ptr %762, i64 1
  store ptr %763, ptr @text, align 8
  store i64 27, ptr %763, align 8
  %764 = load ptr, ptr @text, align 8
  %765 = getelementptr inbounds i64, ptr %764, i64 1
  store ptr %765, ptr @text, align 8
  store i64 26, ptr %765, align 8
  %766 = load i64, ptr %4, align 8
  store i64 %766, ptr @expr_type, align 8
  br label %771

767:                                              ; preds = %750
  %768 = load ptr, ptr @text, align 8
  %769 = getelementptr inbounds i64, ptr %768, i64 1
  store ptr %769, ptr @text, align 8
  store i64 26, ptr %769, align 8
  %770 = load i64, ptr %4, align 8
  store i64 %770, ptr @expr_type, align 8
  br label %771

771:                                              ; preds = %767, %755
  br label %772

772:                                              ; preds = %771, %739
  br label %962

773:                                              ; preds = %717
  %774 = load i64, ptr @token, align 8
  %775 = icmp eq i64 %774, 159
  %776 = zext i1 %775 to i32
  %777 = icmp ne i32 %776, 0
  br i1 %777, label %778, label %784

778:                                              ; preds = %773
  call void @match(i64 159)
  %779 = load ptr, ptr @text, align 8
  %780 = getelementptr inbounds i64, ptr %779, i64 1
  store ptr %780, ptr @text, align 8
  store i64 13, ptr %780, align 8
  call void @expression(i64 162)
  %781 = load ptr, ptr @text, align 8
  %782 = getelementptr inbounds i64, ptr %781, i64 1
  store ptr %782, ptr @text, align 8
  store i64 27, ptr %782, align 8
  %783 = load i64, ptr %4, align 8
  store i64 %783, ptr @expr_type, align 8
  br label %961

784:                                              ; preds = %773
  %785 = load i64, ptr @token, align 8
  %786 = icmp eq i64 %785, 160
  %787 = zext i1 %786 to i32
  %788 = icmp ne i32 %787, 0
  br i1 %788, label %789, label %795

789:                                              ; preds = %784
  call void @match(i64 160)
  %790 = load ptr, ptr @text, align 8
  %791 = getelementptr inbounds i64, ptr %790, i64 1
  store ptr %791, ptr @text, align 8
  store i64 13, ptr %791, align 8
  call void @expression(i64 162)
  %792 = load ptr, ptr @text, align 8
  %793 = getelementptr inbounds i64, ptr %792, i64 1
  store ptr %793, ptr @text, align 8
  store i64 28, ptr %793, align 8
  %794 = load i64, ptr %4, align 8
  store i64 %794, ptr @expr_type, align 8
  br label %960

795:                                              ; preds = %784
  %796 = load i64, ptr @token, align 8
  %797 = icmp eq i64 %796, 161
  %798 = zext i1 %797 to i32
  %799 = icmp ne i32 %798, 0
  br i1 %799, label %800, label %806

800:                                              ; preds = %795
  call void @match(i64 161)
  %801 = load ptr, ptr @text, align 8
  %802 = getelementptr inbounds i64, ptr %801, i64 1
  store ptr %802, ptr @text, align 8
  store i64 13, ptr %802, align 8
  call void @expression(i64 162)
  %803 = load ptr, ptr @text, align 8
  %804 = getelementptr inbounds i64, ptr %803, i64 1
  store ptr %804, ptr @text, align 8
  store i64 29, ptr %804, align 8
  %805 = load i64, ptr %4, align 8
  store i64 %805, ptr @expr_type, align 8
  br label %959

806:                                              ; preds = %795
  %807 = load i64, ptr @token, align 8
  %808 = icmp eq i64 %807, 163
  %809 = zext i1 %808 to i32
  %810 = icmp ne i32 %809, 0
  br i1 %810, label %816, label %811

811:                                              ; preds = %806
  %812 = load i64, ptr @token, align 8
  %813 = icmp eq i64 %812, 162
  %814 = zext i1 %813 to i32
  %815 = icmp ne i32 %814, 0
  br label %816

816:                                              ; preds = %811, %806
  %817 = phi i1 [ %810, %806 ], [ %815, %811 ]
  %818 = zext i1 %817 to i32
  %819 = icmp ne i32 %818, 0
  br i1 %819, label %820, label %907

820:                                              ; preds = %816
  %821 = load ptr, ptr @text, align 8
  %822 = load i64, ptr %821, align 8
  %823 = icmp eq i64 %822, 9
  %824 = zext i1 %823 to i32
  %825 = icmp ne i32 %824, 0
  br i1 %825, label %826, label %830

826:                                              ; preds = %820
  %827 = load ptr, ptr @text, align 8
  store i64 13, ptr %827, align 8
  %828 = load ptr, ptr @text, align 8
  %829 = getelementptr inbounds i64, ptr %828, i64 1
  store ptr %829, ptr @text, align 8
  store i64 9, ptr %829, align 8
  br label %844

830:                                              ; preds = %820
  %831 = load ptr, ptr @text, align 8
  %832 = load i64, ptr %831, align 8
  %833 = icmp eq i64 %832, 10
  %834 = zext i1 %833 to i32
  %835 = icmp ne i32 %834, 0
  br i1 %835, label %836, label %840

836:                                              ; preds = %830
  %837 = load ptr, ptr @text, align 8
  store i64 13, ptr %837, align 8
  %838 = load ptr, ptr @text, align 8
  %839 = getelementptr inbounds i64, ptr %838, i64 1
  store ptr %839, ptr @text, align 8
  store i64 10, ptr %839, align 8
  br label %843

840:                                              ; preds = %830
  %841 = load i64, ptr @line, align 8
  %842 = call i32 (ptr, ...) @printf(ptr @15, i64 %841)
  call void @exit(i32 -1)
  br label %843

843:                                              ; preds = %840, %836
  br label %844

844:                                              ; preds = %843, %826
  %845 = load ptr, ptr @text, align 8
  %846 = getelementptr inbounds i64, ptr %845, i64 1
  store ptr %846, ptr @text, align 8
  store i64 13, ptr %846, align 8
  %847 = load ptr, ptr @text, align 8
  %848 = getelementptr inbounds i64, ptr %847, i64 1
  store ptr %848, ptr @text, align 8
  store i64 1, ptr %848, align 8
  %849 = load ptr, ptr @text, align 8
  %850 = getelementptr inbounds i64, ptr %849, i64 1
  store ptr %850, ptr @text, align 8
  %851 = load i64, ptr @expr_type, align 8
  %852 = icmp sgt i64 %851, 2
  %853 = zext i1 %852 to i32
  %854 = icmp ne i32 %853, 0
  br i1 %854, label %855, label %856

855:                                              ; preds = %844
  br label %857

856:                                              ; preds = %844
  br label %857

857:                                              ; preds = %856, %855
  %858 = phi i64 [ 8, %855 ], [ 1, %856 ]
  store i64 %858, ptr %850, align 8
  %859 = load ptr, ptr @text, align 8
  %860 = getelementptr inbounds i64, ptr %859, i64 1
  store ptr %860, ptr @text, align 8
  %861 = load i64, ptr @token, align 8
  %862 = icmp eq i64 %861, 162
  %863 = zext i1 %862 to i32
  %864 = icmp ne i32 %863, 0
  br i1 %864, label %865, label %866

865:                                              ; preds = %857
  br label %867

866:                                              ; preds = %857
  br label %867

867:                                              ; preds = %866, %865
  %868 = phi i32 [ 25, %865 ], [ 26, %866 ]
  %869 = sext i32 %868 to i64
  store i64 %869, ptr %860, align 8
  %870 = load ptr, ptr @text, align 8
  %871 = getelementptr inbounds i64, ptr %870, i64 1
  store ptr %871, ptr @text, align 8
  %872 = load i64, ptr @expr_type, align 8
  %873 = icmp eq i64 %872, 0
  %874 = zext i1 %873 to i32
  %875 = icmp ne i32 %874, 0
  br i1 %875, label %876, label %877

876:                                              ; preds = %867
  br label %878

877:                                              ; preds = %867
  br label %878

878:                                              ; preds = %877, %876
  %879 = phi i32 [ 12, %876 ], [ 11, %877 ]
  %880 = sext i32 %879 to i64
  store i64 %880, ptr %871, align 8
  %881 = load ptr, ptr @text, align 8
  %882 = getelementptr inbounds i64, ptr %881, i64 1
  store ptr %882, ptr @text, align 8
  store i64 13, ptr %882, align 8
  %883 = load ptr, ptr @text, align 8
  %884 = getelementptr inbounds i64, ptr %883, i64 1
  store ptr %884, ptr @text, align 8
  store i64 1, ptr %884, align 8
  %885 = load ptr, ptr @text, align 8
  %886 = getelementptr inbounds i64, ptr %885, i64 1
  store ptr %886, ptr @text, align 8
  %887 = load i64, ptr @expr_type, align 8
  %888 = icmp sgt i64 %887, 2
  %889 = zext i1 %888 to i32
  %890 = icmp ne i32 %889, 0
  br i1 %890, label %891, label %892

891:                                              ; preds = %878
  br label %893

892:                                              ; preds = %878
  br label %893

893:                                              ; preds = %892, %891
  %894 = phi i64 [ 8, %891 ], [ 1, %892 ]
  store i64 %894, ptr %886, align 8
  %895 = load ptr, ptr @text, align 8
  %896 = getelementptr inbounds i64, ptr %895, i64 1
  store ptr %896, ptr @text, align 8
  %897 = load i64, ptr @token, align 8
  %898 = icmp eq i64 %897, 162
  %899 = zext i1 %898 to i32
  %900 = icmp ne i32 %899, 0
  br i1 %900, label %901, label %902

901:                                              ; preds = %893
  br label %903

902:                                              ; preds = %893
  br label %903

903:                                              ; preds = %902, %901
  %904 = phi i32 [ 26, %901 ], [ 25, %902 ]
  %905 = sext i32 %904 to i64
  store i64 %905, ptr %896, align 8
  %906 = load i64, ptr @token, align 8
  call void @match(i64 %906)
  br label %958

907:                                              ; preds = %816
  %908 = load i64, ptr @token, align 8
  %909 = icmp eq i64 %908, 164
  %910 = zext i1 %909 to i32
  %911 = icmp ne i32 %910, 0
  br i1 %911, label %912, label %951

912:                                              ; preds = %907
  call void @match(i64 164)
  %913 = load ptr, ptr @text, align 8
  %914 = getelementptr inbounds i64, ptr %913, i64 1
  store ptr %914, ptr @text, align 8
  store i64 13, ptr %914, align 8
  call void @expression(i64 142)
  call void @match(i64 93)
  %915 = load i64, ptr %4, align 8
  %916 = icmp sgt i64 %915, 2
  %917 = zext i1 %916 to i32
  %918 = icmp ne i32 %917, 0
  br i1 %918, label %919, label %926

919:                                              ; preds = %912
  %920 = load ptr, ptr @text, align 8
  %921 = getelementptr inbounds i64, ptr %920, i64 1
  store ptr %921, ptr @text, align 8
  store i64 1, ptr %921, align 8
  %922 = load ptr, ptr @text, align 8
  %923 = getelementptr inbounds i64, ptr %922, i64 1
  store ptr %923, ptr @text, align 8
  store i64 8, ptr %923, align 8
  %924 = load ptr, ptr @text, align 8
  %925 = getelementptr inbounds i64, ptr %924, i64 1
  store ptr %925, ptr @text, align 8
  store i64 27, ptr %925, align 8
  br label %935

926:                                              ; preds = %912
  %927 = load i64, ptr %4, align 8
  %928 = icmp slt i64 %927, 2
  %929 = zext i1 %928 to i32
  %930 = icmp ne i32 %929, 0
  br i1 %930, label %931, label %934

931:                                              ; preds = %926
  %932 = load i64, ptr @line, align 8
  %933 = call i32 (ptr, ...) @printf(ptr @16, i64 %932)
  call void @exit(i32 -1)
  br label %934

934:                                              ; preds = %931, %926
  br label %935

935:                                              ; preds = %934, %919
  %936 = load i64, ptr %4, align 8
  %937 = sub nsw i64 %936, 2
  store i64 %937, ptr @expr_type, align 8
  %938 = load ptr, ptr @text, align 8
  %939 = getelementptr inbounds i64, ptr %938, i64 1
  store ptr %939, ptr @text, align 8
  store i64 25, ptr %939, align 8
  %940 = load ptr, ptr @text, align 8
  %941 = getelementptr inbounds i64, ptr %940, i64 1
  store ptr %941, ptr @text, align 8
  %942 = load i64, ptr @expr_type, align 8
  %943 = icmp eq i64 %942, 0
  %944 = zext i1 %943 to i32
  %945 = icmp ne i32 %944, 0
  br i1 %945, label %946, label %947

946:                                              ; preds = %935
  br label %948

947:                                              ; preds = %935
  br label %948

948:                                              ; preds = %947, %946
  %949 = phi i32 [ 10, %946 ], [ 9, %947 ]
  %950 = sext i32 %949 to i64
  store i64 %950, ptr %941, align 8
  br label %957

951:                                              ; preds = %907
  %952 = load ptr, ptr @text, align 8
  %953 = getelementptr inbounds i64, ptr %952, i64 1
  store ptr %953, ptr @text, align 8
  store i64 13, ptr %953, align 8
  %954 = load i64, ptr @line, align 8
  %955 = load i64, ptr @token, align 8
  %956 = call i32 (ptr, ...) @printf(ptr @17, i64 %954, i64 %955)
  call void @exit(i32 -1)
  br label %957

957:                                              ; preds = %951, %948
  br label %958

958:                                              ; preds = %957, %903
  br label %959

959:                                              ; preds = %958, %800
  br label %960

960:                                              ; preds = %959, %789
  br label %961

961:                                              ; preds = %960, %778
  br label %962

962:                                              ; preds = %961, %772
  br label %963

963:                                              ; preds = %962, %714
  br label %964

964:                                              ; preds = %963, %687
  br label %965

965:                                              ; preds = %964, %677
  br label %966

966:                                              ; preds = %965, %667
  br label %967

967:                                              ; preds = %966, %657
  br label %968

968:                                              ; preds = %967, %647
  br label %969

969:                                              ; preds = %968, %637
  br label %970

970:                                              ; preds = %969, %627
  br label %971

971:                                              ; preds = %970, %617
  br label %972

972:                                              ; preds = %971, %607
  br label %973

973:                                              ; preds = %972, %597
  br label %974

974:                                              ; preds = %973, %587
  br label %975

975:                                              ; preds = %974, %573
  br label %976

976:                                              ; preds = %975, %559
  br label %977

977:                                              ; preds = %976, %541
  br label %978

978:                                              ; preds = %977, %520
  br label %979

979:                                              ; preds = %978, %482
  %980 = load i64, ptr @token, align 8
  %981 = load i64, ptr %2, align 8
  %982 = icmp sge i64 %980, %981
  %983 = zext i1 %982 to i32
  %984 = icmp ne i32 %983, 0
  br i1 %984, label %483, label %985

985:                                              ; preds = %979
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
  %10 = call i32 (ptr, ...) @printf(ptr @18, i64 %8, i64 %9)
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
  %23 = call i32 (ptr, ...) @printf(ptr @19, i64 %22)
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
  %32 = call i32 (ptr, ...) @printf(ptr @20, i64 %31)
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
  %42 = call i32 (ptr, ...) @printf(ptr @21, i64 %41)
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
  br label %80

4:                                                ; preds = %90
  %5 = load i64, ptr @token, align 8
  %6 = icmp eq i64 %5, 138
  %7 = zext i1 %6 to i32
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %4
  br label %11

10:                                               ; preds = %4
  br label %11

11:                                               ; preds = %10, %9
  %12 = phi i32 [ 1, %9 ], [ 0, %10 ]
  %13 = sext i32 %12 to i64
  store i64 %13, ptr @basetype, align 8
  %14 = load i64, ptr @token, align 8
  call void @match(i64 %14)
  br label %74

15:                                               ; preds = %74
  %16 = load i64, ptr @basetype, align 8
  store i64 %16, ptr %2, align 8
  br label %20

17:                                               ; preds = %20
  call void @match(i64 159)
  %18 = load i64, ptr %2, align 8
  %19 = add nsw i64 %18, 2
  store i64 %19, ptr %2, align 8
  br label %20

20:                                               ; preds = %17, %15
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
  %32 = call i32 (ptr, ...) @printf(ptr @22, i64 %31)
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
  %42 = call i32 (ptr, ...) @printf(ptr @23, i64 %41)
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
  %58 = load i64, ptr %2, align 8
  store i64 %58, ptr %57, align 8
  %59 = load ptr, ptr @current_id, align 8
  %60 = getelementptr inbounds i64, ptr %59, i32 8
  %61 = load ptr, ptr @current_id, align 8
  %62 = getelementptr inbounds i64, ptr %61, i32 5
  %63 = load i64, ptr %62, align 8
  store i64 %63, ptr %60, align 8
  %64 = load ptr, ptr @current_id, align 8
  %65 = getelementptr inbounds i64, ptr %64, i32 5
  %66 = load i64, ptr %1, align 8
  %67 = add nsw i64 %66, 1
  store i64 %67, ptr %1, align 8
  store i64 %67, ptr %65, align 8
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

74:                                               ; preds = %73, %11
  %75 = load i64, ptr @token, align 8
  %76 = icmp ne i64 %75, 59
  %77 = zext i1 %76 to i32
  %78 = icmp ne i32 %77, 0
  br i1 %78, label %15, label %79

79:                                               ; preds = %74
  call void @match(i64 59)
  br label %80

80:                                               ; preds = %79, %0
  %81 = load i64, ptr @token, align 8
  %82 = icmp eq i64 %81, 134
  %83 = zext i1 %82 to i32
  %84 = icmp ne i32 %83, 0
  br i1 %84, label %90, label %85

85:                                               ; preds = %80
  %86 = load i64, ptr @token, align 8
  %87 = icmp eq i64 %86, 138
  %88 = zext i1 %87 to i32
  %89 = icmp ne i32 %88, 0
  br label %90

90:                                               ; preds = %85, %80
  %91 = phi i1 [ %84, %80 ], [ %89, %85 ]
  %92 = zext i1 %91 to i32
  %93 = icmp ne i32 %92, 0
  br i1 %93, label %4, label %94

94:                                               ; preds = %90
  %95 = load ptr, ptr @text, align 8
  %96 = getelementptr inbounds i64, ptr %95, i64 1
  store ptr %96, ptr @text, align 8
  store i64 6, ptr %96, align 8
  %97 = load ptr, ptr @text, align 8
  %98 = getelementptr inbounds i64, ptr %97, i64 1
  store ptr %98, ptr @text, align 8
  %99 = load i64, ptr %1, align 8
  %100 = load i64, ptr @index_of_bp, align 8
  %101 = sub nsw i64 %99, %100
  store i64 %101, ptr %98, align 8
  br label %103

102:                                              ; preds = %103
  call void (...) @statement()
  br label %103

103:                                              ; preds = %102, %94
  %104 = load i64, ptr @token, align 8
  %105 = icmp ne i64 %104, 125
  %106 = zext i1 %105 to i32
  %107 = icmp ne i32 %106, 0
  br i1 %107, label %102, label %108

108:                                              ; preds = %103
  %109 = load ptr, ptr @text, align 8
  %110 = getelementptr inbounds i64, ptr %109, i64 1
  store ptr %110, ptr @text, align 8
  store i64 8, ptr %110, align 8
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

34:                                               ; preds = %102
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
  %51 = call i32 (ptr, ...) @printf(ptr @24, i64 %50)
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
  %59 = call i32 (ptr, ...) @printf(ptr @25, i64 %58)
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
  %95 = zext i1 %94 to i32
  %96 = icmp ne i32 %95, 0
  br i1 %96, label %97, label %102

97:                                               ; preds = %92
  %98 = load i64, ptr @token, align 8
  %99 = icmp ne i64 %98, 125
  %100 = zext i1 %99 to i32
  %101 = icmp ne i32 %100, 0
  br label %102

102:                                              ; preds = %97, %92
  %103 = phi i1 [ %96, %92 ], [ %101, %97 ]
  %104 = zext i1 %103 to i32
  %105 = icmp ne i32 %104, 0
  br i1 %105, label %34, label %106

106:                                              ; preds = %102
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
  %7 = getelementptr inbounds i64, ptr %6, i32 1
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
  %15 = getelementptr inbounds [189 x i8], ptr @27, i32 0, i64 %14
  %16 = call i32 (ptr, ...) @printf(ptr @26, i64 %12, ptr %15)
  %17 = load i64, ptr %1, align 8
  %18 = icmp sle i64 %17, 7
  %19 = zext i1 %18 to i32
  %20 = icmp ne i32 %19, 0
  br i1 %20, label %21, label %25

21:                                               ; preds = %11
  %22 = load ptr, ptr @pc, align 8
  %23 = load i64, ptr %22, align 8
  %24 = call i32 (ptr, ...) @printf(ptr @28, i64 %23)
  br label %27

25:                                               ; preds = %11
  %26 = call i32 (ptr, ...) @printf(ptr @29)
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
  %35 = getelementptr inbounds i64, ptr %34, i32 1
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
  %63 = getelementptr inbounds i64, ptr %62, i32 1
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
  %76 = getelementptr inbounds i64, ptr %75, i32 1
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
  %159 = getelementptr inbounds i64, ptr %158, i32 1
  store ptr %159, ptr @pc, align 8
  %160 = load i64, ptr %158, align 8
  %161 = sub nsw i64 0, %160
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
  %171 = getelementptr inbounds i64, ptr %170, i32 1
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
  %182 = getelementptr inbounds i64, ptr %181, i32 1
  store ptr %182, ptr @sp, align 8
  %183 = load i64, ptr %181, align 8
  %184 = inttoptr i64 %183 to ptr
  store ptr %184, ptr @bp, align 8
  %185 = load ptr, ptr @sp, align 8
  %186 = getelementptr inbounds i64, ptr %185, i32 1
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
  %197 = getelementptr inbounds i64, ptr %196, i32 1
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
  %208 = getelementptr inbounds i64, ptr %207, i32 1
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
  %219 = getelementptr inbounds i64, ptr %218, i32 1
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
  %230 = getelementptr inbounds i64, ptr %229, i32 1
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
  %241 = getelementptr inbounds i64, ptr %240, i32 1
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
  %254 = getelementptr inbounds i64, ptr %253, i32 1
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
  %267 = getelementptr inbounds i64, ptr %266, i32 1
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
  %280 = getelementptr inbounds i64, ptr %279, i32 1
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
  %293 = getelementptr inbounds i64, ptr %292, i32 1
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
  %306 = getelementptr inbounds i64, ptr %305, i32 1
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
  %319 = getelementptr inbounds i64, ptr %318, i32 1
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
  %330 = getelementptr inbounds i64, ptr %329, i32 1
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
  %341 = getelementptr inbounds i64, ptr %340, i32 1
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
  %352 = getelementptr inbounds i64, ptr %351, i32 1
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
  %363 = getelementptr inbounds i64, ptr %362, i32 1
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
  %374 = getelementptr inbounds i64, ptr %373, i32 1
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
  %385 = getelementptr inbounds i64, ptr %384, i32 1
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
  %397 = call i32 (ptr, ...) @printf(ptr @30, i64 %396)
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
  %524 = call i32 (ptr, ...) @printf(ptr @31, i64 %523)
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
  %11 = getelementptr inbounds ptr, ptr %10, i32 1
  store ptr %11, ptr %4, align 8
  %12 = load i32, ptr %3, align 4
  %13 = icmp sgt i32 %12, 0
  %14 = zext i1 %13 to i32
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %16, label %24

16:                                               ; preds = %2
  %17 = load ptr, ptr %4, align 8
  %18 = load ptr, ptr %17, align 8
  %19 = load i8, ptr %18, align 1
  %20 = sext i8 %19 to i32
  %21 = icmp eq i32 %20, 45
  %22 = zext i1 %21 to i32
  %23 = icmp ne i32 %22, 0
  br label %24

24:                                               ; preds = %16, %2
  %25 = phi i1 [ %15, %2 ], [ %23, %16 ]
  %26 = zext i1 %25 to i32
  %27 = icmp ne i32 %26, 0
  br i1 %27, label %28, label %37

28:                                               ; preds = %24
  %29 = load ptr, ptr %4, align 8
  %30 = load ptr, ptr %29, align 8
  %31 = getelementptr inbounds i8, ptr %30, i32 1
  %32 = load i8, ptr %31, align 1
  %33 = sext i8 %32 to i32
  %34 = icmp eq i32 %33, 115
  %35 = zext i1 %34 to i32
  %36 = icmp ne i32 %35, 0
  br label %37

37:                                               ; preds = %28, %24
  %38 = phi i1 [ %27, %24 ], [ %36, %28 ]
  %39 = zext i1 %38 to i32
  %40 = icmp ne i32 %39, 0
  br i1 %40, label %41, label %46

41:                                               ; preds = %37
  store i64 1, ptr @assembly, align 8
  %42 = load i32, ptr %3, align 4
  %43 = sub nsw i32 %42, 1
  store i32 %43, ptr %3, align 4
  %44 = load ptr, ptr %4, align 8
  %45 = getelementptr inbounds ptr, ptr %44, i64 1
  store ptr %45, ptr %4, align 8
  br label %46

46:                                               ; preds = %41, %37
  %47 = load i32, ptr %3, align 4
  %48 = icmp sgt i32 %47, 0
  %49 = zext i1 %48 to i32
  %50 = icmp ne i32 %49, 0
  br i1 %50, label %51, label %59

51:                                               ; preds = %46
  %52 = load ptr, ptr %4, align 8
  %53 = load ptr, ptr %52, align 8
  %54 = load i8, ptr %53, align 1
  %55 = sext i8 %54 to i32
  %56 = icmp eq i32 %55, 45
  %57 = zext i1 %56 to i32
  %58 = icmp ne i32 %57, 0
  br label %59

59:                                               ; preds = %51, %46
  %60 = phi i1 [ %50, %46 ], [ %58, %51 ]
  %61 = zext i1 %60 to i32
  %62 = icmp ne i32 %61, 0
  br i1 %62, label %63, label %72

63:                                               ; preds = %59
  %64 = load ptr, ptr %4, align 8
  %65 = load ptr, ptr %64, align 8
  %66 = getelementptr inbounds i8, ptr %65, i32 1
  %67 = load i8, ptr %66, align 1
  %68 = sext i8 %67 to i32
  %69 = icmp eq i32 %68, 100
  %70 = zext i1 %69 to i32
  %71 = icmp ne i32 %70, 0
  br label %72

72:                                               ; preds = %63, %59
  %73 = phi i1 [ %62, %59 ], [ %71, %63 ]
  %74 = zext i1 %73 to i32
  %75 = icmp ne i32 %74, 0
  br i1 %75, label %76, label %81

76:                                               ; preds = %72
  store i64 1, ptr @debug, align 8
  %77 = load i32, ptr %3, align 4
  %78 = sub nsw i32 %77, 1
  store i32 %78, ptr %3, align 4
  %79 = load ptr, ptr %4, align 8
  %80 = getelementptr inbounds ptr, ptr %79, i64 1
  store ptr %80, ptr %4, align 8
  br label %81

81:                                               ; preds = %76, %72
  %82 = load i32, ptr %3, align 4
  %83 = icmp slt i32 %82, 1
  %84 = zext i1 %83 to i32
  %85 = icmp ne i32 %84, 0
  br i1 %85, label %86, label %88

86:                                               ; preds = %81
  %87 = call i32 (ptr, ...) @printf(ptr @32)
  ret i32 -1

88:                                               ; preds = %81
  %89 = load ptr, ptr %4, align 8
  %90 = load ptr, ptr %89, align 8
  %91 = call i32 (ptr, i32, ...) @open(ptr %90, i32 0)
  %92 = sext i32 %91 to i64
  store i64 %92, ptr %6, align 8
  %93 = icmp slt i64 %92, 0
  %94 = zext i1 %93 to i32
  %95 = icmp ne i32 %94, 0
  br i1 %95, label %96, label %100

96:                                               ; preds = %88
  %97 = load ptr, ptr %4, align 8
  %98 = load ptr, ptr %97, align 8
  %99 = call i32 (ptr, ...) @printf(ptr @33, ptr %98)
  ret i32 -1

100:                                              ; preds = %88
  store i64 262144, ptr @poolsize, align 8
  store i64 1, ptr @line, align 8
  %101 = load i64, ptr @poolsize, align 8
  %102 = call ptr @malloc(i64 %101)
  store ptr %102, ptr @text, align 8
  %103 = icmp eq ptr %102, null
  %104 = zext i1 %103 to i32
  %105 = icmp ne i32 %104, 0
  br i1 %105, label %106, label %109

106:                                              ; preds = %100
  %107 = load i64, ptr @poolsize, align 8
  %108 = call i32 (ptr, ...) @printf(ptr @34, i64 %107)
  ret i32 -1

109:                                              ; preds = %100
  %110 = load i64, ptr @poolsize, align 8
  %111 = call ptr @malloc(i64 %110)
  store ptr %111, ptr @data, align 8
  %112 = icmp eq ptr %111, null
  %113 = zext i1 %112 to i32
  %114 = icmp ne i32 %113, 0
  br i1 %114, label %115, label %118

115:                                              ; preds = %109
  %116 = load i64, ptr @poolsize, align 8
  %117 = call i32 (ptr, ...) @printf(ptr @35, i64 %116)
  ret i32 -1

118:                                              ; preds = %109
  %119 = load i64, ptr @poolsize, align 8
  %120 = call ptr @malloc(i64 %119)
  store ptr %120, ptr @stack, align 8
  %121 = icmp eq ptr %120, null
  %122 = zext i1 %121 to i32
  %123 = icmp ne i32 %122, 0
  br i1 %123, label %124, label %127

124:                                              ; preds = %118
  %125 = load i64, ptr @poolsize, align 8
  %126 = call i32 (ptr, ...) @printf(ptr @36, i64 %125)
  ret i32 -1

127:                                              ; preds = %118
  %128 = load i64, ptr @poolsize, align 8
  %129 = call ptr @malloc(i64 %128)
  store ptr %129, ptr @symbols, align 8
  %130 = icmp eq ptr %129, null
  %131 = zext i1 %130 to i32
  %132 = icmp ne i32 %131, 0
  br i1 %132, label %133, label %136

133:                                              ; preds = %127
  %134 = load i64, ptr @poolsize, align 8
  %135 = call i32 (ptr, ...) @printf(ptr @37, i64 %134)
  ret i32 -1

136:                                              ; preds = %127
  %137 = load ptr, ptr @text, align 8
  %138 = load i64, ptr @poolsize, align 8
  %139 = call ptr @memset(ptr %137, i32 0, i64 %138)
  %140 = load ptr, ptr @data, align 8
  %141 = load i64, ptr @poolsize, align 8
  %142 = call ptr @memset(ptr %140, i32 0, i64 %141)
  %143 = load ptr, ptr @stack, align 8
  %144 = load i64, ptr @poolsize, align 8
  %145 = call ptr @memset(ptr %143, i32 0, i64 %144)
  %146 = load ptr, ptr @symbols, align 8
  %147 = load i64, ptr @poolsize, align 8
  %148 = call ptr @memset(ptr %146, i32 0, i64 %147)
  %149 = load ptr, ptr @text, align 8
  store ptr %149, ptr @old_text, align 8
  store ptr @38, ptr @src, align 8
  store i64 134, ptr %5, align 8
  br label %155

150:                                              ; preds = %155
  call void (...) @next()
  %151 = load ptr, ptr @current_id, align 8
  %152 = getelementptr inbounds i64, ptr %151, i32 0
  %153 = load i64, ptr %5, align 8
  %154 = add i64 %153, 1
  store i64 %154, ptr %5, align 8
  store i64 %153, ptr %152, align 8
  br label %155

155:                                              ; preds = %150, %136
  %156 = load i64, ptr %5, align 8
  %157 = icmp sle i64 %156, 141
  %158 = zext i1 %157 to i32
  %159 = icmp ne i32 %158, 0
  br i1 %159, label %150, label %160

160:                                              ; preds = %155
  store i64 30, ptr %5, align 8
  br label %170

161:                                              ; preds = %170
  call void (...) @next()
  %162 = load ptr, ptr @current_id, align 8
  %163 = getelementptr inbounds i64, ptr %162, i32 4
  store i64 130, ptr %163, align 8
  %164 = load ptr, ptr @current_id, align 8
  %165 = getelementptr inbounds i64, ptr %164, i32 3
  store i64 1, ptr %165, align 8
  %166 = load ptr, ptr @current_id, align 8
  %167 = getelementptr inbounds i64, ptr %166, i32 5
  %168 = load i64, ptr %5, align 8
  %169 = add i64 %168, 1
  store i64 %169, ptr %5, align 8
  store i64 %168, ptr %167, align 8
  br label %170

170:                                              ; preds = %161, %160
  %171 = load i64, ptr %5, align 8
  %172 = icmp sle i64 %171, 37
  %173 = zext i1 %172 to i32
  %174 = icmp ne i32 %173, 0
  br i1 %174, label %161, label %175

175:                                              ; preds = %170
  call void (...) @next()
  %176 = load ptr, ptr @current_id, align 8
  %177 = getelementptr inbounds i64, ptr %176, i32 0
  store i64 134, ptr %177, align 8
  call void (...) @next()
  %178 = load ptr, ptr @current_id, align 8
  store ptr %178, ptr @idmain, align 8
  %179 = load i64, ptr @poolsize, align 8
  %180 = call ptr @malloc(i64 %179)
  store ptr %180, ptr @old_src, align 8
  store ptr %180, ptr @src, align 8
  %181 = icmp eq ptr %180, null
  %182 = zext i1 %181 to i32
  %183 = icmp ne i32 %182, 0
  br i1 %183, label %184, label %187

184:                                              ; preds = %175
  %185 = load i64, ptr @poolsize, align 8
  %186 = call i32 (ptr, ...) @printf(ptr @39, i64 %185)
  ret i32 -1

187:                                              ; preds = %175
  %188 = load i64, ptr %6, align 8
  %189 = trunc i64 %188 to i32
  %190 = load ptr, ptr @src, align 8
  %191 = load i64, ptr @poolsize, align 8
  %192 = sub nsw i64 %191, 1
  %193 = call i64 @read(i32 %189, ptr %190, i64 %192)
  store i64 %193, ptr %5, align 8
  %194 = icmp sle i64 %193, 0
  %195 = zext i1 %194 to i32
  %196 = icmp ne i32 %195, 0
  br i1 %196, label %197, label %200

197:                                              ; preds = %187
  %198 = load i64, ptr %5, align 8
  %199 = call i32 (ptr, ...) @printf(ptr @40, i64 %198)
  ret i32 -1

200:                                              ; preds = %187
  %201 = load ptr, ptr @src, align 8
  %202 = load i64, ptr %5, align 8
  %203 = getelementptr inbounds i8, ptr %201, i64 %202
  store i8 0, ptr %203, align 1
  %204 = load i64, ptr %6, align 8
  %205 = trunc i64 %204 to i32
  %206 = call i32 @close(i32 %205)
  call void (...) @program()
  %207 = load ptr, ptr @idmain, align 8
  %208 = getelementptr inbounds i64, ptr %207, i32 5
  %209 = load i64, ptr %208, align 8
  %210 = inttoptr i64 %209 to ptr
  store ptr %210, ptr @pc, align 8
  %211 = icmp eq ptr %210, null
  %212 = zext i1 %211 to i32
  %213 = icmp ne i32 %212, 0
  br i1 %213, label %214, label %216

214:                                              ; preds = %200
  %215 = call i32 (ptr, ...) @printf(ptr @41)
  ret i32 -1

216:                                              ; preds = %200
  %217 = load i64, ptr @assembly, align 8
  %218 = icmp ne i64 %217, 0
  br i1 %218, label %219, label %220

219:                                              ; preds = %216
  ret i32 0

220:                                              ; preds = %216
  %221 = load ptr, ptr @stack, align 8
  %222 = ptrtoint ptr %221 to i64
  %223 = load i64, ptr @poolsize, align 8
  %224 = add nsw i64 %222, %223
  %225 = inttoptr i64 %224 to ptr
  store ptr %225, ptr @sp, align 8
  %226 = load ptr, ptr @sp, align 8
  %227 = getelementptr inbounds i64, ptr %226, i64 -1
  store ptr %227, ptr @sp, align 8
  store i64 37, ptr %227, align 8
  %228 = load ptr, ptr @sp, align 8
  %229 = getelementptr inbounds i64, ptr %228, i64 -1
  store ptr %229, ptr @sp, align 8
  store i64 13, ptr %229, align 8
  %230 = load ptr, ptr @sp, align 8
  store ptr %230, ptr %7, align 8
  %231 = load ptr, ptr @sp, align 8
  %232 = getelementptr inbounds i64, ptr %231, i64 -1
  store ptr %232, ptr @sp, align 8
  %233 = load i32, ptr %3, align 4
  %234 = sext i32 %233 to i64
  store i64 %234, ptr %232, align 8
  %235 = load ptr, ptr @sp, align 8
  %236 = getelementptr inbounds i64, ptr %235, i64 -1
  store ptr %236, ptr @sp, align 8
  %237 = load ptr, ptr %4, align 8
  %238 = ptrtoint ptr %237 to i64
  store i64 %238, ptr %236, align 8
  %239 = load ptr, ptr @sp, align 8
  %240 = getelementptr inbounds i64, ptr %239, i64 -1
  store ptr %240, ptr @sp, align 8
  %241 = load ptr, ptr %7, align 8
  %242 = ptrtoint ptr %241 to i64
  store i64 %242, ptr %240, align 8
  %243 = call i64 (...) @eval()
  %244 = trunc i64 %243 to i32
  ret i32 %244
}

attributes #0 = { nounwind optsize }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"XCC version alpha (https://github.com/ianfun/xcc.git)"}
!1 = !{i32 1, !"short_enum", i32 1}
!2 = !{i32 1, !"wchar_size", i32 1}
!3 = !{i32 1, !"short_wchar", i32 1}
