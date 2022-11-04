; ModuleID = 'xc.c'
source_filename = "xc.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@src = dso_local global ptr null, align 8
@token = dso_local global i64 0, align 8
@assembly = dso_local global i64 0, align 8
@.str = private unnamed_addr constant [9 x i8] c"%d: %.*s\00", align 1
@line = dso_local global i64 0, align 8
@old_src = dso_local global ptr null, align 8
@old_text = dso_local global ptr null, align 8
@text = dso_local global ptr null, align 8
@.str.1 = private unnamed_addr constant [6 x i8] c"%8.4s\00", align 1
@.str.2 = private unnamed_addr constant [190 x i8] c"LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT\00", align 1
@.str.3 = private unnamed_addr constant [5 x i8] c" %d\0A\00", align 1
@.str.4 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@symbols = dso_local global ptr null, align 8
@current_id = dso_local global ptr null, align 8
@token_val = dso_local global i64 0, align 8
@data = dso_local global ptr null, align 8
@.str.5 = private unnamed_addr constant [24 x i8] c"%d: expected token: %d\0A\00", align 1
@.str.6 = private unnamed_addr constant [40 x i8] c"%d: unexpected token EOF of expression\0A\00", align 1
@expr_type = dso_local global i64 0, align 8
@.str.7 = private unnamed_addr constant [23 x i8] c"%d: bad function call\0A\00", align 1
@index_of_bp = dso_local global i64 0, align 8
@.str.8 = private unnamed_addr constant [24 x i8] c"%d: undefined variable\0A\00", align 1
@.str.9 = private unnamed_addr constant [21 x i8] c"%d: bad dereference\0A\00", align 1
@.str.10 = private unnamed_addr constant [20 x i8] c"%d: bad address of\0A\00", align 1
@.str.11 = private unnamed_addr constant [33 x i8] c"%d: bad lvalue of pre-increment\0A\00", align 1
@.str.12 = private unnamed_addr constant [20 x i8] c"%d: bad expression\0A\00", align 1
@.str.13 = private unnamed_addr constant [30 x i8] c"%d: bad lvalue in assignment\0A\00", align 1
@.str.14 = private unnamed_addr constant [34 x i8] c"%d: missing colon in conditional\0A\00", align 1
@.str.15 = private unnamed_addr constant [28 x i8] c"%d: bad value in increment\0A\00", align 1
@.str.16 = private unnamed_addr constant [27 x i8] c"%d: pointer type expected\0A\00", align 1
@.str.17 = private unnamed_addr constant [32 x i8] c"%d: compiler error, token = %d\0A\00", align 1
@.str.18 = private unnamed_addr constant [28 x i8] c"%d: bad enum identifier %d\0A\00", align 1
@.str.19 = private unnamed_addr constant [26 x i8] c"%d: bad enum initializer\0A\00", align 1
@.str.20 = private unnamed_addr constant [31 x i8] c"%d: bad parameter declaration\0A\00", align 1
@.str.21 = private unnamed_addr constant [37 x i8] c"%d: duplicate parameter declaration\0A\00", align 1
@basetype = dso_local global i64 0, align 8
@.str.22 = private unnamed_addr constant [27 x i8] c"%d: bad local declaration\0A\00", align 1
@.str.23 = private unnamed_addr constant [33 x i8] c"%d: duplicate local declaration\0A\00", align 1
@.str.24 = private unnamed_addr constant [28 x i8] c"%d: bad global declaration\0A\00", align 1
@.str.25 = private unnamed_addr constant [34 x i8] c"%d: duplicate global declaration\0A\00", align 1
@cycle = dso_local global i64 0, align 8
@pc = dso_local global ptr null, align 8
@debug = dso_local global i64 0, align 8
@.str.26 = private unnamed_addr constant [9 x i8] c"%d> %.4s\00", align 1
@ax = dso_local global i64 0, align 8
@sp = dso_local global ptr null, align 8
@bp = dso_local global ptr null, align 8
@.str.27 = private unnamed_addr constant [9 x i8] c"exit(%d)\00", align 1
@.str.28 = private unnamed_addr constant [24 x i8] c"unknown instruction:%d\0A\00", align 1
@.str.29 = private unnamed_addr constant [30 x i8] c"usage: xc [-s] [-d] file ...\0A\00", align 1
@.str.30 = private unnamed_addr constant [20 x i8] c"could not open(%s)\0A\00", align 1
@poolsize = dso_local global i64 0, align 8
@.str.31 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for text area\0A\00", align 1
@.str.32 = private unnamed_addr constant [36 x i8] c"could not malloc(%d) for data area\0A\00", align 1
@stack = dso_local global ptr null, align 8
@.str.33 = private unnamed_addr constant [37 x i8] c"could not malloc(%d) for stack area\0A\00", align 1
@.str.34 = private unnamed_addr constant [39 x i8] c"could not malloc(%d) for symbol table\0A\00", align 1
@.str.35 = private unnamed_addr constant [101 x i8] c"char else enum if int return sizeof while open read close printf malloc memset memcmp exit void main\00", align 1
@idmain = dso_local global ptr null, align 8
@.str.36 = private unnamed_addr constant [38 x i8] c"could not malloc(%d) for source area\0A\00", align 1
@.str.37 = private unnamed_addr constant [20 x i8] c"read() returned %d\0A\00", align 1
@.str.38 = private unnamed_addr constant [20 x i8] c"main() not defined\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @next() #0 {
  %1 = alloca ptr, align 8
  %2 = alloca i64, align 8
  br label %3

3:                                                ; preds = %564, %0
  %4 = load ptr, ptr @src, align 8
  %5 = load i8, ptr %4, align 1
  %6 = sext i8 %5 to i64
  store i64 %6, ptr @token, align 8
  %7 = icmp ne i64 %6, 0
  br i1 %7, label %8, label %565

8:                                                ; preds = %3
  %9 = load ptr, ptr @src, align 8
  %10 = getelementptr inbounds i8, ptr %9, i32 1
  store ptr %10, ptr @src, align 8
  %11 = load i64, ptr @token, align 8
  %12 = icmp eq i64 %11, 10
  br i1 %12, label %13, label %52

13:                                               ; preds = %8
  %14 = load i64, ptr @assembly, align 8
  %15 = icmp ne i64 %14, 0
  br i1 %15, label %16, label %49

16:                                               ; preds = %13
  %17 = load i64, ptr @line, align 8
  %18 = load ptr, ptr @src, align 8
  %19 = load ptr, ptr @old_src, align 8
  %20 = ptrtoint ptr %18 to i64
  %21 = ptrtoint ptr %19 to i64
  %22 = sub i64 %20, %21
  %23 = load ptr, ptr @old_src, align 8
  %24 = call i32 (ptr, ...) @printf(ptr noundef @.str, i64 noundef %17, i64 noundef %22, ptr noundef %23)
  %25 = load ptr, ptr @src, align 8
  store ptr %25, ptr @old_src, align 8
  br label %26

26:                                               ; preds = %47, %16
  %27 = load ptr, ptr @old_text, align 8
  %28 = load ptr, ptr @text, align 8
  %29 = icmp ult ptr %27, %28
  br i1 %29, label %30, label %48

30:                                               ; preds = %26
  %31 = load ptr, ptr @old_text, align 8
  %32 = getelementptr inbounds i64, ptr %31, i32 1
  store ptr %32, ptr @old_text, align 8
  %33 = load i64, ptr %32, align 8
  %34 = mul nsw i64 %33, 5
  %35 = getelementptr inbounds [190 x i8], ptr @.str.2, i64 0, i64 %34
  %36 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, ptr noundef %35)
  %37 = load ptr, ptr @old_text, align 8
  %38 = load i64, ptr %37, align 8
  %39 = icmp sle i64 %38, 7
  br i1 %39, label %40, label %45

40:                                               ; preds = %30
  %41 = load ptr, ptr @old_text, align 8
  %42 = getelementptr inbounds i64, ptr %41, i32 1
  store ptr %42, ptr @old_text, align 8
  %43 = load i64, ptr %42, align 8
  %44 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i64 noundef %43)
  br label %47

45:                                               ; preds = %30
  %46 = call i32 (ptr, ...) @printf(ptr noundef @.str.4)
  br label %47

47:                                               ; preds = %45, %40
  br label %26, !llvm.loop !6

48:                                               ; preds = %26
  br label %49

49:                                               ; preds = %48, %13
  %50 = load i64, ptr @line, align 8
  %51 = add nsw i64 %50, 1
  store i64 %51, ptr @line, align 8
  br label %564

52:                                               ; preds = %8
  %53 = load i64, ptr @token, align 8
  %54 = icmp eq i64 %53, 35
  br i1 %54, label %55, label %72

55:                                               ; preds = %52
  br label %56

56:                                               ; preds = %68, %55
  %57 = load ptr, ptr @src, align 8
  %58 = load i8, ptr %57, align 1
  %59 = sext i8 %58 to i32
  %60 = icmp ne i32 %59, 0
  br i1 %60, label %61, label %66

61:                                               ; preds = %56
  %62 = load ptr, ptr @src, align 8
  %63 = load i8, ptr %62, align 1
  %64 = sext i8 %63 to i32
  %65 = icmp ne i32 %64, 10
  br label %66

66:                                               ; preds = %61, %56
  %67 = phi i1 [ false, %56 ], [ %65, %61 ]
  br i1 %67, label %68, label %71

68:                                               ; preds = %66
  %69 = load ptr, ptr @src, align 8
  %70 = getelementptr inbounds i8, ptr %69, i32 1
  store ptr %70, ptr @src, align 8
  br label %56, !llvm.loop !8

71:                                               ; preds = %66
  br label %563

72:                                               ; preds = %52
  %73 = load i64, ptr @token, align 8
  %74 = icmp sge i64 %73, 97
  br i1 %74, label %75, label %78

75:                                               ; preds = %72
  %76 = load i64, ptr @token, align 8
  %77 = icmp sle i64 %76, 122
  br i1 %77, label %87, label %78

78:                                               ; preds = %75, %72
  %79 = load i64, ptr @token, align 8
  %80 = icmp sge i64 %79, 65
  br i1 %80, label %81, label %84

81:                                               ; preds = %78
  %82 = load i64, ptr @token, align 8
  %83 = icmp sle i64 %82, 90
  br i1 %83, label %87, label %84

84:                                               ; preds = %81, %78
  %85 = load i64, ptr @token, align 8
  %86 = icmp eq i64 %85, 95
  br i1 %86, label %87, label %180

87:                                               ; preds = %84, %81, %75
  %88 = load ptr, ptr @src, align 8
  %89 = getelementptr inbounds i8, ptr %88, i64 -1
  store ptr %89, ptr %1, align 8
  %90 = load i64, ptr @token, align 8
  store i64 %90, ptr %2, align 8
  br label %91

91:                                               ; preds = %128, %87
  %92 = load ptr, ptr @src, align 8
  %93 = load i8, ptr %92, align 1
  %94 = sext i8 %93 to i32
  %95 = icmp sge i32 %94, 97
  br i1 %95, label %96, label %101

96:                                               ; preds = %91
  %97 = load ptr, ptr @src, align 8
  %98 = load i8, ptr %97, align 1
  %99 = sext i8 %98 to i32
  %100 = icmp sle i32 %99, 122
  br i1 %100, label %126, label %101

101:                                              ; preds = %96, %91
  %102 = load ptr, ptr @src, align 8
  %103 = load i8, ptr %102, align 1
  %104 = sext i8 %103 to i32
  %105 = icmp sge i32 %104, 65
  br i1 %105, label %106, label %111

106:                                              ; preds = %101
  %107 = load ptr, ptr @src, align 8
  %108 = load i8, ptr %107, align 1
  %109 = sext i8 %108 to i32
  %110 = icmp sle i32 %109, 90
  br i1 %110, label %126, label %111

111:                                              ; preds = %106, %101
  %112 = load ptr, ptr @src, align 8
  %113 = load i8, ptr %112, align 1
  %114 = sext i8 %113 to i32
  %115 = icmp sge i32 %114, 48
  br i1 %115, label %116, label %121

116:                                              ; preds = %111
  %117 = load ptr, ptr @src, align 8
  %118 = load i8, ptr %117, align 1
  %119 = sext i8 %118 to i32
  %120 = icmp sle i32 %119, 57
  br i1 %120, label %126, label %121

121:                                              ; preds = %116, %111
  %122 = load ptr, ptr @src, align 8
  %123 = load i8, ptr %122, align 1
  %124 = sext i8 %123 to i32
  %125 = icmp eq i32 %124, 95
  br label %126

126:                                              ; preds = %121, %116, %106, %96
  %127 = phi i1 [ true, %116 ], [ true, %106 ], [ true, %96 ], [ %125, %121 ]
  br i1 %127, label %128, label %137

128:                                              ; preds = %126
  %129 = load i64, ptr %2, align 8
  %130 = mul nsw i64 %129, 147
  %131 = load ptr, ptr @src, align 8
  %132 = load i8, ptr %131, align 1
  %133 = sext i8 %132 to i64
  %134 = add nsw i64 %130, %133
  store i64 %134, ptr %2, align 8
  %135 = load ptr, ptr @src, align 8
  %136 = getelementptr inbounds i8, ptr %135, i32 1
  store ptr %136, ptr @src, align 8
  br label %91, !llvm.loop !9

137:                                              ; preds = %126
  %138 = load ptr, ptr @symbols, align 8
  store ptr %138, ptr @current_id, align 8
  br label %139

139:                                              ; preds = %167, %137
  %140 = load ptr, ptr @current_id, align 8
  %141 = getelementptr inbounds i64, ptr %140, i64 0
  %142 = load i64, ptr %141, align 8
  %143 = icmp ne i64 %142, 0
  br i1 %143, label %144, label %170

144:                                              ; preds = %139
  %145 = load ptr, ptr @current_id, align 8
  %146 = getelementptr inbounds i64, ptr %145, i64 1
  %147 = load i64, ptr %146, align 8
  %148 = load i64, ptr %2, align 8
  %149 = icmp eq i64 %147, %148
  br i1 %149, label %150, label %167

150:                                              ; preds = %144
  %151 = load ptr, ptr @current_id, align 8
  %152 = getelementptr inbounds i64, ptr %151, i64 2
  %153 = load i64, ptr %152, align 8
  %154 = inttoptr i64 %153 to ptr
  %155 = load ptr, ptr %1, align 8
  %156 = load ptr, ptr @src, align 8
  %157 = load ptr, ptr %1, align 8
  %158 = ptrtoint ptr %156 to i64
  %159 = ptrtoint ptr %157 to i64
  %160 = sub i64 %158, %159
  %161 = call i32 @memcmp(ptr noundef %154, ptr noundef %155, i64 noundef %160)
  %162 = icmp ne i32 %161, 0
  br i1 %162, label %167, label %163

163:                                              ; preds = %150
  %164 = load ptr, ptr @current_id, align 8
  %165 = getelementptr inbounds i64, ptr %164, i64 0
  %166 = load i64, ptr %165, align 8
  store i64 %166, ptr @token, align 8
  br label %565

167:                                              ; preds = %150, %144
  %168 = load ptr, ptr @current_id, align 8
  %169 = getelementptr inbounds i64, ptr %168, i64 9
  store ptr %169, ptr @current_id, align 8
  br label %139, !llvm.loop !10

170:                                              ; preds = %139
  %171 = load ptr, ptr %1, align 8
  %172 = ptrtoint ptr %171 to i64
  %173 = load ptr, ptr @current_id, align 8
  %174 = getelementptr inbounds i64, ptr %173, i64 2
  store i64 %172, ptr %174, align 8
  %175 = load i64, ptr %2, align 8
  %176 = load ptr, ptr @current_id, align 8
  %177 = getelementptr inbounds i64, ptr %176, i64 1
  store i64 %175, ptr %177, align 8
  %178 = load ptr, ptr @current_id, align 8
  %179 = getelementptr inbounds i64, ptr %178, i64 0
  store i64 133, ptr %179, align 8
  store i64 133, ptr @token, align 8
  br label %565

180:                                              ; preds = %84
  %181 = load i64, ptr @token, align 8
  %182 = icmp sge i64 %181, 48
  br i1 %182, label %183, label %293

183:                                              ; preds = %180
  %184 = load i64, ptr @token, align 8
  %185 = icmp sle i64 %184, 57
  br i1 %185, label %186, label %293

186:                                              ; preds = %183
  %187 = load i64, ptr @token, align 8
  %188 = sub nsw i64 %187, 48
  store i64 %188, ptr @token_val, align 8
  %189 = load i64, ptr @token_val, align 8
  %190 = icmp sgt i64 %189, 0
  br i1 %190, label %191, label %214

191:                                              ; preds = %186
  br label %192

192:                                              ; preds = %204, %191
  %193 = load ptr, ptr @src, align 8
  %194 = load i8, ptr %193, align 1
  %195 = sext i8 %194 to i32
  %196 = icmp sge i32 %195, 48
  br i1 %196, label %197, label %202

197:                                              ; preds = %192
  %198 = load ptr, ptr @src, align 8
  %199 = load i8, ptr %198, align 1
  %200 = sext i8 %199 to i32
  %201 = icmp sle i32 %200, 57
  br label %202

202:                                              ; preds = %197, %192
  %203 = phi i1 [ false, %192 ], [ %201, %197 ]
  br i1 %203, label %204, label %213

204:                                              ; preds = %202
  %205 = load i64, ptr @token_val, align 8
  %206 = mul nsw i64 %205, 10
  %207 = load ptr, ptr @src, align 8
  %208 = getelementptr inbounds i8, ptr %207, i32 1
  store ptr %208, ptr @src, align 8
  %209 = load i8, ptr %207, align 1
  %210 = sext i8 %209 to i64
  %211 = add nsw i64 %206, %210
  %212 = sub nsw i64 %211, 48
  store i64 %212, ptr @token_val, align 8
  br label %192, !llvm.loop !11

213:                                              ; preds = %202
  br label %292

214:                                              ; preds = %186
  %215 = load ptr, ptr @src, align 8
  %216 = load i8, ptr %215, align 1
  %217 = sext i8 %216 to i32
  %218 = icmp eq i32 %217, 120
  br i1 %218, label %224, label %219

219:                                              ; preds = %214
  %220 = load ptr, ptr @src, align 8
  %221 = load i8, ptr %220, align 1
  %222 = sext i8 %221 to i32
  %223 = icmp eq i32 %222, 88
  br i1 %223, label %224, label %268

224:                                              ; preds = %219, %214
  %225 = load ptr, ptr @src, align 8
  %226 = getelementptr inbounds i8, ptr %225, i32 1
  store ptr %226, ptr @src, align 8
  %227 = load i8, ptr %226, align 1
  %228 = sext i8 %227 to i64
  store i64 %228, ptr @token, align 8
  br label %229

229:                                              ; preds = %251, %224
  %230 = load i64, ptr @token, align 8
  %231 = icmp sge i64 %230, 48
  br i1 %231, label %232, label %235

232:                                              ; preds = %229
  %233 = load i64, ptr @token, align 8
  %234 = icmp sle i64 %233, 57
  br i1 %234, label %249, label %235

235:                                              ; preds = %232, %229
  %236 = load i64, ptr @token, align 8
  %237 = icmp sge i64 %236, 97
  br i1 %237, label %238, label %241

238:                                              ; preds = %235
  %239 = load i64, ptr @token, align 8
  %240 = icmp sle i64 %239, 102
  br i1 %240, label %249, label %241

241:                                              ; preds = %238, %235
  %242 = load i64, ptr @token, align 8
  %243 = icmp sge i64 %242, 65
  br i1 %243, label %244, label %247

244:                                              ; preds = %241
  %245 = load i64, ptr @token, align 8
  %246 = icmp sle i64 %245, 70
  br label %247

247:                                              ; preds = %244, %241
  %248 = phi i1 [ false, %241 ], [ %246, %244 ]
  br label %249

249:                                              ; preds = %247, %238, %232
  %250 = phi i1 [ true, %238 ], [ true, %232 ], [ %248, %247 ]
  br i1 %250, label %251, label %267

251:                                              ; preds = %249
  %252 = load i64, ptr @token_val, align 8
  %253 = mul nsw i64 %252, 16
  %254 = load i64, ptr @token, align 8
  %255 = and i64 %254, 15
  %256 = add nsw i64 %253, %255
  %257 = load i64, ptr @token, align 8
  %258 = icmp sge i64 %257, 65
  %259 = zext i1 %258 to i64
  %260 = select i1 %258, i32 9, i32 0
  %261 = sext i32 %260 to i64
  %262 = add nsw i64 %256, %261
  store i64 %262, ptr @token_val, align 8
  %263 = load ptr, ptr @src, align 8
  %264 = getelementptr inbounds i8, ptr %263, i32 1
  store ptr %264, ptr @src, align 8
  %265 = load i8, ptr %264, align 1
  %266 = sext i8 %265 to i64
  store i64 %266, ptr @token, align 8
  br label %229, !llvm.loop !12

267:                                              ; preds = %249
  br label %291

268:                                              ; preds = %219
  br label %269

269:                                              ; preds = %281, %268
  %270 = load ptr, ptr @src, align 8
  %271 = load i8, ptr %270, align 1
  %272 = sext i8 %271 to i32
  %273 = icmp sge i32 %272, 48
  br i1 %273, label %274, label %279

274:                                              ; preds = %269
  %275 = load ptr, ptr @src, align 8
  %276 = load i8, ptr %275, align 1
  %277 = sext i8 %276 to i32
  %278 = icmp sle i32 %277, 55
  br label %279

279:                                              ; preds = %274, %269
  %280 = phi i1 [ false, %269 ], [ %278, %274 ]
  br i1 %280, label %281, label %290

281:                                              ; preds = %279
  %282 = load i64, ptr @token_val, align 8
  %283 = mul nsw i64 %282, 8
  %284 = load ptr, ptr @src, align 8
  %285 = getelementptr inbounds i8, ptr %284, i32 1
  store ptr %285, ptr @src, align 8
  %286 = load i8, ptr %284, align 1
  %287 = sext i8 %286 to i64
  %288 = add nsw i64 %283, %287
  %289 = sub nsw i64 %288, 48
  store i64 %289, ptr @token_val, align 8
  br label %269, !llvm.loop !13

290:                                              ; preds = %279
  br label %291

291:                                              ; preds = %290, %267
  br label %292

292:                                              ; preds = %291, %213
  store i64 128, ptr @token, align 8
  br label %565

293:                                              ; preds = %183, %180
  %294 = load i64, ptr @token, align 8
  %295 = icmp eq i64 %294, 47
  br i1 %295, label %296, label %320

296:                                              ; preds = %293
  %297 = load ptr, ptr @src, align 8
  %298 = load i8, ptr %297, align 1
  %299 = sext i8 %298 to i32
  %300 = icmp eq i32 %299, 47
  br i1 %300, label %301, label %318

301:                                              ; preds = %296
  br label %302

302:                                              ; preds = %314, %301
  %303 = load ptr, ptr @src, align 8
  %304 = load i8, ptr %303, align 1
  %305 = sext i8 %304 to i32
  %306 = icmp ne i32 %305, 0
  br i1 %306, label %307, label %312

307:                                              ; preds = %302
  %308 = load ptr, ptr @src, align 8
  %309 = load i8, ptr %308, align 1
  %310 = sext i8 %309 to i32
  %311 = icmp ne i32 %310, 10
  br label %312

312:                                              ; preds = %307, %302
  %313 = phi i1 [ false, %302 ], [ %311, %307 ]
  br i1 %313, label %314, label %317

314:                                              ; preds = %312
  %315 = load ptr, ptr @src, align 8
  %316 = getelementptr inbounds i8, ptr %315, i32 1
  store ptr %316, ptr @src, align 8
  br label %302, !llvm.loop !14

317:                                              ; preds = %312
  br label %319

318:                                              ; preds = %296
  store i64 160, ptr @token, align 8
  br label %565

319:                                              ; preds = %317
  br label %560

320:                                              ; preds = %293
  %321 = load i64, ptr @token, align 8
  %322 = icmp eq i64 %321, 34
  br i1 %322, label %326, label %323

323:                                              ; preds = %320
  %324 = load i64, ptr @token, align 8
  %325 = icmp eq i64 %324, 39
  br i1 %325, label %326, label %376

326:                                              ; preds = %323, %320
  %327 = load ptr, ptr @data, align 8
  store ptr %327, ptr %1, align 8
  br label %328

328:                                              ; preds = %365, %326
  %329 = load ptr, ptr @src, align 8
  %330 = load i8, ptr %329, align 1
  %331 = sext i8 %330 to i32
  %332 = icmp ne i32 %331, 0
  br i1 %332, label %333, label %339

333:                                              ; preds = %328
  %334 = load ptr, ptr @src, align 8
  %335 = load i8, ptr %334, align 1
  %336 = sext i8 %335 to i64
  %337 = load i64, ptr @token, align 8
  %338 = icmp ne i64 %336, %337
  br label %339

339:                                              ; preds = %333, %328
  %340 = phi i1 [ false, %328 ], [ %338, %333 ]
  br i1 %340, label %341, label %366

341:                                              ; preds = %339
  %342 = load ptr, ptr @src, align 8
  %343 = getelementptr inbounds i8, ptr %342, i32 1
  store ptr %343, ptr @src, align 8
  %344 = load i8, ptr %342, align 1
  %345 = sext i8 %344 to i64
  store i64 %345, ptr @token_val, align 8
  %346 = load i64, ptr @token_val, align 8
  %347 = icmp eq i64 %346, 92
  br i1 %347, label %348, label %357

348:                                              ; preds = %341
  %349 = load ptr, ptr @src, align 8
  %350 = getelementptr inbounds i8, ptr %349, i32 1
  store ptr %350, ptr @src, align 8
  %351 = load i8, ptr %349, align 1
  %352 = sext i8 %351 to i64
  store i64 %352, ptr @token_val, align 8
  %353 = load i64, ptr @token_val, align 8
  %354 = icmp eq i64 %353, 110
  br i1 %354, label %355, label %356

355:                                              ; preds = %348
  store i64 10, ptr @token_val, align 8
  br label %356

356:                                              ; preds = %355, %348
  br label %357

357:                                              ; preds = %356, %341
  %358 = load i64, ptr @token, align 8
  %359 = icmp eq i64 %358, 34
  br i1 %359, label %360, label %365

360:                                              ; preds = %357
  %361 = load i64, ptr @token_val, align 8
  %362 = trunc i64 %361 to i8
  %363 = load ptr, ptr @data, align 8
  %364 = getelementptr inbounds i8, ptr %363, i32 1
  store ptr %364, ptr @data, align 8
  store i8 %362, ptr %363, align 1
  br label %365

365:                                              ; preds = %360, %357
  br label %328, !llvm.loop !15

366:                                              ; preds = %339
  %367 = load ptr, ptr @src, align 8
  %368 = getelementptr inbounds i8, ptr %367, i32 1
  store ptr %368, ptr @src, align 8
  %369 = load i64, ptr @token, align 8
  %370 = icmp eq i64 %369, 34
  br i1 %370, label %371, label %374

371:                                              ; preds = %366
  %372 = load ptr, ptr %1, align 8
  %373 = ptrtoint ptr %372 to i64
  store i64 %373, ptr @token_val, align 8
  br label %375

374:                                              ; preds = %366
  store i64 128, ptr @token, align 8
  br label %375

375:                                              ; preds = %374, %371
  br label %565

376:                                              ; preds = %323
  %377 = load i64, ptr @token, align 8
  %378 = icmp eq i64 %377, 61
  br i1 %378, label %379, label %389

379:                                              ; preds = %376
  %380 = load ptr, ptr @src, align 8
  %381 = load i8, ptr %380, align 1
  %382 = sext i8 %381 to i32
  %383 = icmp eq i32 %382, 61
  br i1 %383, label %384, label %387

384:                                              ; preds = %379
  %385 = load ptr, ptr @src, align 8
  %386 = getelementptr inbounds i8, ptr %385, i32 1
  store ptr %386, ptr @src, align 8
  store i64 149, ptr @token, align 8
  br label %388

387:                                              ; preds = %379
  store i64 142, ptr @token, align 8
  br label %388

388:                                              ; preds = %387, %384
  br label %565

389:                                              ; preds = %376
  %390 = load i64, ptr @token, align 8
  %391 = icmp eq i64 %390, 43
  br i1 %391, label %392, label %402

392:                                              ; preds = %389
  %393 = load ptr, ptr @src, align 8
  %394 = load i8, ptr %393, align 1
  %395 = sext i8 %394 to i32
  %396 = icmp eq i32 %395, 43
  br i1 %396, label %397, label %400

397:                                              ; preds = %392
  %398 = load ptr, ptr @src, align 8
  %399 = getelementptr inbounds i8, ptr %398, i32 1
  store ptr %399, ptr @src, align 8
  store i64 162, ptr @token, align 8
  br label %401

400:                                              ; preds = %392
  store i64 157, ptr @token, align 8
  br label %401

401:                                              ; preds = %400, %397
  br label %565

402:                                              ; preds = %389
  %403 = load i64, ptr @token, align 8
  %404 = icmp eq i64 %403, 45
  br i1 %404, label %405, label %415

405:                                              ; preds = %402
  %406 = load ptr, ptr @src, align 8
  %407 = load i8, ptr %406, align 1
  %408 = sext i8 %407 to i32
  %409 = icmp eq i32 %408, 45
  br i1 %409, label %410, label %413

410:                                              ; preds = %405
  %411 = load ptr, ptr @src, align 8
  %412 = getelementptr inbounds i8, ptr %411, i32 1
  store ptr %412, ptr @src, align 8
  store i64 163, ptr @token, align 8
  br label %414

413:                                              ; preds = %405
  store i64 158, ptr @token, align 8
  br label %414

414:                                              ; preds = %413, %410
  br label %565

415:                                              ; preds = %402
  %416 = load i64, ptr @token, align 8
  %417 = icmp eq i64 %416, 33
  br i1 %417, label %418, label %427

418:                                              ; preds = %415
  %419 = load ptr, ptr @src, align 8
  %420 = load i8, ptr %419, align 1
  %421 = sext i8 %420 to i32
  %422 = icmp eq i32 %421, 61
  br i1 %422, label %423, label %426

423:                                              ; preds = %418
  %424 = load ptr, ptr @src, align 8
  %425 = getelementptr inbounds i8, ptr %424, i32 1
  store ptr %425, ptr @src, align 8
  store i64 150, ptr @token, align 8
  br label %426

426:                                              ; preds = %423, %418
  br label %565

427:                                              ; preds = %415
  %428 = load i64, ptr @token, align 8
  %429 = icmp eq i64 %428, 60
  br i1 %429, label %430, label %449

430:                                              ; preds = %427
  %431 = load ptr, ptr @src, align 8
  %432 = load i8, ptr %431, align 1
  %433 = sext i8 %432 to i32
  %434 = icmp eq i32 %433, 61
  br i1 %434, label %435, label %438

435:                                              ; preds = %430
  %436 = load ptr, ptr @src, align 8
  %437 = getelementptr inbounds i8, ptr %436, i32 1
  store ptr %437, ptr @src, align 8
  store i64 153, ptr @token, align 8
  br label %448

438:                                              ; preds = %430
  %439 = load ptr, ptr @src, align 8
  %440 = load i8, ptr %439, align 1
  %441 = sext i8 %440 to i32
  %442 = icmp eq i32 %441, 60
  br i1 %442, label %443, label %446

443:                                              ; preds = %438
  %444 = load ptr, ptr @src, align 8
  %445 = getelementptr inbounds i8, ptr %444, i32 1
  store ptr %445, ptr @src, align 8
  store i64 155, ptr @token, align 8
  br label %447

446:                                              ; preds = %438
  store i64 151, ptr @token, align 8
  br label %447

447:                                              ; preds = %446, %443
  br label %448

448:                                              ; preds = %447, %435
  br label %565

449:                                              ; preds = %427
  %450 = load i64, ptr @token, align 8
  %451 = icmp eq i64 %450, 62
  br i1 %451, label %452, label %471

452:                                              ; preds = %449
  %453 = load ptr, ptr @src, align 8
  %454 = load i8, ptr %453, align 1
  %455 = sext i8 %454 to i32
  %456 = icmp eq i32 %455, 61
  br i1 %456, label %457, label %460

457:                                              ; preds = %452
  %458 = load ptr, ptr @src, align 8
  %459 = getelementptr inbounds i8, ptr %458, i32 1
  store ptr %459, ptr @src, align 8
  store i64 154, ptr @token, align 8
  br label %470

460:                                              ; preds = %452
  %461 = load ptr, ptr @src, align 8
  %462 = load i8, ptr %461, align 1
  %463 = sext i8 %462 to i32
  %464 = icmp eq i32 %463, 62
  br i1 %464, label %465, label %468

465:                                              ; preds = %460
  %466 = load ptr, ptr @src, align 8
  %467 = getelementptr inbounds i8, ptr %466, i32 1
  store ptr %467, ptr @src, align 8
  store i64 156, ptr @token, align 8
  br label %469

468:                                              ; preds = %460
  store i64 152, ptr @token, align 8
  br label %469

469:                                              ; preds = %468, %465
  br label %470

470:                                              ; preds = %469, %457
  br label %565

471:                                              ; preds = %449
  %472 = load i64, ptr @token, align 8
  %473 = icmp eq i64 %472, 124
  br i1 %473, label %474, label %484

474:                                              ; preds = %471
  %475 = load ptr, ptr @src, align 8
  %476 = load i8, ptr %475, align 1
  %477 = sext i8 %476 to i32
  %478 = icmp eq i32 %477, 124
  br i1 %478, label %479, label %482

479:                                              ; preds = %474
  %480 = load ptr, ptr @src, align 8
  %481 = getelementptr inbounds i8, ptr %480, i32 1
  store ptr %481, ptr @src, align 8
  store i64 144, ptr @token, align 8
  br label %483

482:                                              ; preds = %474
  store i64 146, ptr @token, align 8
  br label %483

483:                                              ; preds = %482, %479
  br label %565

484:                                              ; preds = %471
  %485 = load i64, ptr @token, align 8
  %486 = icmp eq i64 %485, 38
  br i1 %486, label %487, label %497

487:                                              ; preds = %484
  %488 = load ptr, ptr @src, align 8
  %489 = load i8, ptr %488, align 1
  %490 = sext i8 %489 to i32
  %491 = icmp eq i32 %490, 38
  br i1 %491, label %492, label %495

492:                                              ; preds = %487
  %493 = load ptr, ptr @src, align 8
  %494 = getelementptr inbounds i8, ptr %493, i32 1
  store ptr %494, ptr @src, align 8
  store i64 145, ptr @token, align 8
  br label %496

495:                                              ; preds = %487
  store i64 148, ptr @token, align 8
  br label %496

496:                                              ; preds = %495, %492
  br label %565

497:                                              ; preds = %484
  %498 = load i64, ptr @token, align 8
  %499 = icmp eq i64 %498, 94
  br i1 %499, label %500, label %501

500:                                              ; preds = %497
  store i64 147, ptr @token, align 8
  br label %565

501:                                              ; preds = %497
  %502 = load i64, ptr @token, align 8
  %503 = icmp eq i64 %502, 37
  br i1 %503, label %504, label %505

504:                                              ; preds = %501
  store i64 161, ptr @token, align 8
  br label %565

505:                                              ; preds = %501
  %506 = load i64, ptr @token, align 8
  %507 = icmp eq i64 %506, 42
  br i1 %507, label %508, label %509

508:                                              ; preds = %505
  store i64 159, ptr @token, align 8
  br label %565

509:                                              ; preds = %505
  %510 = load i64, ptr @token, align 8
  %511 = icmp eq i64 %510, 91
  br i1 %511, label %512, label %513

512:                                              ; preds = %509
  store i64 164, ptr @token, align 8
  br label %565

513:                                              ; preds = %509
  %514 = load i64, ptr @token, align 8
  %515 = icmp eq i64 %514, 63
  br i1 %515, label %516, label %517

516:                                              ; preds = %513
  store i64 143, ptr @token, align 8
  br label %565

517:                                              ; preds = %513
  %518 = load i64, ptr @token, align 8
  %519 = icmp eq i64 %518, 126
  br i1 %519, label %544, label %520

520:                                              ; preds = %517
  %521 = load i64, ptr @token, align 8
  %522 = icmp eq i64 %521, 59
  br i1 %522, label %544, label %523

523:                                              ; preds = %520
  %524 = load i64, ptr @token, align 8
  %525 = icmp eq i64 %524, 123
  br i1 %525, label %544, label %526

526:                                              ; preds = %523
  %527 = load i64, ptr @token, align 8
  %528 = icmp eq i64 %527, 125
  br i1 %528, label %544, label %529

529:                                              ; preds = %526
  %530 = load i64, ptr @token, align 8
  %531 = icmp eq i64 %530, 40
  br i1 %531, label %544, label %532

532:                                              ; preds = %529
  %533 = load i64, ptr @token, align 8
  %534 = icmp eq i64 %533, 41
  br i1 %534, label %544, label %535

535:                                              ; preds = %532
  %536 = load i64, ptr @token, align 8
  %537 = icmp eq i64 %536, 93
  br i1 %537, label %544, label %538

538:                                              ; preds = %535
  %539 = load i64, ptr @token, align 8
  %540 = icmp eq i64 %539, 44
  br i1 %540, label %544, label %541

541:                                              ; preds = %538
  %542 = load i64, ptr @token, align 8
  %543 = icmp eq i64 %542, 58
  br i1 %543, label %544, label %545

544:                                              ; preds = %541, %538, %535, %532, %529, %526, %523, %520, %517
  br label %565

545:                                              ; preds = %541
  br label %546

546:                                              ; preds = %545
  br label %547

547:                                              ; preds = %546
  br label %548

548:                                              ; preds = %547
  br label %549

549:                                              ; preds = %548
  br label %550

550:                                              ; preds = %549
  br label %551

551:                                              ; preds = %550
  br label %552

552:                                              ; preds = %551
  br label %553

553:                                              ; preds = %552
  br label %554

554:                                              ; preds = %553
  br label %555

555:                                              ; preds = %554
  br label %556

556:                                              ; preds = %555
  br label %557

557:                                              ; preds = %556
  br label %558

558:                                              ; preds = %557
  br label %559

559:                                              ; preds = %558
  br label %560

560:                                              ; preds = %559, %319
  br label %561

561:                                              ; preds = %560
  br label %562

562:                                              ; preds = %561
  br label %563

563:                                              ; preds = %562, %71
  br label %564

564:                                              ; preds = %563, %49
  br label %3, !llvm.loop !16

565:                                              ; preds = %163, %170, %292, %318, %375, %388, %401, %414, %426, %448, %470, %483, %496, %500, %504, %508, %512, %516, %544, %3
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

declare i32 @memcmp(ptr noundef, ptr noundef, i64 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @match(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = load i64, ptr @token, align 8
  %4 = load i64, ptr %2, align 8
  %5 = icmp eq i64 %3, %4
  br i1 %5, label %6, label %7

6:                                                ; preds = %1
  call void @next()
  br label %11

7:                                                ; preds = %1
  %8 = load i64, ptr @line, align 8
  %9 = load i64, ptr %2, align 8
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i64 noundef %8, i64 noundef %9)
  call void @exit(i32 noundef -1) #3
  unreachable

11:                                               ; preds = %6
  ret void
}

; Function Attrs: noreturn
declare void @exit(i32 noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @expression(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  store i64 %0, ptr %2, align 8
  %6 = load i64, ptr @token, align 8
  %7 = icmp ne i64 %6, 0
  br i1 %7, label %11, label %8

8:                                                ; preds = %1
  %9 = load i64, ptr @line, align 8
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i64 noundef %9)
  call void @exit(i32 noundef -1) #3
  unreachable

11:                                               ; preds = %1
  %12 = load i64, ptr @token, align 8
  %13 = icmp eq i64 %12, 128
  br i1 %13, label %14, label %20

14:                                               ; preds = %11
  call void @match(i64 noundef 128)
  %15 = load ptr, ptr @text, align 8
  %16 = getelementptr inbounds i64, ptr %15, i32 1
  store ptr %16, ptr @text, align 8
  store i64 1, ptr %16, align 8
  %17 = load i64, ptr @token_val, align 8
  %18 = load ptr, ptr @text, align 8
  %19 = getelementptr inbounds i64, ptr %18, i32 1
  store ptr %19, ptr @text, align 8
  store i64 %17, ptr %19, align 8
  store i64 1, ptr @expr_type, align 8
  br label %370

20:                                               ; preds = %11
  %21 = load i64, ptr @token, align 8
  %22 = icmp eq i64 %21, 34
  br i1 %22, label %23, label %39

23:                                               ; preds = %20
  %24 = load ptr, ptr @text, align 8
  %25 = getelementptr inbounds i64, ptr %24, i32 1
  store ptr %25, ptr @text, align 8
  store i64 1, ptr %25, align 8
  %26 = load i64, ptr @token_val, align 8
  %27 = load ptr, ptr @text, align 8
  %28 = getelementptr inbounds i64, ptr %27, i32 1
  store ptr %28, ptr @text, align 8
  store i64 %26, ptr %28, align 8
  call void @match(i64 noundef 34)
  br label %29

29:                                               ; preds = %32, %23
  %30 = load i64, ptr @token, align 8
  %31 = icmp eq i64 %30, 34
  br i1 %31, label %32, label %33

32:                                               ; preds = %29
  call void @match(i64 noundef 34)
  br label %29, !llvm.loop !17

33:                                               ; preds = %29
  %34 = load ptr, ptr @data, align 8
  %35 = ptrtoint ptr %34 to i64
  %36 = add i64 %35, 8
  %37 = and i64 %36, -8
  %38 = inttoptr i64 %37 to ptr
  store ptr %38, ptr @data, align 8
  store i64 2, ptr @expr_type, align 8
  br label %369

39:                                               ; preds = %20
  %40 = load i64, ptr @token, align 8
  %41 = icmp eq i64 %40, 140
  br i1 %41, label %42, label %67

42:                                               ; preds = %39
  call void @match(i64 noundef 140)
  call void @match(i64 noundef 40)
  store i64 1, ptr @expr_type, align 8
  %43 = load i64, ptr @token, align 8
  %44 = icmp eq i64 %43, 138
  br i1 %44, label %45, label %46

45:                                               ; preds = %42
  call void @match(i64 noundef 138)
  br label %51

46:                                               ; preds = %42
  %47 = load i64, ptr @token, align 8
  %48 = icmp eq i64 %47, 134
  br i1 %48, label %49, label %50

49:                                               ; preds = %46
  call void @match(i64 noundef 134)
  store i64 0, ptr @expr_type, align 8
  br label %50

50:                                               ; preds = %49, %46
  br label %51

51:                                               ; preds = %50, %45
  br label %52

52:                                               ; preds = %55, %51
  %53 = load i64, ptr @token, align 8
  %54 = icmp eq i64 %53, 159
  br i1 %54, label %55, label %58

55:                                               ; preds = %52
  call void @match(i64 noundef 159)
  %56 = load i64, ptr @expr_type, align 8
  %57 = add nsw i64 %56, 2
  store i64 %57, ptr @expr_type, align 8
  br label %52, !llvm.loop !18

58:                                               ; preds = %52
  call void @match(i64 noundef 41)
  %59 = load ptr, ptr @text, align 8
  %60 = getelementptr inbounds i64, ptr %59, i32 1
  store ptr %60, ptr @text, align 8
  store i64 1, ptr %60, align 8
  %61 = load i64, ptr @expr_type, align 8
  %62 = icmp eq i64 %61, 0
  %63 = zext i1 %62 to i64
  %64 = select i1 %62, i64 1, i64 8
  %65 = load ptr, ptr @text, align 8
  %66 = getelementptr inbounds i64, ptr %65, i32 1
  store ptr %66, ptr @text, align 8
  store i64 %64, ptr %66, align 8
  store i64 1, ptr @expr_type, align 8
  br label %368

67:                                               ; preds = %39
  %68 = load i64, ptr @token, align 8
  %69 = icmp eq i64 %68, 133
  br i1 %69, label %70, label %186

70:                                               ; preds = %67
  call void @match(i64 noundef 133)
  %71 = load ptr, ptr @current_id, align 8
  store ptr %71, ptr %3, align 8
  %72 = load i64, ptr @token, align 8
  %73 = icmp eq i64 %72, 40
  br i1 %73, label %74, label %128

74:                                               ; preds = %70
  call void @match(i64 noundef 40)
  store i64 0, ptr %4, align 8
  br label %75

75:                                               ; preds = %86, %74
  %76 = load i64, ptr @token, align 8
  %77 = icmp ne i64 %76, 41
  br i1 %77, label %78, label %87

78:                                               ; preds = %75
  call void @expression(i64 noundef 142)
  %79 = load ptr, ptr @text, align 8
  %80 = getelementptr inbounds i64, ptr %79, i32 1
  store ptr %80, ptr @text, align 8
  store i64 13, ptr %80, align 8
  %81 = load i64, ptr %4, align 8
  %82 = add nsw i64 %81, 1
  store i64 %82, ptr %4, align 8
  %83 = load i64, ptr @token, align 8
  %84 = icmp eq i64 %83, 44
  br i1 %84, label %85, label %86

85:                                               ; preds = %78
  call void @match(i64 noundef 44)
  br label %86

86:                                               ; preds = %85, %78
  br label %75, !llvm.loop !19

87:                                               ; preds = %75
  call void @match(i64 noundef 41)
  %88 = load ptr, ptr %3, align 8
  %89 = getelementptr inbounds i64, ptr %88, i64 4
  %90 = load i64, ptr %89, align 8
  %91 = icmp eq i64 %90, 130
  br i1 %91, label %92, label %98

92:                                               ; preds = %87
  %93 = load ptr, ptr %3, align 8
  %94 = getelementptr inbounds i64, ptr %93, i64 5
  %95 = load i64, ptr %94, align 8
  %96 = load ptr, ptr @text, align 8
  %97 = getelementptr inbounds i64, ptr %96, i32 1
  store ptr %97, ptr @text, align 8
  store i64 %95, ptr %97, align 8
  br label %115

98:                                               ; preds = %87
  %99 = load ptr, ptr %3, align 8
  %100 = getelementptr inbounds i64, ptr %99, i64 4
  %101 = load i64, ptr %100, align 8
  %102 = icmp eq i64 %101, 129
  br i1 %102, label %103, label %111

103:                                              ; preds = %98
  %104 = load ptr, ptr @text, align 8
  %105 = getelementptr inbounds i64, ptr %104, i32 1
  store ptr %105, ptr @text, align 8
  store i64 3, ptr %105, align 8
  %106 = load ptr, ptr %3, align 8
  %107 = getelementptr inbounds i64, ptr %106, i64 5
  %108 = load i64, ptr %107, align 8
  %109 = load ptr, ptr @text, align 8
  %110 = getelementptr inbounds i64, ptr %109, i32 1
  store ptr %110, ptr @text, align 8
  store i64 %108, ptr %110, align 8
  br label %114

111:                                              ; preds = %98
  %112 = load i64, ptr @line, align 8
  %113 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, i64 noundef %112)
  call void @exit(i32 noundef -1) #3
  unreachable

114:                                              ; preds = %103
  br label %115

115:                                              ; preds = %114, %92
  %116 = load i64, ptr %4, align 8
  %117 = icmp sgt i64 %116, 0
  br i1 %117, label %118, label %124

118:                                              ; preds = %115
  %119 = load ptr, ptr @text, align 8
  %120 = getelementptr inbounds i64, ptr %119, i32 1
  store ptr %120, ptr @text, align 8
  store i64 7, ptr %120, align 8
  %121 = load i64, ptr %4, align 8
  %122 = load ptr, ptr @text, align 8
  %123 = getelementptr inbounds i64, ptr %122, i32 1
  store ptr %123, ptr @text, align 8
  store i64 %121, ptr %123, align 8
  br label %124

124:                                              ; preds = %118, %115
  %125 = load ptr, ptr %3, align 8
  %126 = getelementptr inbounds i64, ptr %125, i64 3
  %127 = load i64, ptr %126, align 8
  store i64 %127, ptr @expr_type, align 8
  br label %185

128:                                              ; preds = %70
  %129 = load ptr, ptr %3, align 8
  %130 = getelementptr inbounds i64, ptr %129, i64 4
  %131 = load i64, ptr %130, align 8
  %132 = icmp eq i64 %131, 128
  br i1 %132, label %133, label %141

133:                                              ; preds = %128
  %134 = load ptr, ptr @text, align 8
  %135 = getelementptr inbounds i64, ptr %134, i32 1
  store ptr %135, ptr @text, align 8
  store i64 1, ptr %135, align 8
  %136 = load ptr, ptr %3, align 8
  %137 = getelementptr inbounds i64, ptr %136, i64 5
  %138 = load i64, ptr %137, align 8
  %139 = load ptr, ptr @text, align 8
  %140 = getelementptr inbounds i64, ptr %139, i32 1
  store ptr %140, ptr @text, align 8
  store i64 %138, ptr %140, align 8
  store i64 1, ptr @expr_type, align 8
  br label %184

141:                                              ; preds = %128
  %142 = load ptr, ptr %3, align 8
  %143 = getelementptr inbounds i64, ptr %142, i64 4
  %144 = load i64, ptr %143, align 8
  %145 = icmp eq i64 %144, 132
  br i1 %145, label %146, label %156

146:                                              ; preds = %141
  %147 = load ptr, ptr @text, align 8
  %148 = getelementptr inbounds i64, ptr %147, i32 1
  store ptr %148, ptr @text, align 8
  store i64 0, ptr %148, align 8
  %149 = load i64, ptr @index_of_bp, align 8
  %150 = load ptr, ptr %3, align 8
  %151 = getelementptr inbounds i64, ptr %150, i64 5
  %152 = load i64, ptr %151, align 8
  %153 = sub nsw i64 %149, %152
  %154 = load ptr, ptr @text, align 8
  %155 = getelementptr inbounds i64, ptr %154, i32 1
  store ptr %155, ptr @text, align 8
  store i64 %153, ptr %155, align 8
  br label %173

156:                                              ; preds = %141
  %157 = load ptr, ptr %3, align 8
  %158 = getelementptr inbounds i64, ptr %157, i64 4
  %159 = load i64, ptr %158, align 8
  %160 = icmp eq i64 %159, 131
  br i1 %160, label %161, label %169

161:                                              ; preds = %156
  %162 = load ptr, ptr @text, align 8
  %163 = getelementptr inbounds i64, ptr %162, i32 1
  store ptr %163, ptr @text, align 8
  store i64 1, ptr %163, align 8
  %164 = load ptr, ptr %3, align 8
  %165 = getelementptr inbounds i64, ptr %164, i64 5
  %166 = load i64, ptr %165, align 8
  %167 = load ptr, ptr @text, align 8
  %168 = getelementptr inbounds i64, ptr %167, i32 1
  store ptr %168, ptr @text, align 8
  store i64 %166, ptr %168, align 8
  br label %172

169:                                              ; preds = %156
  %170 = load i64, ptr @line, align 8
  %171 = call i32 (ptr, ...) @printf(ptr noundef @.str.8, i64 noundef %170)
  call void @exit(i32 noundef -1) #3
  unreachable

172:                                              ; preds = %161
  br label %173

173:                                              ; preds = %172, %146
  %174 = load ptr, ptr %3, align 8
  %175 = getelementptr inbounds i64, ptr %174, i64 3
  %176 = load i64, ptr %175, align 8
  store i64 %176, ptr @expr_type, align 8
  %177 = load i64, ptr @expr_type, align 8
  %178 = icmp eq i64 %177, 0
  %179 = zext i1 %178 to i64
  %180 = select i1 %178, i32 10, i32 9
  %181 = sext i32 %180 to i64
  %182 = load ptr, ptr @text, align 8
  %183 = getelementptr inbounds i64, ptr %182, i32 1
  store ptr %183, ptr @text, align 8
  store i64 %181, ptr %183, align 8
  br label %184

184:                                              ; preds = %173, %133
  br label %185

185:                                              ; preds = %184, %124
  br label %367

186:                                              ; preds = %67
  %187 = load i64, ptr @token, align 8
  %188 = icmp eq i64 %187, 40
  br i1 %188, label %189, label %212

189:                                              ; preds = %186
  call void @match(i64 noundef 40)
  %190 = load i64, ptr @token, align 8
  %191 = icmp eq i64 %190, 138
  br i1 %191, label %195, label %192

192:                                              ; preds = %189
  %193 = load i64, ptr @token, align 8
  %194 = icmp eq i64 %193, 134
  br i1 %194, label %195, label %210

195:                                              ; preds = %192, %189
  %196 = load i64, ptr @token, align 8
  %197 = icmp eq i64 %196, 134
  %198 = zext i1 %197 to i64
  %199 = select i1 %197, i32 0, i32 1
  %200 = sext i32 %199 to i64
  store i64 %200, ptr %4, align 8
  %201 = load i64, ptr @token, align 8
  call void @match(i64 noundef %201)
  br label %202

202:                                              ; preds = %205, %195
  %203 = load i64, ptr @token, align 8
  %204 = icmp eq i64 %203, 159
  br i1 %204, label %205, label %208

205:                                              ; preds = %202
  call void @match(i64 noundef 159)
  %206 = load i64, ptr %4, align 8
  %207 = add nsw i64 %206, 2
  store i64 %207, ptr %4, align 8
  br label %202, !llvm.loop !20

208:                                              ; preds = %202
  call void @match(i64 noundef 41)
  call void @expression(i64 noundef 162)
  %209 = load i64, ptr %4, align 8
  store i64 %209, ptr @expr_type, align 8
  br label %211

210:                                              ; preds = %192
  call void @expression(i64 noundef 142)
  call void @match(i64 noundef 41)
  br label %211

211:                                              ; preds = %210, %208
  br label %366

212:                                              ; preds = %186
  %213 = load i64, ptr @token, align 8
  %214 = icmp eq i64 %213, 159
  br i1 %214, label %215, label %232

215:                                              ; preds = %212
  call void @match(i64 noundef 159)
  call void @expression(i64 noundef 162)
  %216 = load i64, ptr @expr_type, align 8
  %217 = icmp sge i64 %216, 2
  br i1 %217, label %218, label %221

218:                                              ; preds = %215
  %219 = load i64, ptr @expr_type, align 8
  %220 = sub nsw i64 %219, 2
  store i64 %220, ptr @expr_type, align 8
  br label %224

221:                                              ; preds = %215
  %222 = load i64, ptr @line, align 8
  %223 = call i32 (ptr, ...) @printf(ptr noundef @.str.9, i64 noundef %222)
  call void @exit(i32 noundef -1) #3
  unreachable

224:                                              ; preds = %218
  %225 = load i64, ptr @expr_type, align 8
  %226 = icmp eq i64 %225, 0
  %227 = zext i1 %226 to i64
  %228 = select i1 %226, i32 10, i32 9
  %229 = sext i32 %228 to i64
  %230 = load ptr, ptr @text, align 8
  %231 = getelementptr inbounds i64, ptr %230, i32 1
  store ptr %231, ptr @text, align 8
  store i64 %229, ptr %231, align 8
  br label %365

232:                                              ; preds = %212
  %233 = load i64, ptr @token, align 8
  %234 = icmp eq i64 %233, 148
  br i1 %234, label %235, label %252

235:                                              ; preds = %232
  call void @match(i64 noundef 148)
  call void @expression(i64 noundef 162)
  %236 = load ptr, ptr @text, align 8
  %237 = load i64, ptr %236, align 8
  %238 = icmp eq i64 %237, 10
  br i1 %238, label %243, label %239

239:                                              ; preds = %235
  %240 = load ptr, ptr @text, align 8
  %241 = load i64, ptr %240, align 8
  %242 = icmp eq i64 %241, 9
  br i1 %242, label %243, label %246

243:                                              ; preds = %239, %235
  %244 = load ptr, ptr @text, align 8
  %245 = getelementptr inbounds i64, ptr %244, i32 -1
  store ptr %245, ptr @text, align 8
  br label %249

246:                                              ; preds = %239
  %247 = load i64, ptr @line, align 8
  %248 = call i32 (ptr, ...) @printf(ptr noundef @.str.10, i64 noundef %247)
  call void @exit(i32 noundef -1) #3
  unreachable

249:                                              ; preds = %243
  %250 = load i64, ptr @expr_type, align 8
  %251 = add nsw i64 %250, 2
  store i64 %251, ptr @expr_type, align 8
  br label %364

252:                                              ; preds = %232
  %253 = load i64, ptr @token, align 8
  %254 = icmp eq i64 %253, 33
  br i1 %254, label %255, label %264

255:                                              ; preds = %252
  call void @match(i64 noundef 33)
  call void @expression(i64 noundef 162)
  %256 = load ptr, ptr @text, align 8
  %257 = getelementptr inbounds i64, ptr %256, i32 1
  store ptr %257, ptr @text, align 8
  store i64 13, ptr %257, align 8
  %258 = load ptr, ptr @text, align 8
  %259 = getelementptr inbounds i64, ptr %258, i32 1
  store ptr %259, ptr @text, align 8
  store i64 1, ptr %259, align 8
  %260 = load ptr, ptr @text, align 8
  %261 = getelementptr inbounds i64, ptr %260, i32 1
  store ptr %261, ptr @text, align 8
  store i64 0, ptr %261, align 8
  %262 = load ptr, ptr @text, align 8
  %263 = getelementptr inbounds i64, ptr %262, i32 1
  store ptr %263, ptr @text, align 8
  store i64 17, ptr %263, align 8
  store i64 1, ptr @expr_type, align 8
  br label %363

264:                                              ; preds = %252
  %265 = load i64, ptr @token, align 8
  %266 = icmp eq i64 %265, 126
  br i1 %266, label %267, label %276

267:                                              ; preds = %264
  call void @match(i64 noundef 126)
  call void @expression(i64 noundef 162)
  %268 = load ptr, ptr @text, align 8
  %269 = getelementptr inbounds i64, ptr %268, i32 1
  store ptr %269, ptr @text, align 8
  store i64 13, ptr %269, align 8
  %270 = load ptr, ptr @text, align 8
  %271 = getelementptr inbounds i64, ptr %270, i32 1
  store ptr %271, ptr @text, align 8
  store i64 1, ptr %271, align 8
  %272 = load ptr, ptr @text, align 8
  %273 = getelementptr inbounds i64, ptr %272, i32 1
  store ptr %273, ptr @text, align 8
  store i64 -1, ptr %273, align 8
  %274 = load ptr, ptr @text, align 8
  %275 = getelementptr inbounds i64, ptr %274, i32 1
  store ptr %275, ptr @text, align 8
  store i64 15, ptr %275, align 8
  store i64 1, ptr @expr_type, align 8
  br label %362

276:                                              ; preds = %264
  %277 = load i64, ptr @token, align 8
  %278 = icmp eq i64 %277, 157
  br i1 %278, label %279, label %280

279:                                              ; preds = %276
  call void @match(i64 noundef 157)
  call void @expression(i64 noundef 162)
  store i64 1, ptr @expr_type, align 8
  br label %361

280:                                              ; preds = %276
  %281 = load i64, ptr @token, align 8
  %282 = icmp eq i64 %281, 158
  br i1 %282, label %283, label %303

283:                                              ; preds = %280
  call void @match(i64 noundef 158)
  %284 = load i64, ptr @token, align 8
  %285 = icmp eq i64 %284, 128
  br i1 %285, label %286, label %293

286:                                              ; preds = %283
  %287 = load ptr, ptr @text, align 8
  %288 = getelementptr inbounds i64, ptr %287, i32 1
  store ptr %288, ptr @text, align 8
  store i64 1, ptr %288, align 8
  %289 = load i64, ptr @token_val, align 8
  %290 = sub nsw i64 0, %289
  %291 = load ptr, ptr @text, align 8
  %292 = getelementptr inbounds i64, ptr %291, i32 1
  store ptr %292, ptr @text, align 8
  store i64 %290, ptr %292, align 8
  call void @match(i64 noundef 128)
  br label %302

293:                                              ; preds = %283
  %294 = load ptr, ptr @text, align 8
  %295 = getelementptr inbounds i64, ptr %294, i32 1
  store ptr %295, ptr @text, align 8
  store i64 1, ptr %295, align 8
  %296 = load ptr, ptr @text, align 8
  %297 = getelementptr inbounds i64, ptr %296, i32 1
  store ptr %297, ptr @text, align 8
  store i64 -1, ptr %297, align 8
  %298 = load ptr, ptr @text, align 8
  %299 = getelementptr inbounds i64, ptr %298, i32 1
  store ptr %299, ptr @text, align 8
  store i64 13, ptr %299, align 8
  call void @expression(i64 noundef 162)
  %300 = load ptr, ptr @text, align 8
  %301 = getelementptr inbounds i64, ptr %300, i32 1
  store ptr %301, ptr @text, align 8
  store i64 27, ptr %301, align 8
  br label %302

302:                                              ; preds = %293, %286
  store i64 1, ptr @expr_type, align 8
  br label %360

303:                                              ; preds = %280
  %304 = load i64, ptr @token, align 8
  %305 = icmp eq i64 %304, 162
  br i1 %305, label %309, label %306

306:                                              ; preds = %303
  %307 = load i64, ptr @token, align 8
  %308 = icmp eq i64 %307, 163
  br i1 %308, label %309, label %356

309:                                              ; preds = %306, %303
  %310 = load i64, ptr @token, align 8
  store i64 %310, ptr %4, align 8
  %311 = load i64, ptr @token, align 8
  call void @match(i64 noundef %311)
  call void @expression(i64 noundef 162)
  %312 = load ptr, ptr @text, align 8
  %313 = load i64, ptr %312, align 8
  %314 = icmp eq i64 %313, 10
  br i1 %314, label %315, label %319

315:                                              ; preds = %309
  %316 = load ptr, ptr @text, align 8
  store i64 13, ptr %316, align 8
  %317 = load ptr, ptr @text, align 8
  %318 = getelementptr inbounds i64, ptr %317, i32 1
  store ptr %318, ptr @text, align 8
  store i64 10, ptr %318, align 8
  br label %331

319:                                              ; preds = %309
  %320 = load ptr, ptr @text, align 8
  %321 = load i64, ptr %320, align 8
  %322 = icmp eq i64 %321, 9
  br i1 %322, label %323, label %327

323:                                              ; preds = %319
  %324 = load ptr, ptr @text, align 8
  store i64 13, ptr %324, align 8
  %325 = load ptr, ptr @text, align 8
  %326 = getelementptr inbounds i64, ptr %325, i32 1
  store ptr %326, ptr @text, align 8
  store i64 9, ptr %326, align 8
  br label %330

327:                                              ; preds = %319
  %328 = load i64, ptr @line, align 8
  %329 = call i32 (ptr, ...) @printf(ptr noundef @.str.11, i64 noundef %328)
  call void @exit(i32 noundef -1) #3
  unreachable

330:                                              ; preds = %323
  br label %331

331:                                              ; preds = %330, %315
  %332 = load ptr, ptr @text, align 8
  %333 = getelementptr inbounds i64, ptr %332, i32 1
  store ptr %333, ptr @text, align 8
  store i64 13, ptr %333, align 8
  %334 = load ptr, ptr @text, align 8
  %335 = getelementptr inbounds i64, ptr %334, i32 1
  store ptr %335, ptr @text, align 8
  store i64 1, ptr %335, align 8
  %336 = load i64, ptr @expr_type, align 8
  %337 = icmp sgt i64 %336, 2
  %338 = zext i1 %337 to i64
  %339 = select i1 %337, i64 8, i64 1
  %340 = load ptr, ptr @text, align 8
  %341 = getelementptr inbounds i64, ptr %340, i32 1
  store ptr %341, ptr @text, align 8
  store i64 %339, ptr %341, align 8
  %342 = load i64, ptr %4, align 8
  %343 = icmp eq i64 %342, 162
  %344 = zext i1 %343 to i64
  %345 = select i1 %343, i32 25, i32 26
  %346 = sext i32 %345 to i64
  %347 = load ptr, ptr @text, align 8
  %348 = getelementptr inbounds i64, ptr %347, i32 1
  store ptr %348, ptr @text, align 8
  store i64 %346, ptr %348, align 8
  %349 = load i64, ptr @expr_type, align 8
  %350 = icmp eq i64 %349, 0
  %351 = zext i1 %350 to i64
  %352 = select i1 %350, i32 12, i32 11
  %353 = sext i32 %352 to i64
  %354 = load ptr, ptr @text, align 8
  %355 = getelementptr inbounds i64, ptr %354, i32 1
  store ptr %355, ptr @text, align 8
  store i64 %353, ptr %355, align 8
  br label %359

356:                                              ; preds = %306
  %357 = load i64, ptr @line, align 8
  %358 = call i32 (ptr, ...) @printf(ptr noundef @.str.12, i64 noundef %357)
  call void @exit(i32 noundef -1) #3
  unreachable

359:                                              ; preds = %331
  br label %360

360:                                              ; preds = %359, %302
  br label %361

361:                                              ; preds = %360, %279
  br label %362

362:                                              ; preds = %361, %267
  br label %363

363:                                              ; preds = %362, %255
  br label %364

364:                                              ; preds = %363, %249
  br label %365

365:                                              ; preds = %364, %224
  br label %366

366:                                              ; preds = %365, %211
  br label %367

367:                                              ; preds = %366, %185
  br label %368

368:                                              ; preds = %367, %58
  br label %369

369:                                              ; preds = %368, %33
  br label %370

370:                                              ; preds = %369, %14
  br label %371

371:                                              ; preds = %762, %370
  %372 = load i64, ptr @token, align 8
  %373 = load i64, ptr %2, align 8
  %374 = icmp sge i64 %372, %373
  br i1 %374, label %375, label %763

375:                                              ; preds = %371
  %376 = load i64, ptr @expr_type, align 8
  store i64 %376, ptr %4, align 8
  %377 = load i64, ptr @token, align 8
  %378 = icmp eq i64 %377, 142
  br i1 %378, label %379, label %401

379:                                              ; preds = %375
  call void @match(i64 noundef 142)
  %380 = load ptr, ptr @text, align 8
  %381 = load i64, ptr %380, align 8
  %382 = icmp eq i64 %381, 10
  br i1 %382, label %387, label %383

383:                                              ; preds = %379
  %384 = load ptr, ptr @text, align 8
  %385 = load i64, ptr %384, align 8
  %386 = icmp eq i64 %385, 9
  br i1 %386, label %387, label %389

387:                                              ; preds = %383, %379
  %388 = load ptr, ptr @text, align 8
  store i64 13, ptr %388, align 8
  br label %392

389:                                              ; preds = %383
  %390 = load i64, ptr @line, align 8
  %391 = call i32 (ptr, ...) @printf(ptr noundef @.str.13, i64 noundef %390)
  call void @exit(i32 noundef -1) #3
  unreachable

392:                                              ; preds = %387
  call void @expression(i64 noundef 142)
  %393 = load i64, ptr %4, align 8
  store i64 %393, ptr @expr_type, align 8
  %394 = load i64, ptr @expr_type, align 8
  %395 = icmp eq i64 %394, 0
  %396 = zext i1 %395 to i64
  %397 = select i1 %395, i32 12, i32 11
  %398 = sext i32 %397 to i64
  %399 = load ptr, ptr @text, align 8
  %400 = getelementptr inbounds i64, ptr %399, i32 1
  store ptr %400, ptr @text, align 8
  store i64 %398, ptr %400, align 8
  br label %762

401:                                              ; preds = %375
  %402 = load i64, ptr @token, align 8
  %403 = icmp eq i64 %402, 143
  br i1 %403, label %404, label %428

404:                                              ; preds = %401
  call void @match(i64 noundef 143)
  %405 = load ptr, ptr @text, align 8
  %406 = getelementptr inbounds i64, ptr %405, i32 1
  store ptr %406, ptr @text, align 8
  store i64 4, ptr %406, align 8
  %407 = load ptr, ptr @text, align 8
  %408 = getelementptr inbounds i64, ptr %407, i32 1
  store ptr %408, ptr @text, align 8
  store ptr %408, ptr %5, align 8
  call void @expression(i64 noundef 142)
  %409 = load i64, ptr @token, align 8
  %410 = icmp eq i64 %409, 58
  br i1 %410, label %411, label %412

411:                                              ; preds = %404
  call void @match(i64 noundef 58)
  br label %415

412:                                              ; preds = %404
  %413 = load i64, ptr @line, align 8
  %414 = call i32 (ptr, ...) @printf(ptr noundef @.str.14, i64 noundef %413)
  call void @exit(i32 noundef -1) #3
  unreachable

415:                                              ; preds = %411
  %416 = load ptr, ptr @text, align 8
  %417 = getelementptr inbounds i64, ptr %416, i64 3
  %418 = ptrtoint ptr %417 to i64
  %419 = load ptr, ptr %5, align 8
  store i64 %418, ptr %419, align 8
  %420 = load ptr, ptr @text, align 8
  %421 = getelementptr inbounds i64, ptr %420, i32 1
  store ptr %421, ptr @text, align 8
  store i64 2, ptr %421, align 8
  %422 = load ptr, ptr @text, align 8
  %423 = getelementptr inbounds i64, ptr %422, i32 1
  store ptr %423, ptr @text, align 8
  store ptr %423, ptr %5, align 8
  call void @expression(i64 noundef 143)
  %424 = load ptr, ptr @text, align 8
  %425 = getelementptr inbounds i64, ptr %424, i64 1
  %426 = ptrtoint ptr %425 to i64
  %427 = load ptr, ptr %5, align 8
  store i64 %426, ptr %427, align 8
  br label %761

428:                                              ; preds = %401
  %429 = load i64, ptr @token, align 8
  %430 = icmp eq i64 %429, 144
  br i1 %430, label %431, label %440

431:                                              ; preds = %428
  call void @match(i64 noundef 144)
  %432 = load ptr, ptr @text, align 8
  %433 = getelementptr inbounds i64, ptr %432, i32 1
  store ptr %433, ptr @text, align 8
  store i64 5, ptr %433, align 8
  %434 = load ptr, ptr @text, align 8
  %435 = getelementptr inbounds i64, ptr %434, i32 1
  store ptr %435, ptr @text, align 8
  store ptr %435, ptr %5, align 8
  call void @expression(i64 noundef 145)
  %436 = load ptr, ptr @text, align 8
  %437 = getelementptr inbounds i64, ptr %436, i64 1
  %438 = ptrtoint ptr %437 to i64
  %439 = load ptr, ptr %5, align 8
  store i64 %438, ptr %439, align 8
  store i64 1, ptr @expr_type, align 8
  br label %760

440:                                              ; preds = %428
  %441 = load i64, ptr @token, align 8
  %442 = icmp eq i64 %441, 145
  br i1 %442, label %443, label %452

443:                                              ; preds = %440
  call void @match(i64 noundef 145)
  %444 = load ptr, ptr @text, align 8
  %445 = getelementptr inbounds i64, ptr %444, i32 1
  store ptr %445, ptr @text, align 8
  store i64 4, ptr %445, align 8
  %446 = load ptr, ptr @text, align 8
  %447 = getelementptr inbounds i64, ptr %446, i32 1
  store ptr %447, ptr @text, align 8
  store ptr %447, ptr %5, align 8
  call void @expression(i64 noundef 146)
  %448 = load ptr, ptr @text, align 8
  %449 = getelementptr inbounds i64, ptr %448, i64 1
  %450 = ptrtoint ptr %449 to i64
  %451 = load ptr, ptr %5, align 8
  store i64 %450, ptr %451, align 8
  store i64 1, ptr @expr_type, align 8
  br label %759

452:                                              ; preds = %440
  %453 = load i64, ptr @token, align 8
  %454 = icmp eq i64 %453, 146
  br i1 %454, label %455, label %460

455:                                              ; preds = %452
  call void @match(i64 noundef 146)
  %456 = load ptr, ptr @text, align 8
  %457 = getelementptr inbounds i64, ptr %456, i32 1
  store ptr %457, ptr @text, align 8
  store i64 13, ptr %457, align 8
  call void @expression(i64 noundef 147)
  %458 = load ptr, ptr @text, align 8
  %459 = getelementptr inbounds i64, ptr %458, i32 1
  store ptr %459, ptr @text, align 8
  store i64 14, ptr %459, align 8
  store i64 1, ptr @expr_type, align 8
  br label %758

460:                                              ; preds = %452
  %461 = load i64, ptr @token, align 8
  %462 = icmp eq i64 %461, 147
  br i1 %462, label %463, label %468

463:                                              ; preds = %460
  call void @match(i64 noundef 147)
  %464 = load ptr, ptr @text, align 8
  %465 = getelementptr inbounds i64, ptr %464, i32 1
  store ptr %465, ptr @text, align 8
  store i64 13, ptr %465, align 8
  call void @expression(i64 noundef 148)
  %466 = load ptr, ptr @text, align 8
  %467 = getelementptr inbounds i64, ptr %466, i32 1
  store ptr %467, ptr @text, align 8
  store i64 15, ptr %467, align 8
  store i64 1, ptr @expr_type, align 8
  br label %757

468:                                              ; preds = %460
  %469 = load i64, ptr @token, align 8
  %470 = icmp eq i64 %469, 148
  br i1 %470, label %471, label %476

471:                                              ; preds = %468
  call void @match(i64 noundef 148)
  %472 = load ptr, ptr @text, align 8
  %473 = getelementptr inbounds i64, ptr %472, i32 1
  store ptr %473, ptr @text, align 8
  store i64 13, ptr %473, align 8
  call void @expression(i64 noundef 149)
  %474 = load ptr, ptr @text, align 8
  %475 = getelementptr inbounds i64, ptr %474, i32 1
  store ptr %475, ptr @text, align 8
  store i64 16, ptr %475, align 8
  store i64 1, ptr @expr_type, align 8
  br label %756

476:                                              ; preds = %468
  %477 = load i64, ptr @token, align 8
  %478 = icmp eq i64 %477, 149
  br i1 %478, label %479, label %484

479:                                              ; preds = %476
  call void @match(i64 noundef 149)
  %480 = load ptr, ptr @text, align 8
  %481 = getelementptr inbounds i64, ptr %480, i32 1
  store ptr %481, ptr @text, align 8
  store i64 13, ptr %481, align 8
  call void @expression(i64 noundef 150)
  %482 = load ptr, ptr @text, align 8
  %483 = getelementptr inbounds i64, ptr %482, i32 1
  store ptr %483, ptr @text, align 8
  store i64 17, ptr %483, align 8
  store i64 1, ptr @expr_type, align 8
  br label %755

484:                                              ; preds = %476
  %485 = load i64, ptr @token, align 8
  %486 = icmp eq i64 %485, 150
  br i1 %486, label %487, label %492

487:                                              ; preds = %484
  call void @match(i64 noundef 150)
  %488 = load ptr, ptr @text, align 8
  %489 = getelementptr inbounds i64, ptr %488, i32 1
  store ptr %489, ptr @text, align 8
  store i64 13, ptr %489, align 8
  call void @expression(i64 noundef 151)
  %490 = load ptr, ptr @text, align 8
  %491 = getelementptr inbounds i64, ptr %490, i32 1
  store ptr %491, ptr @text, align 8
  store i64 18, ptr %491, align 8
  store i64 1, ptr @expr_type, align 8
  br label %754

492:                                              ; preds = %484
  %493 = load i64, ptr @token, align 8
  %494 = icmp eq i64 %493, 151
  br i1 %494, label %495, label %500

495:                                              ; preds = %492
  call void @match(i64 noundef 151)
  %496 = load ptr, ptr @text, align 8
  %497 = getelementptr inbounds i64, ptr %496, i32 1
  store ptr %497, ptr @text, align 8
  store i64 13, ptr %497, align 8
  call void @expression(i64 noundef 155)
  %498 = load ptr, ptr @text, align 8
  %499 = getelementptr inbounds i64, ptr %498, i32 1
  store ptr %499, ptr @text, align 8
  store i64 19, ptr %499, align 8
  store i64 1, ptr @expr_type, align 8
  br label %753

500:                                              ; preds = %492
  %501 = load i64, ptr @token, align 8
  %502 = icmp eq i64 %501, 152
  br i1 %502, label %503, label %508

503:                                              ; preds = %500
  call void @match(i64 noundef 152)
  %504 = load ptr, ptr @text, align 8
  %505 = getelementptr inbounds i64, ptr %504, i32 1
  store ptr %505, ptr @text, align 8
  store i64 13, ptr %505, align 8
  call void @expression(i64 noundef 155)
  %506 = load ptr, ptr @text, align 8
  %507 = getelementptr inbounds i64, ptr %506, i32 1
  store ptr %507, ptr @text, align 8
  store i64 20, ptr %507, align 8
  store i64 1, ptr @expr_type, align 8
  br label %752

508:                                              ; preds = %500
  %509 = load i64, ptr @token, align 8
  %510 = icmp eq i64 %509, 153
  br i1 %510, label %511, label %516

511:                                              ; preds = %508
  call void @match(i64 noundef 153)
  %512 = load ptr, ptr @text, align 8
  %513 = getelementptr inbounds i64, ptr %512, i32 1
  store ptr %513, ptr @text, align 8
  store i64 13, ptr %513, align 8
  call void @expression(i64 noundef 155)
  %514 = load ptr, ptr @text, align 8
  %515 = getelementptr inbounds i64, ptr %514, i32 1
  store ptr %515, ptr @text, align 8
  store i64 21, ptr %515, align 8
  store i64 1, ptr @expr_type, align 8
  br label %751

516:                                              ; preds = %508
  %517 = load i64, ptr @token, align 8
  %518 = icmp eq i64 %517, 154
  br i1 %518, label %519, label %524

519:                                              ; preds = %516
  call void @match(i64 noundef 154)
  %520 = load ptr, ptr @text, align 8
  %521 = getelementptr inbounds i64, ptr %520, i32 1
  store ptr %521, ptr @text, align 8
  store i64 13, ptr %521, align 8
  call void @expression(i64 noundef 155)
  %522 = load ptr, ptr @text, align 8
  %523 = getelementptr inbounds i64, ptr %522, i32 1
  store ptr %523, ptr @text, align 8
  store i64 22, ptr %523, align 8
  store i64 1, ptr @expr_type, align 8
  br label %750

524:                                              ; preds = %516
  %525 = load i64, ptr @token, align 8
  %526 = icmp eq i64 %525, 155
  br i1 %526, label %527, label %532

527:                                              ; preds = %524
  call void @match(i64 noundef 155)
  %528 = load ptr, ptr @text, align 8
  %529 = getelementptr inbounds i64, ptr %528, i32 1
  store ptr %529, ptr @text, align 8
  store i64 13, ptr %529, align 8
  call void @expression(i64 noundef 157)
  %530 = load ptr, ptr @text, align 8
  %531 = getelementptr inbounds i64, ptr %530, i32 1
  store ptr %531, ptr @text, align 8
  store i64 23, ptr %531, align 8
  store i64 1, ptr @expr_type, align 8
  br label %749

532:                                              ; preds = %524
  %533 = load i64, ptr @token, align 8
  %534 = icmp eq i64 %533, 156
  br i1 %534, label %535, label %540

535:                                              ; preds = %532
  call void @match(i64 noundef 156)
  %536 = load ptr, ptr @text, align 8
  %537 = getelementptr inbounds i64, ptr %536, i32 1
  store ptr %537, ptr @text, align 8
  store i64 13, ptr %537, align 8
  call void @expression(i64 noundef 157)
  %538 = load ptr, ptr @text, align 8
  %539 = getelementptr inbounds i64, ptr %538, i32 1
  store ptr %539, ptr @text, align 8
  store i64 24, ptr %539, align 8
  store i64 1, ptr @expr_type, align 8
  br label %748

540:                                              ; preds = %532
  %541 = load i64, ptr @token, align 8
  %542 = icmp eq i64 %541, 157
  br i1 %542, label %543, label %561

543:                                              ; preds = %540
  call void @match(i64 noundef 157)
  %544 = load ptr, ptr @text, align 8
  %545 = getelementptr inbounds i64, ptr %544, i32 1
  store ptr %545, ptr @text, align 8
  store i64 13, ptr %545, align 8
  call void @expression(i64 noundef 159)
  %546 = load i64, ptr %4, align 8
  store i64 %546, ptr @expr_type, align 8
  %547 = load i64, ptr @expr_type, align 8
  %548 = icmp sgt i64 %547, 2
  br i1 %548, label %549, label %558

549:                                              ; preds = %543
  %550 = load ptr, ptr @text, align 8
  %551 = getelementptr inbounds i64, ptr %550, i32 1
  store ptr %551, ptr @text, align 8
  store i64 13, ptr %551, align 8
  %552 = load ptr, ptr @text, align 8
  %553 = getelementptr inbounds i64, ptr %552, i32 1
  store ptr %553, ptr @text, align 8
  store i64 1, ptr %553, align 8
  %554 = load ptr, ptr @text, align 8
  %555 = getelementptr inbounds i64, ptr %554, i32 1
  store ptr %555, ptr @text, align 8
  store i64 8, ptr %555, align 8
  %556 = load ptr, ptr @text, align 8
  %557 = getelementptr inbounds i64, ptr %556, i32 1
  store ptr %557, ptr @text, align 8
  store i64 27, ptr %557, align 8
  br label %558

558:                                              ; preds = %549, %543
  %559 = load ptr, ptr @text, align 8
  %560 = getelementptr inbounds i64, ptr %559, i32 1
  store ptr %560, ptr @text, align 8
  store i64 25, ptr %560, align 8
  br label %747

561:                                              ; preds = %540
  %562 = load i64, ptr @token, align 8
  %563 = icmp eq i64 %562, 158
  br i1 %563, label %564, label %605

564:                                              ; preds = %561
  call void @match(i64 noundef 158)
  %565 = load ptr, ptr @text, align 8
  %566 = getelementptr inbounds i64, ptr %565, i32 1
  store ptr %566, ptr @text, align 8
  store i64 13, ptr %566, align 8
  call void @expression(i64 noundef 159)
  %567 = load i64, ptr %4, align 8
  %568 = icmp sgt i64 %567, 2
  br i1 %568, label %569, label %584

569:                                              ; preds = %564
  %570 = load i64, ptr %4, align 8
  %571 = load i64, ptr @expr_type, align 8
  %572 = icmp eq i64 %570, %571
  br i1 %572, label %573, label %584

573:                                              ; preds = %569
  %574 = load ptr, ptr @text, align 8
  %575 = getelementptr inbounds i64, ptr %574, i32 1
  store ptr %575, ptr @text, align 8
  store i64 26, ptr %575, align 8
  %576 = load ptr, ptr @text, align 8
  %577 = getelementptr inbounds i64, ptr %576, i32 1
  store ptr %577, ptr @text, align 8
  store i64 13, ptr %577, align 8
  %578 = load ptr, ptr @text, align 8
  %579 = getelementptr inbounds i64, ptr %578, i32 1
  store ptr %579, ptr @text, align 8
  store i64 1, ptr %579, align 8
  %580 = load ptr, ptr @text, align 8
  %581 = getelementptr inbounds i64, ptr %580, i32 1
  store ptr %581, ptr @text, align 8
  store i64 8, ptr %581, align 8
  %582 = load ptr, ptr @text, align 8
  %583 = getelementptr inbounds i64, ptr %582, i32 1
  store ptr %583, ptr @text, align 8
  store i64 28, ptr %583, align 8
  store i64 1, ptr @expr_type, align 8
  br label %604

584:                                              ; preds = %569, %564
  %585 = load i64, ptr %4, align 8
  %586 = icmp sgt i64 %585, 2
  br i1 %586, label %587, label %599

587:                                              ; preds = %584
  %588 = load ptr, ptr @text, align 8
  %589 = getelementptr inbounds i64, ptr %588, i32 1
  store ptr %589, ptr @text, align 8
  store i64 13, ptr %589, align 8
  %590 = load ptr, ptr @text, align 8
  %591 = getelementptr inbounds i64, ptr %590, i32 1
  store ptr %591, ptr @text, align 8
  store i64 1, ptr %591, align 8
  %592 = load ptr, ptr @text, align 8
  %593 = getelementptr inbounds i64, ptr %592, i32 1
  store ptr %593, ptr @text, align 8
  store i64 8, ptr %593, align 8
  %594 = load ptr, ptr @text, align 8
  %595 = getelementptr inbounds i64, ptr %594, i32 1
  store ptr %595, ptr @text, align 8
  store i64 27, ptr %595, align 8
  %596 = load ptr, ptr @text, align 8
  %597 = getelementptr inbounds i64, ptr %596, i32 1
  store ptr %597, ptr @text, align 8
  store i64 26, ptr %597, align 8
  %598 = load i64, ptr %4, align 8
  store i64 %598, ptr @expr_type, align 8
  br label %603

599:                                              ; preds = %584
  %600 = load ptr, ptr @text, align 8
  %601 = getelementptr inbounds i64, ptr %600, i32 1
  store ptr %601, ptr @text, align 8
  store i64 26, ptr %601, align 8
  %602 = load i64, ptr %4, align 8
  store i64 %602, ptr @expr_type, align 8
  br label %603

603:                                              ; preds = %599, %587
  br label %604

604:                                              ; preds = %603, %573
  br label %746

605:                                              ; preds = %561
  %606 = load i64, ptr @token, align 8
  %607 = icmp eq i64 %606, 159
  br i1 %607, label %608, label %614

608:                                              ; preds = %605
  call void @match(i64 noundef 159)
  %609 = load ptr, ptr @text, align 8
  %610 = getelementptr inbounds i64, ptr %609, i32 1
  store ptr %610, ptr @text, align 8
  store i64 13, ptr %610, align 8
  call void @expression(i64 noundef 162)
  %611 = load ptr, ptr @text, align 8
  %612 = getelementptr inbounds i64, ptr %611, i32 1
  store ptr %612, ptr @text, align 8
  store i64 27, ptr %612, align 8
  %613 = load i64, ptr %4, align 8
  store i64 %613, ptr @expr_type, align 8
  br label %745

614:                                              ; preds = %605
  %615 = load i64, ptr @token, align 8
  %616 = icmp eq i64 %615, 160
  br i1 %616, label %617, label %623

617:                                              ; preds = %614
  call void @match(i64 noundef 160)
  %618 = load ptr, ptr @text, align 8
  %619 = getelementptr inbounds i64, ptr %618, i32 1
  store ptr %619, ptr @text, align 8
  store i64 13, ptr %619, align 8
  call void @expression(i64 noundef 162)
  %620 = load ptr, ptr @text, align 8
  %621 = getelementptr inbounds i64, ptr %620, i32 1
  store ptr %621, ptr @text, align 8
  store i64 28, ptr %621, align 8
  %622 = load i64, ptr %4, align 8
  store i64 %622, ptr @expr_type, align 8
  br label %744

623:                                              ; preds = %614
  %624 = load i64, ptr @token, align 8
  %625 = icmp eq i64 %624, 161
  br i1 %625, label %626, label %632

626:                                              ; preds = %623
  call void @match(i64 noundef 161)
  %627 = load ptr, ptr @text, align 8
  %628 = getelementptr inbounds i64, ptr %627, i32 1
  store ptr %628, ptr @text, align 8
  store i64 13, ptr %628, align 8
  call void @expression(i64 noundef 162)
  %629 = load ptr, ptr @text, align 8
  %630 = getelementptr inbounds i64, ptr %629, i32 1
  store ptr %630, ptr @text, align 8
  store i64 29, ptr %630, align 8
  %631 = load i64, ptr %4, align 8
  store i64 %631, ptr @expr_type, align 8
  br label %743

632:                                              ; preds = %623
  %633 = load i64, ptr @token, align 8
  %634 = icmp eq i64 %633, 162
  br i1 %634, label %638, label %635

635:                                              ; preds = %632
  %636 = load i64, ptr @token, align 8
  %637 = icmp eq i64 %636, 163
  br i1 %637, label %638, label %701

638:                                              ; preds = %635, %632
  %639 = load ptr, ptr @text, align 8
  %640 = load i64, ptr %639, align 8
  %641 = icmp eq i64 %640, 9
  br i1 %641, label %642, label %646

642:                                              ; preds = %638
  %643 = load ptr, ptr @text, align 8
  store i64 13, ptr %643, align 8
  %644 = load ptr, ptr @text, align 8
  %645 = getelementptr inbounds i64, ptr %644, i32 1
  store ptr %645, ptr @text, align 8
  store i64 9, ptr %645, align 8
  br label %658

646:                                              ; preds = %638
  %647 = load ptr, ptr @text, align 8
  %648 = load i64, ptr %647, align 8
  %649 = icmp eq i64 %648, 10
  br i1 %649, label %650, label %654

650:                                              ; preds = %646
  %651 = load ptr, ptr @text, align 8
  store i64 13, ptr %651, align 8
  %652 = load ptr, ptr @text, align 8
  %653 = getelementptr inbounds i64, ptr %652, i32 1
  store ptr %653, ptr @text, align 8
  store i64 10, ptr %653, align 8
  br label %657

654:                                              ; preds = %646
  %655 = load i64, ptr @line, align 8
  %656 = call i32 (ptr, ...) @printf(ptr noundef @.str.15, i64 noundef %655)
  call void @exit(i32 noundef -1) #3
  unreachable

657:                                              ; preds = %650
  br label %658

658:                                              ; preds = %657, %642
  %659 = load ptr, ptr @text, align 8
  %660 = getelementptr inbounds i64, ptr %659, i32 1
  store ptr %660, ptr @text, align 8
  store i64 13, ptr %660, align 8
  %661 = load ptr, ptr @text, align 8
  %662 = getelementptr inbounds i64, ptr %661, i32 1
  store ptr %662, ptr @text, align 8
  store i64 1, ptr %662, align 8
  %663 = load i64, ptr @expr_type, align 8
  %664 = icmp sgt i64 %663, 2
  %665 = zext i1 %664 to i64
  %666 = select i1 %664, i64 8, i64 1
  %667 = load ptr, ptr @text, align 8
  %668 = getelementptr inbounds i64, ptr %667, i32 1
  store ptr %668, ptr @text, align 8
  store i64 %666, ptr %668, align 8
  %669 = load i64, ptr @token, align 8
  %670 = icmp eq i64 %669, 162
  %671 = zext i1 %670 to i64
  %672 = select i1 %670, i32 25, i32 26
  %673 = sext i32 %672 to i64
  %674 = load ptr, ptr @text, align 8
  %675 = getelementptr inbounds i64, ptr %674, i32 1
  store ptr %675, ptr @text, align 8
  store i64 %673, ptr %675, align 8
  %676 = load i64, ptr @expr_type, align 8
  %677 = icmp eq i64 %676, 0
  %678 = zext i1 %677 to i64
  %679 = select i1 %677, i32 12, i32 11
  %680 = sext i32 %679 to i64
  %681 = load ptr, ptr @text, align 8
  %682 = getelementptr inbounds i64, ptr %681, i32 1
  store ptr %682, ptr @text, align 8
  store i64 %680, ptr %682, align 8
  %683 = load ptr, ptr @text, align 8
  %684 = getelementptr inbounds i64, ptr %683, i32 1
  store ptr %684, ptr @text, align 8
  store i64 13, ptr %684, align 8
  %685 = load ptr, ptr @text, align 8
  %686 = getelementptr inbounds i64, ptr %685, i32 1
  store ptr %686, ptr @text, align 8
  store i64 1, ptr %686, align 8
  %687 = load i64, ptr @expr_type, align 8
  %688 = icmp sgt i64 %687, 2
  %689 = zext i1 %688 to i64
  %690 = select i1 %688, i64 8, i64 1
  %691 = load ptr, ptr @text, align 8
  %692 = getelementptr inbounds i64, ptr %691, i32 1
  store ptr %692, ptr @text, align 8
  store i64 %690, ptr %692, align 8
  %693 = load i64, ptr @token, align 8
  %694 = icmp eq i64 %693, 162
  %695 = zext i1 %694 to i64
  %696 = select i1 %694, i32 26, i32 25
  %697 = sext i32 %696 to i64
  %698 = load ptr, ptr @text, align 8
  %699 = getelementptr inbounds i64, ptr %698, i32 1
  store ptr %699, ptr @text, align 8
  store i64 %697, ptr %699, align 8
  %700 = load i64, ptr @token, align 8
  call void @match(i64 noundef %700)
  br label %742

701:                                              ; preds = %635
  %702 = load i64, ptr @token, align 8
  %703 = icmp eq i64 %702, 164
  br i1 %703, label %704, label %735

704:                                              ; preds = %701
  call void @match(i64 noundef 164)
  %705 = load ptr, ptr @text, align 8
  %706 = getelementptr inbounds i64, ptr %705, i32 1
  store ptr %706, ptr @text, align 8
  store i64 13, ptr %706, align 8
  call void @expression(i64 noundef 142)
  call void @match(i64 noundef 93)
  %707 = load i64, ptr %4, align 8
  %708 = icmp sgt i64 %707, 2
  br i1 %708, label %709, label %716

709:                                              ; preds = %704
  %710 = load ptr, ptr @text, align 8
  %711 = getelementptr inbounds i64, ptr %710, i32 1
  store ptr %711, ptr @text, align 8
  store i64 1, ptr %711, align 8
  %712 = load ptr, ptr @text, align 8
  %713 = getelementptr inbounds i64, ptr %712, i32 1
  store ptr %713, ptr @text, align 8
  store i64 8, ptr %713, align 8
  %714 = load ptr, ptr @text, align 8
  %715 = getelementptr inbounds i64, ptr %714, i32 1
  store ptr %715, ptr @text, align 8
  store i64 27, ptr %715, align 8
  br label %723

716:                                              ; preds = %704
  %717 = load i64, ptr %4, align 8
  %718 = icmp slt i64 %717, 2
  br i1 %718, label %719, label %722

719:                                              ; preds = %716
  %720 = load i64, ptr @line, align 8
  %721 = call i32 (ptr, ...) @printf(ptr noundef @.str.16, i64 noundef %720)
  call void @exit(i32 noundef -1) #3
  unreachable

722:                                              ; preds = %716
  br label %723

723:                                              ; preds = %722, %709
  %724 = load i64, ptr %4, align 8
  %725 = sub nsw i64 %724, 2
  store i64 %725, ptr @expr_type, align 8
  %726 = load ptr, ptr @text, align 8
  %727 = getelementptr inbounds i64, ptr %726, i32 1
  store ptr %727, ptr @text, align 8
  store i64 25, ptr %727, align 8
  %728 = load i64, ptr @expr_type, align 8
  %729 = icmp eq i64 %728, 0
  %730 = zext i1 %729 to i64
  %731 = select i1 %729, i32 10, i32 9
  %732 = sext i32 %731 to i64
  %733 = load ptr, ptr @text, align 8
  %734 = getelementptr inbounds i64, ptr %733, i32 1
  store ptr %734, ptr @text, align 8
  store i64 %732, ptr %734, align 8
  br label %741

735:                                              ; preds = %701
  %736 = load ptr, ptr @text, align 8
  %737 = getelementptr inbounds i64, ptr %736, i32 1
  store ptr %737, ptr @text, align 8
  store i64 13, ptr %737, align 8
  %738 = load i64, ptr @line, align 8
  %739 = load i64, ptr @token, align 8
  %740 = call i32 (ptr, ...) @printf(ptr noundef @.str.17, i64 noundef %738, i64 noundef %739)
  call void @exit(i32 noundef -1) #3
  unreachable

741:                                              ; preds = %723
  br label %742

742:                                              ; preds = %741, %658
  br label %743

743:                                              ; preds = %742, %626
  br label %744

744:                                              ; preds = %743, %617
  br label %745

745:                                              ; preds = %744, %608
  br label %746

746:                                              ; preds = %745, %604
  br label %747

747:                                              ; preds = %746, %558
  br label %748

748:                                              ; preds = %747, %535
  br label %749

749:                                              ; preds = %748, %527
  br label %750

750:                                              ; preds = %749, %519
  br label %751

751:                                              ; preds = %750, %511
  br label %752

752:                                              ; preds = %751, %503
  br label %753

753:                                              ; preds = %752, %495
  br label %754

754:                                              ; preds = %753, %487
  br label %755

755:                                              ; preds = %754, %479
  br label %756

756:                                              ; preds = %755, %471
  br label %757

757:                                              ; preds = %756, %463
  br label %758

758:                                              ; preds = %757, %455
  br label %759

759:                                              ; preds = %758, %443
  br label %760

760:                                              ; preds = %759, %431
  br label %761

761:                                              ; preds = %760, %415
  br label %762

762:                                              ; preds = %761, %392
  br label %371, !llvm.loop !21

763:                                              ; preds = %371
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @statement() #0 {
  %1 = alloca ptr, align 8
  %2 = alloca ptr, align 8
  %3 = load i64, ptr @token, align 8
  %4 = icmp eq i64 %3, 137
  br i1 %4, label %5, label %26

5:                                                ; preds = %0
  call void @match(i64 noundef 137)
  call void @match(i64 noundef 40)
  call void @expression(i64 noundef 142)
  call void @match(i64 noundef 41)
  %6 = load ptr, ptr @text, align 8
  %7 = getelementptr inbounds i64, ptr %6, i32 1
  store ptr %7, ptr @text, align 8
  store i64 4, ptr %7, align 8
  %8 = load ptr, ptr @text, align 8
  %9 = getelementptr inbounds i64, ptr %8, i32 1
  store ptr %9, ptr @text, align 8
  store ptr %9, ptr %2, align 8
  call void @statement()
  %10 = load i64, ptr @token, align 8
  %11 = icmp eq i64 %10, 135
  br i1 %11, label %12, label %21

12:                                               ; preds = %5
  call void @match(i64 noundef 135)
  %13 = load ptr, ptr @text, align 8
  %14 = getelementptr inbounds i64, ptr %13, i64 3
  %15 = ptrtoint ptr %14 to i64
  %16 = load ptr, ptr %2, align 8
  store i64 %15, ptr %16, align 8
  %17 = load ptr, ptr @text, align 8
  %18 = getelementptr inbounds i64, ptr %17, i32 1
  store ptr %18, ptr @text, align 8
  store i64 2, ptr %18, align 8
  %19 = load ptr, ptr @text, align 8
  %20 = getelementptr inbounds i64, ptr %19, i32 1
  store ptr %20, ptr @text, align 8
  store ptr %20, ptr %2, align 8
  call void @statement()
  br label %21

21:                                               ; preds = %12, %5
  %22 = load ptr, ptr @text, align 8
  %23 = getelementptr inbounds i64, ptr %22, i64 1
  %24 = ptrtoint ptr %23 to i64
  %25 = load ptr, ptr %2, align 8
  store i64 %24, ptr %25, align 8
  br label %74

26:                                               ; preds = %0
  %27 = load i64, ptr @token, align 8
  %28 = icmp eq i64 %27, 141
  br i1 %28, label %29, label %46

29:                                               ; preds = %26
  call void @match(i64 noundef 141)
  %30 = load ptr, ptr @text, align 8
  %31 = getelementptr inbounds i64, ptr %30, i64 1
  store ptr %31, ptr %1, align 8
  call void @match(i64 noundef 40)
  call void @expression(i64 noundef 142)
  call void @match(i64 noundef 41)
  %32 = load ptr, ptr @text, align 8
  %33 = getelementptr inbounds i64, ptr %32, i32 1
  store ptr %33, ptr @text, align 8
  store i64 4, ptr %33, align 8
  %34 = load ptr, ptr @text, align 8
  %35 = getelementptr inbounds i64, ptr %34, i32 1
  store ptr %35, ptr @text, align 8
  store ptr %35, ptr %2, align 8
  call void @statement()
  %36 = load ptr, ptr @text, align 8
  %37 = getelementptr inbounds i64, ptr %36, i32 1
  store ptr %37, ptr @text, align 8
  store i64 2, ptr %37, align 8
  %38 = load ptr, ptr %1, align 8
  %39 = ptrtoint ptr %38 to i64
  %40 = load ptr, ptr @text, align 8
  %41 = getelementptr inbounds i64, ptr %40, i32 1
  store ptr %41, ptr @text, align 8
  store i64 %39, ptr %41, align 8
  %42 = load ptr, ptr @text, align 8
  %43 = getelementptr inbounds i64, ptr %42, i64 1
  %44 = ptrtoint ptr %43 to i64
  %45 = load ptr, ptr %2, align 8
  store i64 %44, ptr %45, align 8
  br label %73

46:                                               ; preds = %26
  %47 = load i64, ptr @token, align 8
  %48 = icmp eq i64 %47, 123
  br i1 %48, label %49, label %55

49:                                               ; preds = %46
  call void @match(i64 noundef 123)
  br label %50

50:                                               ; preds = %53, %49
  %51 = load i64, ptr @token, align 8
  %52 = icmp ne i64 %51, 125
  br i1 %52, label %53, label %54

53:                                               ; preds = %50
  call void @statement()
  br label %50, !llvm.loop !22

54:                                               ; preds = %50
  call void @match(i64 noundef 125)
  br label %72

55:                                               ; preds = %46
  %56 = load i64, ptr @token, align 8
  %57 = icmp eq i64 %56, 139
  br i1 %57, label %58, label %65

58:                                               ; preds = %55
  call void @match(i64 noundef 139)
  %59 = load i64, ptr @token, align 8
  %60 = icmp ne i64 %59, 59
  br i1 %60, label %61, label %62

61:                                               ; preds = %58
  call void @expression(i64 noundef 142)
  br label %62

62:                                               ; preds = %61, %58
  call void @match(i64 noundef 59)
  %63 = load ptr, ptr @text, align 8
  %64 = getelementptr inbounds i64, ptr %63, i32 1
  store ptr %64, ptr @text, align 8
  store i64 8, ptr %64, align 8
  br label %71

65:                                               ; preds = %55
  %66 = load i64, ptr @token, align 8
  %67 = icmp eq i64 %66, 59
  br i1 %67, label %68, label %69

68:                                               ; preds = %65
  call void @match(i64 noundef 59)
  br label %70

69:                                               ; preds = %65
  call void @expression(i64 noundef 142)
  call void @match(i64 noundef 59)
  br label %70

70:                                               ; preds = %69, %68
  br label %71

71:                                               ; preds = %70, %62
  br label %72

72:                                               ; preds = %71, %54
  br label %73

73:                                               ; preds = %72, %29
  br label %74

74:                                               ; preds = %73, %21
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @enum_declaration() #0 {
  %1 = alloca i64, align 8
  store i64 0, ptr %1, align 8
  br label %2

2:                                                ; preds = %35, %0
  %3 = load i64, ptr @token, align 8
  %4 = icmp ne i64 %3, 125
  br i1 %4, label %5, label %36

5:                                                ; preds = %2
  %6 = load i64, ptr @token, align 8
  %7 = icmp ne i64 %6, 133
  br i1 %7, label %8, label %12

8:                                                ; preds = %5
  %9 = load i64, ptr @line, align 8
  %10 = load i64, ptr @token, align 8
  %11 = call i32 (ptr, ...) @printf(ptr noundef @.str.18, i64 noundef %9, i64 noundef %10)
  call void @exit(i32 noundef -1) #3
  unreachable

12:                                               ; preds = %5
  call void @next()
  %13 = load i64, ptr @token, align 8
  %14 = icmp eq i64 %13, 142
  br i1 %14, label %15, label %23

15:                                               ; preds = %12
  call void @next()
  %16 = load i64, ptr @token, align 8
  %17 = icmp ne i64 %16, 128
  br i1 %17, label %18, label %21

18:                                               ; preds = %15
  %19 = load i64, ptr @line, align 8
  %20 = call i32 (ptr, ...) @printf(ptr noundef @.str.19, i64 noundef %19)
  call void @exit(i32 noundef -1) #3
  unreachable

21:                                               ; preds = %15
  %22 = load i64, ptr @token_val, align 8
  store i64 %22, ptr %1, align 8
  call void @next()
  br label %23

23:                                               ; preds = %21, %12
  %24 = load ptr, ptr @current_id, align 8
  %25 = getelementptr inbounds i64, ptr %24, i64 4
  store i64 128, ptr %25, align 8
  %26 = load ptr, ptr @current_id, align 8
  %27 = getelementptr inbounds i64, ptr %26, i64 3
  store i64 1, ptr %27, align 8
  %28 = load i64, ptr %1, align 8
  %29 = add nsw i64 %28, 1
  store i64 %29, ptr %1, align 8
  %30 = load ptr, ptr @current_id, align 8
  %31 = getelementptr inbounds i64, ptr %30, i64 5
  store i64 %28, ptr %31, align 8
  %32 = load i64, ptr @token, align 8
  %33 = icmp eq i64 %32, 44
  br i1 %33, label %34, label %35

34:                                               ; preds = %23
  call void @next()
  br label %35

35:                                               ; preds = %34, %23
  br label %2, !llvm.loop !23

36:                                               ; preds = %2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @function_parameter() #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  store i64 0, ptr %2, align 8
  br label %3

3:                                                ; preds = %64, %0
  %4 = load i64, ptr @token, align 8
  %5 = icmp ne i64 %4, 41
  br i1 %5, label %6, label %65

6:                                                ; preds = %3
  store i64 1, ptr %1, align 8
  %7 = load i64, ptr @token, align 8
  %8 = icmp eq i64 %7, 138
  br i1 %8, label %9, label %10

9:                                                ; preds = %6
  call void @match(i64 noundef 138)
  br label %15

10:                                               ; preds = %6
  %11 = load i64, ptr @token, align 8
  %12 = icmp eq i64 %11, 134
  br i1 %12, label %13, label %14

13:                                               ; preds = %10
  store i64 0, ptr %1, align 8
  call void @match(i64 noundef 134)
  br label %14

14:                                               ; preds = %13, %10
  br label %15

15:                                               ; preds = %14, %9
  br label %16

16:                                               ; preds = %19, %15
  %17 = load i64, ptr @token, align 8
  %18 = icmp eq i64 %17, 159
  br i1 %18, label %19, label %22

19:                                               ; preds = %16
  call void @match(i64 noundef 159)
  %20 = load i64, ptr %1, align 8
  %21 = add nsw i64 %20, 2
  store i64 %21, ptr %1, align 8
  br label %16, !llvm.loop !24

22:                                               ; preds = %16
  %23 = load i64, ptr @token, align 8
  %24 = icmp ne i64 %23, 133
  br i1 %24, label %25, label %28

25:                                               ; preds = %22
  %26 = load i64, ptr @line, align 8
  %27 = call i32 (ptr, ...) @printf(ptr noundef @.str.20, i64 noundef %26)
  call void @exit(i32 noundef -1) #3
  unreachable

28:                                               ; preds = %22
  %29 = load ptr, ptr @current_id, align 8
  %30 = getelementptr inbounds i64, ptr %29, i64 4
  %31 = load i64, ptr %30, align 8
  %32 = icmp eq i64 %31, 132
  br i1 %32, label %33, label %36

33:                                               ; preds = %28
  %34 = load i64, ptr @line, align 8
  %35 = call i32 (ptr, ...) @printf(ptr noundef @.str.21, i64 noundef %34)
  call void @exit(i32 noundef -1) #3
  unreachable

36:                                               ; preds = %28
  call void @match(i64 noundef 133)
  %37 = load ptr, ptr @current_id, align 8
  %38 = getelementptr inbounds i64, ptr %37, i64 4
  %39 = load i64, ptr %38, align 8
  %40 = load ptr, ptr @current_id, align 8
  %41 = getelementptr inbounds i64, ptr %40, i64 7
  store i64 %39, ptr %41, align 8
  %42 = load ptr, ptr @current_id, align 8
  %43 = getelementptr inbounds i64, ptr %42, i64 4
  store i64 132, ptr %43, align 8
  %44 = load ptr, ptr @current_id, align 8
  %45 = getelementptr inbounds i64, ptr %44, i64 3
  %46 = load i64, ptr %45, align 8
  %47 = load ptr, ptr @current_id, align 8
  %48 = getelementptr inbounds i64, ptr %47, i64 6
  store i64 %46, ptr %48, align 8
  %49 = load i64, ptr %1, align 8
  %50 = load ptr, ptr @current_id, align 8
  %51 = getelementptr inbounds i64, ptr %50, i64 3
  store i64 %49, ptr %51, align 8
  %52 = load ptr, ptr @current_id, align 8
  %53 = getelementptr inbounds i64, ptr %52, i64 5
  %54 = load i64, ptr %53, align 8
  %55 = load ptr, ptr @current_id, align 8
  %56 = getelementptr inbounds i64, ptr %55, i64 8
  store i64 %54, ptr %56, align 8
  %57 = load i64, ptr %2, align 8
  %58 = add nsw i64 %57, 1
  store i64 %58, ptr %2, align 8
  %59 = load ptr, ptr @current_id, align 8
  %60 = getelementptr inbounds i64, ptr %59, i64 5
  store i64 %57, ptr %60, align 8
  %61 = load i64, ptr @token, align 8
  %62 = icmp eq i64 %61, 44
  br i1 %62, label %63, label %64

63:                                               ; preds = %36
  call void @match(i64 noundef 44)
  br label %64

64:                                               ; preds = %63, %36
  br label %3, !llvm.loop !25

65:                                               ; preds = %3
  %66 = load i64, ptr %2, align 8
  %67 = add nsw i64 %66, 1
  store i64 %67, ptr @index_of_bp, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @function_body() #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = load i64, ptr @index_of_bp, align 8
  store i64 %3, ptr %1, align 8
  br label %4

4:                                                ; preds = %73, %0
  %5 = load i64, ptr @token, align 8
  %6 = icmp eq i64 %5, 138
  br i1 %6, label %10, label %7

7:                                                ; preds = %4
  %8 = load i64, ptr @token, align 8
  %9 = icmp eq i64 %8, 134
  br label %10

10:                                               ; preds = %7, %4
  %11 = phi i1 [ true, %4 ], [ %9, %7 ]
  br i1 %11, label %12, label %74

12:                                               ; preds = %10
  %13 = load i64, ptr @token, align 8
  %14 = icmp eq i64 %13, 138
  %15 = zext i1 %14 to i64
  %16 = select i1 %14, i32 1, i32 0
  %17 = sext i32 %16 to i64
  store i64 %17, ptr @basetype, align 8
  %18 = load i64, ptr @token, align 8
  call void @match(i64 noundef %18)
  br label %19

19:                                               ; preds = %72, %12
  %20 = load i64, ptr @token, align 8
  %21 = icmp ne i64 %20, 59
  br i1 %21, label %22, label %73

22:                                               ; preds = %19
  %23 = load i64, ptr @basetype, align 8
  store i64 %23, ptr %2, align 8
  br label %24

24:                                               ; preds = %27, %22
  %25 = load i64, ptr @token, align 8
  %26 = icmp eq i64 %25, 159
  br i1 %26, label %27, label %30

27:                                               ; preds = %24
  call void @match(i64 noundef 159)
  %28 = load i64, ptr %2, align 8
  %29 = add nsw i64 %28, 2
  store i64 %29, ptr %2, align 8
  br label %24, !llvm.loop !26

30:                                               ; preds = %24
  %31 = load i64, ptr @token, align 8
  %32 = icmp ne i64 %31, 133
  br i1 %32, label %33, label %36

33:                                               ; preds = %30
  %34 = load i64, ptr @line, align 8
  %35 = call i32 (ptr, ...) @printf(ptr noundef @.str.22, i64 noundef %34)
  call void @exit(i32 noundef -1) #3
  unreachable

36:                                               ; preds = %30
  %37 = load ptr, ptr @current_id, align 8
  %38 = getelementptr inbounds i64, ptr %37, i64 4
  %39 = load i64, ptr %38, align 8
  %40 = icmp eq i64 %39, 132
  br i1 %40, label %41, label %44

41:                                               ; preds = %36
  %42 = load i64, ptr @line, align 8
  %43 = call i32 (ptr, ...) @printf(ptr noundef @.str.23, i64 noundef %42)
  call void @exit(i32 noundef -1) #3
  unreachable

44:                                               ; preds = %36
  call void @match(i64 noundef 133)
  %45 = load ptr, ptr @current_id, align 8
  %46 = getelementptr inbounds i64, ptr %45, i64 4
  %47 = load i64, ptr %46, align 8
  %48 = load ptr, ptr @current_id, align 8
  %49 = getelementptr inbounds i64, ptr %48, i64 7
  store i64 %47, ptr %49, align 8
  %50 = load ptr, ptr @current_id, align 8
  %51 = getelementptr inbounds i64, ptr %50, i64 4
  store i64 132, ptr %51, align 8
  %52 = load ptr, ptr @current_id, align 8
  %53 = getelementptr inbounds i64, ptr %52, i64 3
  %54 = load i64, ptr %53, align 8
  %55 = load ptr, ptr @current_id, align 8
  %56 = getelementptr inbounds i64, ptr %55, i64 6
  store i64 %54, ptr %56, align 8
  %57 = load i64, ptr %2, align 8
  %58 = load ptr, ptr @current_id, align 8
  %59 = getelementptr inbounds i64, ptr %58, i64 3
  store i64 %57, ptr %59, align 8
  %60 = load ptr, ptr @current_id, align 8
  %61 = getelementptr inbounds i64, ptr %60, i64 5
  %62 = load i64, ptr %61, align 8
  %63 = load ptr, ptr @current_id, align 8
  %64 = getelementptr inbounds i64, ptr %63, i64 8
  store i64 %62, ptr %64, align 8
  %65 = load i64, ptr %1, align 8
  %66 = add nsw i64 %65, 1
  store i64 %66, ptr %1, align 8
  %67 = load ptr, ptr @current_id, align 8
  %68 = getelementptr inbounds i64, ptr %67, i64 5
  store i64 %66, ptr %68, align 8
  %69 = load i64, ptr @token, align 8
  %70 = icmp eq i64 %69, 44
  br i1 %70, label %71, label %72

71:                                               ; preds = %44
  call void @match(i64 noundef 44)
  br label %72

72:                                               ; preds = %71, %44
  br label %19, !llvm.loop !27

73:                                               ; preds = %19
  call void @match(i64 noundef 59)
  br label %4, !llvm.loop !28

74:                                               ; preds = %10
  %75 = load ptr, ptr @text, align 8
  %76 = getelementptr inbounds i64, ptr %75, i32 1
  store ptr %76, ptr @text, align 8
  store i64 6, ptr %76, align 8
  %77 = load i64, ptr %1, align 8
  %78 = load i64, ptr @index_of_bp, align 8
  %79 = sub nsw i64 %77, %78
  %80 = load ptr, ptr @text, align 8
  %81 = getelementptr inbounds i64, ptr %80, i32 1
  store ptr %81, ptr @text, align 8
  store i64 %79, ptr %81, align 8
  br label %82

82:                                               ; preds = %85, %74
  %83 = load i64, ptr @token, align 8
  %84 = icmp ne i64 %83, 125
  br i1 %84, label %85, label %86

85:                                               ; preds = %82
  call void @statement()
  br label %82, !llvm.loop !29

86:                                               ; preds = %82
  %87 = load ptr, ptr @text, align 8
  %88 = getelementptr inbounds i64, ptr %87, i32 1
  store ptr %88, ptr @text, align 8
  store i64 8, ptr %88, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @function_declaration() #0 {
  call void @match(i64 noundef 40)
  call void @function_parameter()
  call void @match(i64 noundef 41)
  call void @match(i64 noundef 123)
  call void @function_body()
  %1 = load ptr, ptr @symbols, align 8
  store ptr %1, ptr @current_id, align 8
  br label %2

2:                                                ; preds = %28, %0
  %3 = load ptr, ptr @current_id, align 8
  %4 = getelementptr inbounds i64, ptr %3, i64 0
  %5 = load i64, ptr %4, align 8
  %6 = icmp ne i64 %5, 0
  br i1 %6, label %7, label %31

7:                                                ; preds = %2
  %8 = load ptr, ptr @current_id, align 8
  %9 = getelementptr inbounds i64, ptr %8, i64 4
  %10 = load i64, ptr %9, align 8
  %11 = icmp eq i64 %10, 132
  br i1 %11, label %12, label %28

12:                                               ; preds = %7
  %13 = load ptr, ptr @current_id, align 8
  %14 = getelementptr inbounds i64, ptr %13, i64 7
  %15 = load i64, ptr %14, align 8
  %16 = load ptr, ptr @current_id, align 8
  %17 = getelementptr inbounds i64, ptr %16, i64 4
  store i64 %15, ptr %17, align 8
  %18 = load ptr, ptr @current_id, align 8
  %19 = getelementptr inbounds i64, ptr %18, i64 6
  %20 = load i64, ptr %19, align 8
  %21 = load ptr, ptr @current_id, align 8
  %22 = getelementptr inbounds i64, ptr %21, i64 3
  store i64 %20, ptr %22, align 8
  %23 = load ptr, ptr @current_id, align 8
  %24 = getelementptr inbounds i64, ptr %23, i64 8
  %25 = load i64, ptr %24, align 8
  %26 = load ptr, ptr @current_id, align 8
  %27 = getelementptr inbounds i64, ptr %26, i64 5
  store i64 %25, ptr %27, align 8
  br label %28

28:                                               ; preds = %12, %7
  %29 = load ptr, ptr @current_id, align 8
  %30 = getelementptr inbounds i64, ptr %29, i64 9
  store ptr %30, ptr @current_id, align 8
  br label %2, !llvm.loop !30

31:                                               ; preds = %2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @global_declaration() #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  store i64 1, ptr @basetype, align 8
  %3 = load i64, ptr @token, align 8
  %4 = icmp eq i64 %3, 136
  br i1 %4, label %5, label %14

5:                                                ; preds = %0
  call void @match(i64 noundef 136)
  %6 = load i64, ptr @token, align 8
  %7 = icmp ne i64 %6, 123
  br i1 %7, label %8, label %9

8:                                                ; preds = %5
  call void @match(i64 noundef 133)
  br label %9

9:                                                ; preds = %8, %5
  %10 = load i64, ptr @token, align 8
  %11 = icmp eq i64 %10, 123
  br i1 %11, label %12, label %13

12:                                               ; preds = %9
  call void @match(i64 noundef 123)
  call void @enum_declaration()
  call void @match(i64 noundef 125)
  br label %13

13:                                               ; preds = %12, %9
  call void @match(i64 noundef 59)
  br label %83

14:                                               ; preds = %0
  %15 = load i64, ptr @token, align 8
  %16 = icmp eq i64 %15, 138
  br i1 %16, label %17, label %18

17:                                               ; preds = %14
  call void @match(i64 noundef 138)
  br label %23

18:                                               ; preds = %14
  %19 = load i64, ptr @token, align 8
  %20 = icmp eq i64 %19, 134
  br i1 %20, label %21, label %22

21:                                               ; preds = %18
  call void @match(i64 noundef 134)
  store i64 0, ptr @basetype, align 8
  br label %22

22:                                               ; preds = %21, %18
  br label %23

23:                                               ; preds = %22, %17
  br label %24

24:                                               ; preds = %81, %23
  %25 = load i64, ptr @token, align 8
  %26 = icmp ne i64 %25, 59
  br i1 %26, label %27, label %30

27:                                               ; preds = %24
  %28 = load i64, ptr @token, align 8
  %29 = icmp ne i64 %28, 125
  br label %30

30:                                               ; preds = %27, %24
  %31 = phi i1 [ false, %24 ], [ %29, %27 ]
  br i1 %31, label %32, label %82

32:                                               ; preds = %30
  %33 = load i64, ptr @basetype, align 8
  store i64 %33, ptr %1, align 8
  br label %34

34:                                               ; preds = %37, %32
  %35 = load i64, ptr @token, align 8
  %36 = icmp eq i64 %35, 159
  br i1 %36, label %37, label %40

37:                                               ; preds = %34
  call void @match(i64 noundef 159)
  %38 = load i64, ptr %1, align 8
  %39 = add nsw i64 %38, 2
  store i64 %39, ptr %1, align 8
  br label %34, !llvm.loop !31

40:                                               ; preds = %34
  %41 = load i64, ptr @token, align 8
  %42 = icmp ne i64 %41, 133
  br i1 %42, label %43, label %46

43:                                               ; preds = %40
  %44 = load i64, ptr @line, align 8
  %45 = call i32 (ptr, ...) @printf(ptr noundef @.str.24, i64 noundef %44)
  call void @exit(i32 noundef -1) #3
  unreachable

46:                                               ; preds = %40
  %47 = load ptr, ptr @current_id, align 8
  %48 = getelementptr inbounds i64, ptr %47, i64 4
  %49 = load i64, ptr %48, align 8
  %50 = icmp ne i64 %49, 0
  br i1 %50, label %51, label %54

51:                                               ; preds = %46
  %52 = load i64, ptr @line, align 8
  %53 = call i32 (ptr, ...) @printf(ptr noundef @.str.25, i64 noundef %52)
  call void @exit(i32 noundef -1) #3
  unreachable

54:                                               ; preds = %46
  call void @match(i64 noundef 133)
  %55 = load i64, ptr %1, align 8
  %56 = load ptr, ptr @current_id, align 8
  %57 = getelementptr inbounds i64, ptr %56, i64 3
  store i64 %55, ptr %57, align 8
  %58 = load i64, ptr @token, align 8
  %59 = icmp eq i64 %58, 40
  br i1 %59, label %60, label %68

60:                                               ; preds = %54
  %61 = load ptr, ptr @current_id, align 8
  %62 = getelementptr inbounds i64, ptr %61, i64 4
  store i64 129, ptr %62, align 8
  %63 = load ptr, ptr @text, align 8
  %64 = getelementptr inbounds i64, ptr %63, i64 1
  %65 = ptrtoint ptr %64 to i64
  %66 = load ptr, ptr @current_id, align 8
  %67 = getelementptr inbounds i64, ptr %66, i64 5
  store i64 %65, ptr %67, align 8
  call void @function_declaration()
  br label %77

68:                                               ; preds = %54
  %69 = load ptr, ptr @current_id, align 8
  %70 = getelementptr inbounds i64, ptr %69, i64 4
  store i64 131, ptr %70, align 8
  %71 = load ptr, ptr @data, align 8
  %72 = ptrtoint ptr %71 to i64
  %73 = load ptr, ptr @current_id, align 8
  %74 = getelementptr inbounds i64, ptr %73, i64 5
  store i64 %72, ptr %74, align 8
  %75 = load ptr, ptr @data, align 8
  %76 = getelementptr inbounds i8, ptr %75, i64 8
  store ptr %76, ptr @data, align 8
  br label %77

77:                                               ; preds = %68, %60
  %78 = load i64, ptr @token, align 8
  %79 = icmp eq i64 %78, 44
  br i1 %79, label %80, label %81

80:                                               ; preds = %77
  call void @match(i64 noundef 44)
  br label %81

81:                                               ; preds = %80, %77
  br label %24, !llvm.loop !32

82:                                               ; preds = %30
  call void @next()
  br label %83

83:                                               ; preds = %82, %13
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @program() #0 {
  call void @next()
  br label %1

1:                                                ; preds = %4, %0
  %2 = load i64, ptr @token, align 8
  %3 = icmp sgt i64 %2, 0
  br i1 %3, label %4, label %5

4:                                                ; preds = %1
  call void @global_declaration()
  br label %1, !llvm.loop !33

5:                                                ; preds = %1
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @eval() #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = alloca ptr, align 8
  store i64 0, ptr @cycle, align 8
  br label %4

4:                                                ; preds = %0, %485
  %5 = load i64, ptr @cycle, align 8
  %6 = add nsw i64 %5, 1
  store i64 %6, ptr @cycle, align 8
  %7 = load ptr, ptr @pc, align 8
  %8 = getelementptr inbounds i64, ptr %7, i32 1
  store ptr %8, ptr @pc, align 8
  %9 = load i64, ptr %7, align 8
  store i64 %9, ptr %2, align 8
  %10 = load i64, ptr @debug, align 8
  %11 = icmp ne i64 %10, 0
  br i1 %11, label %12, label %27

12:                                               ; preds = %4
  %13 = load i64, ptr @cycle, align 8
  %14 = load i64, ptr %2, align 8
  %15 = mul nsw i64 %14, 5
  %16 = getelementptr inbounds [190 x i8], ptr @.str.2, i64 0, i64 %15
  %17 = call i32 (ptr, ...) @printf(ptr noundef @.str.26, i64 noundef %13, ptr noundef %16)
  %18 = load i64, ptr %2, align 8
  %19 = icmp sle i64 %18, 7
  br i1 %19, label %20, label %24

20:                                               ; preds = %12
  %21 = load ptr, ptr @pc, align 8
  %22 = load i64, ptr %21, align 8
  %23 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i64 noundef %22)
  br label %26

24:                                               ; preds = %12
  %25 = call i32 (ptr, ...) @printf(ptr noundef @.str.4)
  br label %26

26:                                               ; preds = %24, %20
  br label %27

27:                                               ; preds = %26, %4
  %28 = load i64, ptr %2, align 8
  %29 = icmp eq i64 %28, 1
  br i1 %29, label %30, label %34

30:                                               ; preds = %27
  %31 = load ptr, ptr @pc, align 8
  %32 = getelementptr inbounds i64, ptr %31, i32 1
  store ptr %32, ptr @pc, align 8
  %33 = load i64, ptr %31, align 8
  store i64 %33, ptr @ax, align 8
  br label %485

34:                                               ; preds = %27
  %35 = load i64, ptr %2, align 8
  %36 = icmp eq i64 %35, 10
  br i1 %36, label %37, label %42

37:                                               ; preds = %34
  %38 = load i64, ptr @ax, align 8
  %39 = inttoptr i64 %38 to ptr
  %40 = load i8, ptr %39, align 1
  %41 = sext i8 %40 to i64
  store i64 %41, ptr @ax, align 8
  br label %484

42:                                               ; preds = %34
  %43 = load i64, ptr %2, align 8
  %44 = icmp eq i64 %43, 9
  br i1 %44, label %45, label %49

45:                                               ; preds = %42
  %46 = load i64, ptr @ax, align 8
  %47 = inttoptr i64 %46 to ptr
  %48 = load i64, ptr %47, align 8
  store i64 %48, ptr @ax, align 8
  br label %483

49:                                               ; preds = %42
  %50 = load i64, ptr %2, align 8
  %51 = icmp eq i64 %50, 12
  br i1 %51, label %52, label %60

52:                                               ; preds = %49
  %53 = load i64, ptr @ax, align 8
  %54 = trunc i64 %53 to i8
  %55 = load ptr, ptr @sp, align 8
  %56 = getelementptr inbounds i64, ptr %55, i32 1
  store ptr %56, ptr @sp, align 8
  %57 = load i64, ptr %55, align 8
  %58 = inttoptr i64 %57 to ptr
  store i8 %54, ptr %58, align 1
  %59 = sext i8 %54 to i64
  store i64 %59, ptr @ax, align 8
  br label %482

60:                                               ; preds = %49
  %61 = load i64, ptr %2, align 8
  %62 = icmp eq i64 %61, 11
  br i1 %62, label %63, label %69

63:                                               ; preds = %60
  %64 = load i64, ptr @ax, align 8
  %65 = load ptr, ptr @sp, align 8
  %66 = getelementptr inbounds i64, ptr %65, i32 1
  store ptr %66, ptr @sp, align 8
  %67 = load i64, ptr %65, align 8
  %68 = inttoptr i64 %67 to ptr
  store i64 %64, ptr %68, align 8
  br label %481

69:                                               ; preds = %60
  %70 = load i64, ptr %2, align 8
  %71 = icmp eq i64 %70, 13
  br i1 %71, label %72, label %76

72:                                               ; preds = %69
  %73 = load i64, ptr @ax, align 8
  %74 = load ptr, ptr @sp, align 8
  %75 = getelementptr inbounds i64, ptr %74, i32 -1
  store ptr %75, ptr @sp, align 8
  store i64 %73, ptr %75, align 8
  br label %480

76:                                               ; preds = %69
  %77 = load i64, ptr %2, align 8
  %78 = icmp eq i64 %77, 2
  br i1 %78, label %79, label %83

79:                                               ; preds = %76
  %80 = load ptr, ptr @pc, align 8
  %81 = load i64, ptr %80, align 8
  %82 = inttoptr i64 %81 to ptr
  store ptr %82, ptr @pc, align 8
  br label %479

83:                                               ; preds = %76
  %84 = load i64, ptr %2, align 8
  %85 = icmp eq i64 %84, 4
  br i1 %85, label %86, label %98

86:                                               ; preds = %83
  %87 = load i64, ptr @ax, align 8
  %88 = icmp ne i64 %87, 0
  br i1 %88, label %89, label %92

89:                                               ; preds = %86
  %90 = load ptr, ptr @pc, align 8
  %91 = getelementptr inbounds i64, ptr %90, i64 1
  br label %96

92:                                               ; preds = %86
  %93 = load ptr, ptr @pc, align 8
  %94 = load i64, ptr %93, align 8
  %95 = inttoptr i64 %94 to ptr
  br label %96

96:                                               ; preds = %92, %89
  %97 = phi ptr [ %91, %89 ], [ %95, %92 ]
  store ptr %97, ptr @pc, align 8
  br label %478

98:                                               ; preds = %83
  %99 = load i64, ptr %2, align 8
  %100 = icmp eq i64 %99, 5
  br i1 %100, label %101, label %113

101:                                              ; preds = %98
  %102 = load i64, ptr @ax, align 8
  %103 = icmp ne i64 %102, 0
  br i1 %103, label %104, label %108

104:                                              ; preds = %101
  %105 = load ptr, ptr @pc, align 8
  %106 = load i64, ptr %105, align 8
  %107 = inttoptr i64 %106 to ptr
  br label %111

108:                                              ; preds = %101
  %109 = load ptr, ptr @pc, align 8
  %110 = getelementptr inbounds i64, ptr %109, i64 1
  br label %111

111:                                              ; preds = %108, %104
  %112 = phi ptr [ %107, %104 ], [ %110, %108 ]
  store ptr %112, ptr @pc, align 8
  br label %477

113:                                              ; preds = %98
  %114 = load i64, ptr %2, align 8
  %115 = icmp eq i64 %114, 3
  br i1 %115, label %116, label %125

116:                                              ; preds = %113
  %117 = load ptr, ptr @pc, align 8
  %118 = getelementptr inbounds i64, ptr %117, i64 1
  %119 = ptrtoint ptr %118 to i64
  %120 = load ptr, ptr @sp, align 8
  %121 = getelementptr inbounds i64, ptr %120, i32 -1
  store ptr %121, ptr @sp, align 8
  store i64 %119, ptr %121, align 8
  %122 = load ptr, ptr @pc, align 8
  %123 = load i64, ptr %122, align 8
  %124 = inttoptr i64 %123 to ptr
  store ptr %124, ptr @pc, align 8
  br label %476

125:                                              ; preds = %113
  %126 = load i64, ptr %2, align 8
  %127 = icmp eq i64 %126, 6
  br i1 %127, label %128, label %140

128:                                              ; preds = %125
  %129 = load ptr, ptr @bp, align 8
  %130 = ptrtoint ptr %129 to i64
  %131 = load ptr, ptr @sp, align 8
  %132 = getelementptr inbounds i64, ptr %131, i32 -1
  store ptr %132, ptr @sp, align 8
  store i64 %130, ptr %132, align 8
  %133 = load ptr, ptr @sp, align 8
  store ptr %133, ptr @bp, align 8
  %134 = load ptr, ptr @sp, align 8
  %135 = load ptr, ptr @pc, align 8
  %136 = getelementptr inbounds i64, ptr %135, i32 1
  store ptr %136, ptr @pc, align 8
  %137 = load i64, ptr %135, align 8
  %138 = sub i64 0, %137
  %139 = getelementptr inbounds i64, ptr %134, i64 %138
  store ptr %139, ptr @sp, align 8
  br label %475

140:                                              ; preds = %125
  %141 = load i64, ptr %2, align 8
  %142 = icmp eq i64 %141, 7
  br i1 %142, label %143, label %149

143:                                              ; preds = %140
  %144 = load ptr, ptr @sp, align 8
  %145 = load ptr, ptr @pc, align 8
  %146 = getelementptr inbounds i64, ptr %145, i32 1
  store ptr %146, ptr @pc, align 8
  %147 = load i64, ptr %145, align 8
  %148 = getelementptr inbounds i64, ptr %144, i64 %147
  store ptr %148, ptr @sp, align 8
  br label %474

149:                                              ; preds = %140
  %150 = load i64, ptr %2, align 8
  %151 = icmp eq i64 %150, 8
  br i1 %151, label %152, label %162

152:                                              ; preds = %149
  %153 = load ptr, ptr @bp, align 8
  store ptr %153, ptr @sp, align 8
  %154 = load ptr, ptr @sp, align 8
  %155 = getelementptr inbounds i64, ptr %154, i32 1
  store ptr %155, ptr @sp, align 8
  %156 = load i64, ptr %154, align 8
  %157 = inttoptr i64 %156 to ptr
  store ptr %157, ptr @bp, align 8
  %158 = load ptr, ptr @sp, align 8
  %159 = getelementptr inbounds i64, ptr %158, i32 1
  store ptr %159, ptr @sp, align 8
  %160 = load i64, ptr %158, align 8
  %161 = inttoptr i64 %160 to ptr
  store ptr %161, ptr @pc, align 8
  br label %473

162:                                              ; preds = %149
  %163 = load i64, ptr %2, align 8
  %164 = icmp eq i64 %163, 0
  br i1 %164, label %165, label %172

165:                                              ; preds = %162
  %166 = load ptr, ptr @bp, align 8
  %167 = load ptr, ptr @pc, align 8
  %168 = getelementptr inbounds i64, ptr %167, i32 1
  store ptr %168, ptr @pc, align 8
  %169 = load i64, ptr %167, align 8
  %170 = getelementptr inbounds i64, ptr %166, i64 %169
  %171 = ptrtoint ptr %170 to i64
  store i64 %171, ptr @ax, align 8
  br label %472

172:                                              ; preds = %162
  %173 = load i64, ptr %2, align 8
  %174 = icmp eq i64 %173, 14
  br i1 %174, label %175, label %181

175:                                              ; preds = %172
  %176 = load ptr, ptr @sp, align 8
  %177 = getelementptr inbounds i64, ptr %176, i32 1
  store ptr %177, ptr @sp, align 8
  %178 = load i64, ptr %176, align 8
  %179 = load i64, ptr @ax, align 8
  %180 = or i64 %178, %179
  store i64 %180, ptr @ax, align 8
  br label %471

181:                                              ; preds = %172
  %182 = load i64, ptr %2, align 8
  %183 = icmp eq i64 %182, 15
  br i1 %183, label %184, label %190

184:                                              ; preds = %181
  %185 = load ptr, ptr @sp, align 8
  %186 = getelementptr inbounds i64, ptr %185, i32 1
  store ptr %186, ptr @sp, align 8
  %187 = load i64, ptr %185, align 8
  %188 = load i64, ptr @ax, align 8
  %189 = xor i64 %187, %188
  store i64 %189, ptr @ax, align 8
  br label %470

190:                                              ; preds = %181
  %191 = load i64, ptr %2, align 8
  %192 = icmp eq i64 %191, 16
  br i1 %192, label %193, label %199

193:                                              ; preds = %190
  %194 = load ptr, ptr @sp, align 8
  %195 = getelementptr inbounds i64, ptr %194, i32 1
  store ptr %195, ptr @sp, align 8
  %196 = load i64, ptr %194, align 8
  %197 = load i64, ptr @ax, align 8
  %198 = and i64 %196, %197
  store i64 %198, ptr @ax, align 8
  br label %469

199:                                              ; preds = %190
  %200 = load i64, ptr %2, align 8
  %201 = icmp eq i64 %200, 17
  br i1 %201, label %202, label %210

202:                                              ; preds = %199
  %203 = load ptr, ptr @sp, align 8
  %204 = getelementptr inbounds i64, ptr %203, i32 1
  store ptr %204, ptr @sp, align 8
  %205 = load i64, ptr %203, align 8
  %206 = load i64, ptr @ax, align 8
  %207 = icmp eq i64 %205, %206
  %208 = zext i1 %207 to i32
  %209 = sext i32 %208 to i64
  store i64 %209, ptr @ax, align 8
  br label %468

210:                                              ; preds = %199
  %211 = load i64, ptr %2, align 8
  %212 = icmp eq i64 %211, 18
  br i1 %212, label %213, label %221

213:                                              ; preds = %210
  %214 = load ptr, ptr @sp, align 8
  %215 = getelementptr inbounds i64, ptr %214, i32 1
  store ptr %215, ptr @sp, align 8
  %216 = load i64, ptr %214, align 8
  %217 = load i64, ptr @ax, align 8
  %218 = icmp ne i64 %216, %217
  %219 = zext i1 %218 to i32
  %220 = sext i32 %219 to i64
  store i64 %220, ptr @ax, align 8
  br label %467

221:                                              ; preds = %210
  %222 = load i64, ptr %2, align 8
  %223 = icmp eq i64 %222, 19
  br i1 %223, label %224, label %232

224:                                              ; preds = %221
  %225 = load ptr, ptr @sp, align 8
  %226 = getelementptr inbounds i64, ptr %225, i32 1
  store ptr %226, ptr @sp, align 8
  %227 = load i64, ptr %225, align 8
  %228 = load i64, ptr @ax, align 8
  %229 = icmp slt i64 %227, %228
  %230 = zext i1 %229 to i32
  %231 = sext i32 %230 to i64
  store i64 %231, ptr @ax, align 8
  br label %466

232:                                              ; preds = %221
  %233 = load i64, ptr %2, align 8
  %234 = icmp eq i64 %233, 21
  br i1 %234, label %235, label %243

235:                                              ; preds = %232
  %236 = load ptr, ptr @sp, align 8
  %237 = getelementptr inbounds i64, ptr %236, i32 1
  store ptr %237, ptr @sp, align 8
  %238 = load i64, ptr %236, align 8
  %239 = load i64, ptr @ax, align 8
  %240 = icmp sle i64 %238, %239
  %241 = zext i1 %240 to i32
  %242 = sext i32 %241 to i64
  store i64 %242, ptr @ax, align 8
  br label %465

243:                                              ; preds = %232
  %244 = load i64, ptr %2, align 8
  %245 = icmp eq i64 %244, 20
  br i1 %245, label %246, label %254

246:                                              ; preds = %243
  %247 = load ptr, ptr @sp, align 8
  %248 = getelementptr inbounds i64, ptr %247, i32 1
  store ptr %248, ptr @sp, align 8
  %249 = load i64, ptr %247, align 8
  %250 = load i64, ptr @ax, align 8
  %251 = icmp sgt i64 %249, %250
  %252 = zext i1 %251 to i32
  %253 = sext i32 %252 to i64
  store i64 %253, ptr @ax, align 8
  br label %464

254:                                              ; preds = %243
  %255 = load i64, ptr %2, align 8
  %256 = icmp eq i64 %255, 22
  br i1 %256, label %257, label %265

257:                                              ; preds = %254
  %258 = load ptr, ptr @sp, align 8
  %259 = getelementptr inbounds i64, ptr %258, i32 1
  store ptr %259, ptr @sp, align 8
  %260 = load i64, ptr %258, align 8
  %261 = load i64, ptr @ax, align 8
  %262 = icmp sge i64 %260, %261
  %263 = zext i1 %262 to i32
  %264 = sext i32 %263 to i64
  store i64 %264, ptr @ax, align 8
  br label %463

265:                                              ; preds = %254
  %266 = load i64, ptr %2, align 8
  %267 = icmp eq i64 %266, 23
  br i1 %267, label %268, label %274

268:                                              ; preds = %265
  %269 = load ptr, ptr @sp, align 8
  %270 = getelementptr inbounds i64, ptr %269, i32 1
  store ptr %270, ptr @sp, align 8
  %271 = load i64, ptr %269, align 8
  %272 = load i64, ptr @ax, align 8
  %273 = shl i64 %271, %272
  store i64 %273, ptr @ax, align 8
  br label %462

274:                                              ; preds = %265
  %275 = load i64, ptr %2, align 8
  %276 = icmp eq i64 %275, 24
  br i1 %276, label %277, label %283

277:                                              ; preds = %274
  %278 = load ptr, ptr @sp, align 8
  %279 = getelementptr inbounds i64, ptr %278, i32 1
  store ptr %279, ptr @sp, align 8
  %280 = load i64, ptr %278, align 8
  %281 = load i64, ptr @ax, align 8
  %282 = ashr i64 %280, %281
  store i64 %282, ptr @ax, align 8
  br label %461

283:                                              ; preds = %274
  %284 = load i64, ptr %2, align 8
  %285 = icmp eq i64 %284, 25
  br i1 %285, label %286, label %292

286:                                              ; preds = %283
  %287 = load ptr, ptr @sp, align 8
  %288 = getelementptr inbounds i64, ptr %287, i32 1
  store ptr %288, ptr @sp, align 8
  %289 = load i64, ptr %287, align 8
  %290 = load i64, ptr @ax, align 8
  %291 = add nsw i64 %289, %290
  store i64 %291, ptr @ax, align 8
  br label %460

292:                                              ; preds = %283
  %293 = load i64, ptr %2, align 8
  %294 = icmp eq i64 %293, 26
  br i1 %294, label %295, label %301

295:                                              ; preds = %292
  %296 = load ptr, ptr @sp, align 8
  %297 = getelementptr inbounds i64, ptr %296, i32 1
  store ptr %297, ptr @sp, align 8
  %298 = load i64, ptr %296, align 8
  %299 = load i64, ptr @ax, align 8
  %300 = sub nsw i64 %298, %299
  store i64 %300, ptr @ax, align 8
  br label %459

301:                                              ; preds = %292
  %302 = load i64, ptr %2, align 8
  %303 = icmp eq i64 %302, 27
  br i1 %303, label %304, label %310

304:                                              ; preds = %301
  %305 = load ptr, ptr @sp, align 8
  %306 = getelementptr inbounds i64, ptr %305, i32 1
  store ptr %306, ptr @sp, align 8
  %307 = load i64, ptr %305, align 8
  %308 = load i64, ptr @ax, align 8
  %309 = mul nsw i64 %307, %308
  store i64 %309, ptr @ax, align 8
  br label %458

310:                                              ; preds = %301
  %311 = load i64, ptr %2, align 8
  %312 = icmp eq i64 %311, 28
  br i1 %312, label %313, label %319

313:                                              ; preds = %310
  %314 = load ptr, ptr @sp, align 8
  %315 = getelementptr inbounds i64, ptr %314, i32 1
  store ptr %315, ptr @sp, align 8
  %316 = load i64, ptr %314, align 8
  %317 = load i64, ptr @ax, align 8
  %318 = sdiv i64 %316, %317
  store i64 %318, ptr @ax, align 8
  br label %457

319:                                              ; preds = %310
  %320 = load i64, ptr %2, align 8
  %321 = icmp eq i64 %320, 29
  br i1 %321, label %322, label %328

322:                                              ; preds = %319
  %323 = load ptr, ptr @sp, align 8
  %324 = getelementptr inbounds i64, ptr %323, i32 1
  store ptr %324, ptr @sp, align 8
  %325 = load i64, ptr %323, align 8
  %326 = load i64, ptr @ax, align 8
  %327 = srem i64 %325, %326
  store i64 %327, ptr @ax, align 8
  br label %456

328:                                              ; preds = %319
  %329 = load i64, ptr %2, align 8
  %330 = icmp eq i64 %329, 37
  br i1 %330, label %331, label %337

331:                                              ; preds = %328
  %332 = load ptr, ptr @sp, align 8
  %333 = load i64, ptr %332, align 8
  %334 = call i32 (ptr, ...) @printf(ptr noundef @.str.27, i64 noundef %333)
  %335 = load ptr, ptr @sp, align 8
  %336 = load i64, ptr %335, align 8
  store i64 %336, ptr %1, align 8
  br label %486

337:                                              ; preds = %328
  %338 = load i64, ptr %2, align 8
  %339 = icmp eq i64 %338, 30
  br i1 %339, label %340, label %351

340:                                              ; preds = %337
  %341 = load ptr, ptr @sp, align 8
  %342 = getelementptr inbounds i64, ptr %341, i64 1
  %343 = load i64, ptr %342, align 8
  %344 = inttoptr i64 %343 to ptr
  %345 = load ptr, ptr @sp, align 8
  %346 = getelementptr inbounds i64, ptr %345, i64 0
  %347 = load i64, ptr %346, align 8
  %348 = trunc i64 %347 to i32
  %349 = call i32 (ptr, i32, ...) @open(ptr noundef %344, i32 noundef %348)
  %350 = sext i32 %349 to i64
  store i64 %350, ptr @ax, align 8
  br label %454

351:                                              ; preds = %337
  %352 = load i64, ptr %2, align 8
  %353 = icmp eq i64 %352, 32
  br i1 %353, label %354, label %360

354:                                              ; preds = %351
  %355 = load ptr, ptr @sp, align 8
  %356 = load i64, ptr %355, align 8
  %357 = trunc i64 %356 to i32
  %358 = call i32 @close(i32 noundef %357)
  %359 = sext i32 %358 to i64
  store i64 %359, ptr @ax, align 8
  br label %453

360:                                              ; preds = %351
  %361 = load i64, ptr %2, align 8
  %362 = icmp eq i64 %361, 31
  br i1 %362, label %363, label %375

363:                                              ; preds = %360
  %364 = load ptr, ptr @sp, align 8
  %365 = getelementptr inbounds i64, ptr %364, i64 2
  %366 = load i64, ptr %365, align 8
  %367 = trunc i64 %366 to i32
  %368 = load ptr, ptr @sp, align 8
  %369 = getelementptr inbounds i64, ptr %368, i64 1
  %370 = load i64, ptr %369, align 8
  %371 = inttoptr i64 %370 to ptr
  %372 = load ptr, ptr @sp, align 8
  %373 = load i64, ptr %372, align 8
  %374 = call i64 @read(i32 noundef %367, ptr noundef %371, i64 noundef %373)
  store i64 %374, ptr @ax, align 8
  br label %452

375:                                              ; preds = %360
  %376 = load i64, ptr %2, align 8
  %377 = icmp eq i64 %376, 33
  br i1 %377, label %378, label %405

378:                                              ; preds = %375
  %379 = load ptr, ptr @sp, align 8
  %380 = load ptr, ptr @pc, align 8
  %381 = getelementptr inbounds i64, ptr %380, i64 1
  %382 = load i64, ptr %381, align 8
  %383 = getelementptr inbounds i64, ptr %379, i64 %382
  store ptr %383, ptr %3, align 8
  %384 = load ptr, ptr %3, align 8
  %385 = getelementptr inbounds i64, ptr %384, i64 -1
  %386 = load i64, ptr %385, align 8
  %387 = inttoptr i64 %386 to ptr
  %388 = load ptr, ptr %3, align 8
  %389 = getelementptr inbounds i64, ptr %388, i64 -2
  %390 = load i64, ptr %389, align 8
  %391 = load ptr, ptr %3, align 8
  %392 = getelementptr inbounds i64, ptr %391, i64 -3
  %393 = load i64, ptr %392, align 8
  %394 = load ptr, ptr %3, align 8
  %395 = getelementptr inbounds i64, ptr %394, i64 -4
  %396 = load i64, ptr %395, align 8
  %397 = load ptr, ptr %3, align 8
  %398 = getelementptr inbounds i64, ptr %397, i64 -5
  %399 = load i64, ptr %398, align 8
  %400 = load ptr, ptr %3, align 8
  %401 = getelementptr inbounds i64, ptr %400, i64 -6
  %402 = load i64, ptr %401, align 8
  %403 = call i32 (ptr, ...) @printf(ptr noundef %387, i64 noundef %390, i64 noundef %393, i64 noundef %396, i64 noundef %399, i64 noundef %402)
  %404 = sext i32 %403 to i64
  store i64 %404, ptr @ax, align 8
  br label %451

405:                                              ; preds = %375
  %406 = load i64, ptr %2, align 8
  %407 = icmp eq i64 %406, 34
  br i1 %407, label %408, label %413

408:                                              ; preds = %405
  %409 = load ptr, ptr @sp, align 8
  %410 = load i64, ptr %409, align 8
  %411 = call ptr @malloc(i64 noundef %410)
  %412 = ptrtoint ptr %411 to i64
  store i64 %412, ptr @ax, align 8
  br label %450

413:                                              ; preds = %405
  %414 = load i64, ptr %2, align 8
  %415 = icmp eq i64 %414, 35
  br i1 %415, label %416, label %429

416:                                              ; preds = %413
  %417 = load ptr, ptr @sp, align 8
  %418 = getelementptr inbounds i64, ptr %417, i64 2
  %419 = load i64, ptr %418, align 8
  %420 = inttoptr i64 %419 to ptr
  %421 = load ptr, ptr @sp, align 8
  %422 = getelementptr inbounds i64, ptr %421, i64 1
  %423 = load i64, ptr %422, align 8
  %424 = trunc i64 %423 to i32
  %425 = load ptr, ptr @sp, align 8
  %426 = load i64, ptr %425, align 8
  %427 = call ptr @memset(ptr noundef %420, i32 noundef %424, i64 noundef %426)
  %428 = ptrtoint ptr %427 to i64
  store i64 %428, ptr @ax, align 8
  br label %449

429:                                              ; preds = %413
  %430 = load i64, ptr %2, align 8
  %431 = icmp eq i64 %430, 36
  br i1 %431, label %432, label %445

432:                                              ; preds = %429
  %433 = load ptr, ptr @sp, align 8
  %434 = getelementptr inbounds i64, ptr %433, i64 2
  %435 = load i64, ptr %434, align 8
  %436 = inttoptr i64 %435 to ptr
  %437 = load ptr, ptr @sp, align 8
  %438 = getelementptr inbounds i64, ptr %437, i64 1
  %439 = load i64, ptr %438, align 8
  %440 = inttoptr i64 %439 to ptr
  %441 = load ptr, ptr @sp, align 8
  %442 = load i64, ptr %441, align 8
  %443 = call i32 @memcmp(ptr noundef %436, ptr noundef %440, i64 noundef %442)
  %444 = sext i32 %443 to i64
  store i64 %444, ptr @ax, align 8
  br label %448

445:                                              ; preds = %429
  %446 = load i64, ptr %2, align 8
  %447 = call i32 (ptr, ...) @printf(ptr noundef @.str.28, i64 noundef %446)
  store i64 -1, ptr %1, align 8
  br label %486

448:                                              ; preds = %432
  br label %449

449:                                              ; preds = %448, %416
  br label %450

450:                                              ; preds = %449, %408
  br label %451

451:                                              ; preds = %450, %378
  br label %452

452:                                              ; preds = %451, %363
  br label %453

453:                                              ; preds = %452, %354
  br label %454

454:                                              ; preds = %453, %340
  br label %455

455:                                              ; preds = %454
  br label %456

456:                                              ; preds = %455, %322
  br label %457

457:                                              ; preds = %456, %313
  br label %458

458:                                              ; preds = %457, %304
  br label %459

459:                                              ; preds = %458, %295
  br label %460

460:                                              ; preds = %459, %286
  br label %461

461:                                              ; preds = %460, %277
  br label %462

462:                                              ; preds = %461, %268
  br label %463

463:                                              ; preds = %462, %257
  br label %464

464:                                              ; preds = %463, %246
  br label %465

465:                                              ; preds = %464, %235
  br label %466

466:                                              ; preds = %465, %224
  br label %467

467:                                              ; preds = %466, %213
  br label %468

468:                                              ; preds = %467, %202
  br label %469

469:                                              ; preds = %468, %193
  br label %470

470:                                              ; preds = %469, %184
  br label %471

471:                                              ; preds = %470, %175
  br label %472

472:                                              ; preds = %471, %165
  br label %473

473:                                              ; preds = %472, %152
  br label %474

474:                                              ; preds = %473, %143
  br label %475

475:                                              ; preds = %474, %128
  br label %476

476:                                              ; preds = %475, %116
  br label %477

477:                                              ; preds = %476, %111
  br label %478

478:                                              ; preds = %477, %96
  br label %479

479:                                              ; preds = %478, %79
  br label %480

480:                                              ; preds = %479, %72
  br label %481

481:                                              ; preds = %480, %63
  br label %482

482:                                              ; preds = %481, %52
  br label %483

483:                                              ; preds = %482, %45
  br label %484

484:                                              ; preds = %483, %37
  br label %485

485:                                              ; preds = %484, %30
  br label %4

486:                                              ; preds = %445, %331
  %487 = load i64, ptr %1, align 8
  ret i64 %487
}

declare i32 @open(ptr noundef, i32 noundef, ...) #1

declare i32 @close(i32 noundef) #1

declare i64 @read(i32 noundef, ptr noundef, i64 noundef) #1

declare ptr @malloc(i64 noundef) #1

declare ptr @memset(ptr noundef, i32 noundef, i64 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  %9 = load i32, ptr %4, align 4
  %10 = add nsw i32 %9, -1
  store i32 %10, ptr %4, align 4
  %11 = load ptr, ptr %5, align 8
  %12 = getelementptr inbounds ptr, ptr %11, i32 1
  store ptr %12, ptr %5, align 8
  %13 = load i32, ptr %4, align 4
  %14 = icmp sgt i32 %13, 0
  br i1 %14, label %15, label %33

15:                                               ; preds = %2
  %16 = load ptr, ptr %5, align 8
  %17 = load ptr, ptr %16, align 8
  %18 = load i8, ptr %17, align 1
  %19 = sext i8 %18 to i32
  %20 = icmp eq i32 %19, 45
  br i1 %20, label %21, label %33

21:                                               ; preds = %15
  %22 = load ptr, ptr %5, align 8
  %23 = load ptr, ptr %22, align 8
  %24 = getelementptr inbounds i8, ptr %23, i64 1
  %25 = load i8, ptr %24, align 1
  %26 = sext i8 %25 to i32
  %27 = icmp eq i32 %26, 115
  br i1 %27, label %28, label %33

28:                                               ; preds = %21
  store i64 1, ptr @assembly, align 8
  %29 = load i32, ptr %4, align 4
  %30 = add nsw i32 %29, -1
  store i32 %30, ptr %4, align 4
  %31 = load ptr, ptr %5, align 8
  %32 = getelementptr inbounds ptr, ptr %31, i32 1
  store ptr %32, ptr %5, align 8
  br label %33

33:                                               ; preds = %28, %21, %15, %2
  %34 = load i32, ptr %4, align 4
  %35 = icmp sgt i32 %34, 0
  br i1 %35, label %36, label %54

36:                                               ; preds = %33
  %37 = load ptr, ptr %5, align 8
  %38 = load ptr, ptr %37, align 8
  %39 = load i8, ptr %38, align 1
  %40 = sext i8 %39 to i32
  %41 = icmp eq i32 %40, 45
  br i1 %41, label %42, label %54

42:                                               ; preds = %36
  %43 = load ptr, ptr %5, align 8
  %44 = load ptr, ptr %43, align 8
  %45 = getelementptr inbounds i8, ptr %44, i64 1
  %46 = load i8, ptr %45, align 1
  %47 = sext i8 %46 to i32
  %48 = icmp eq i32 %47, 100
  br i1 %48, label %49, label %54

49:                                               ; preds = %42
  store i64 1, ptr @debug, align 8
  %50 = load i32, ptr %4, align 4
  %51 = add nsw i32 %50, -1
  store i32 %51, ptr %4, align 4
  %52 = load ptr, ptr %5, align 8
  %53 = getelementptr inbounds ptr, ptr %52, i32 1
  store ptr %53, ptr %5, align 8
  br label %54

54:                                               ; preds = %49, %42, %36, %33
  %55 = load i32, ptr %4, align 4
  %56 = icmp slt i32 %55, 1
  br i1 %56, label %57, label %59

57:                                               ; preds = %54
  %58 = call i32 (ptr, ...) @printf(ptr noundef @.str.29)
  store i32 -1, ptr %3, align 4
  br label %196

59:                                               ; preds = %54
  %60 = load ptr, ptr %5, align 8
  %61 = load ptr, ptr %60, align 8
  %62 = call i32 (ptr, i32, ...) @open(ptr noundef %61, i32 noundef 0)
  %63 = sext i32 %62 to i64
  store i64 %63, ptr %7, align 8
  %64 = icmp slt i64 %63, 0
  br i1 %64, label %65, label %69

65:                                               ; preds = %59
  %66 = load ptr, ptr %5, align 8
  %67 = load ptr, ptr %66, align 8
  %68 = call i32 (ptr, ...) @printf(ptr noundef @.str.30, ptr noundef %67)
  store i32 -1, ptr %3, align 4
  br label %196

69:                                               ; preds = %59
  store i64 262144, ptr @poolsize, align 8
  store i64 1, ptr @line, align 8
  %70 = load i64, ptr @poolsize, align 8
  %71 = call ptr @malloc(i64 noundef %70)
  store ptr %71, ptr @text, align 8
  %72 = icmp ne ptr %71, null
  br i1 %72, label %76, label %73

73:                                               ; preds = %69
  %74 = load i64, ptr @poolsize, align 8
  %75 = call i32 (ptr, ...) @printf(ptr noundef @.str.31, i64 noundef %74)
  store i32 -1, ptr %3, align 4
  br label %196

76:                                               ; preds = %69
  %77 = load i64, ptr @poolsize, align 8
  %78 = call ptr @malloc(i64 noundef %77)
  store ptr %78, ptr @data, align 8
  %79 = icmp ne ptr %78, null
  br i1 %79, label %83, label %80

80:                                               ; preds = %76
  %81 = load i64, ptr @poolsize, align 8
  %82 = call i32 (ptr, ...) @printf(ptr noundef @.str.32, i64 noundef %81)
  store i32 -1, ptr %3, align 4
  br label %196

83:                                               ; preds = %76
  %84 = load i64, ptr @poolsize, align 8
  %85 = call ptr @malloc(i64 noundef %84)
  store ptr %85, ptr @stack, align 8
  %86 = icmp ne ptr %85, null
  br i1 %86, label %90, label %87

87:                                               ; preds = %83
  %88 = load i64, ptr @poolsize, align 8
  %89 = call i32 (ptr, ...) @printf(ptr noundef @.str.33, i64 noundef %88)
  store i32 -1, ptr %3, align 4
  br label %196

90:                                               ; preds = %83
  %91 = load i64, ptr @poolsize, align 8
  %92 = call ptr @malloc(i64 noundef %91)
  store ptr %92, ptr @symbols, align 8
  %93 = icmp ne ptr %92, null
  br i1 %93, label %97, label %94

94:                                               ; preds = %90
  %95 = load i64, ptr @poolsize, align 8
  %96 = call i32 (ptr, ...) @printf(ptr noundef @.str.34, i64 noundef %95)
  store i32 -1, ptr %3, align 4
  br label %196

97:                                               ; preds = %90
  %98 = load ptr, ptr @text, align 8
  %99 = load i64, ptr @poolsize, align 8
  %100 = call ptr @memset(ptr noundef %98, i32 noundef 0, i64 noundef %99)
  %101 = load ptr, ptr @data, align 8
  %102 = load i64, ptr @poolsize, align 8
  %103 = call ptr @memset(ptr noundef %101, i32 noundef 0, i64 noundef %102)
  %104 = load ptr, ptr @stack, align 8
  %105 = load i64, ptr @poolsize, align 8
  %106 = call ptr @memset(ptr noundef %104, i32 noundef 0, i64 noundef %105)
  %107 = load ptr, ptr @symbols, align 8
  %108 = load i64, ptr @poolsize, align 8
  %109 = call ptr @memset(ptr noundef %107, i32 noundef 0, i64 noundef %108)
  %110 = load ptr, ptr @text, align 8
  store ptr %110, ptr @old_text, align 8
  store ptr @.str.35, ptr @src, align 8
  store i64 134, ptr %6, align 8
  br label %111

111:                                              ; preds = %114, %97
  %112 = load i64, ptr %6, align 8
  %113 = icmp sle i64 %112, 141
  br i1 %113, label %114, label %119

114:                                              ; preds = %111
  call void @next()
  %115 = load i64, ptr %6, align 8
  %116 = add nsw i64 %115, 1
  store i64 %116, ptr %6, align 8
  %117 = load ptr, ptr @current_id, align 8
  %118 = getelementptr inbounds i64, ptr %117, i64 0
  store i64 %115, ptr %118, align 8
  br label %111, !llvm.loop !34

119:                                              ; preds = %111
  store i64 30, ptr %6, align 8
  br label %120

120:                                              ; preds = %123, %119
  %121 = load i64, ptr %6, align 8
  %122 = icmp sle i64 %121, 37
  br i1 %122, label %123, label %132

123:                                              ; preds = %120
  call void @next()
  %124 = load ptr, ptr @current_id, align 8
  %125 = getelementptr inbounds i64, ptr %124, i64 4
  store i64 130, ptr %125, align 8
  %126 = load ptr, ptr @current_id, align 8
  %127 = getelementptr inbounds i64, ptr %126, i64 3
  store i64 1, ptr %127, align 8
  %128 = load i64, ptr %6, align 8
  %129 = add nsw i64 %128, 1
  store i64 %129, ptr %6, align 8
  %130 = load ptr, ptr @current_id, align 8
  %131 = getelementptr inbounds i64, ptr %130, i64 5
  store i64 %128, ptr %131, align 8
  br label %120, !llvm.loop !35

132:                                              ; preds = %120
  call void @next()
  %133 = load ptr, ptr @current_id, align 8
  %134 = getelementptr inbounds i64, ptr %133, i64 0
  store i64 134, ptr %134, align 8
  call void @next()
  %135 = load ptr, ptr @current_id, align 8
  store ptr %135, ptr @idmain, align 8
  %136 = load i64, ptr @poolsize, align 8
  %137 = call ptr @malloc(i64 noundef %136)
  store ptr %137, ptr @old_src, align 8
  store ptr %137, ptr @src, align 8
  %138 = icmp ne ptr %137, null
  br i1 %138, label %142, label %139

139:                                              ; preds = %132
  %140 = load i64, ptr @poolsize, align 8
  %141 = call i32 (ptr, ...) @printf(ptr noundef @.str.36, i64 noundef %140)
  store i32 -1, ptr %3, align 4
  br label %196

142:                                              ; preds = %132
  %143 = load i64, ptr %7, align 8
  %144 = trunc i64 %143 to i32
  %145 = load ptr, ptr @src, align 8
  %146 = load i64, ptr @poolsize, align 8
  %147 = sub nsw i64 %146, 1
  %148 = call i64 @read(i32 noundef %144, ptr noundef %145, i64 noundef %147)
  store i64 %148, ptr %6, align 8
  %149 = icmp sle i64 %148, 0
  br i1 %149, label %150, label %153

150:                                              ; preds = %142
  %151 = load i64, ptr %6, align 8
  %152 = call i32 (ptr, ...) @printf(ptr noundef @.str.37, i64 noundef %151)
  store i32 -1, ptr %3, align 4
  br label %196

153:                                              ; preds = %142
  %154 = load ptr, ptr @src, align 8
  %155 = load i64, ptr %6, align 8
  %156 = getelementptr inbounds i8, ptr %154, i64 %155
  store i8 0, ptr %156, align 1
  %157 = load i64, ptr %7, align 8
  %158 = trunc i64 %157 to i32
  %159 = call i32 @close(i32 noundef %158)
  call void @program()
  %160 = load ptr, ptr @idmain, align 8
  %161 = getelementptr inbounds i64, ptr %160, i64 5
  %162 = load i64, ptr %161, align 8
  %163 = inttoptr i64 %162 to ptr
  store ptr %163, ptr @pc, align 8
  %164 = icmp ne ptr %163, null
  br i1 %164, label %167, label %165

165:                                              ; preds = %153
  %166 = call i32 (ptr, ...) @printf(ptr noundef @.str.38)
  store i32 -1, ptr %3, align 4
  br label %196

167:                                              ; preds = %153
  %168 = load i64, ptr @assembly, align 8
  %169 = icmp ne i64 %168, 0
  br i1 %169, label %170, label %171

170:                                              ; preds = %167
  store i32 0, ptr %3, align 4
  br label %196

171:                                              ; preds = %167
  %172 = load ptr, ptr @stack, align 8
  %173 = ptrtoint ptr %172 to i64
  %174 = load i64, ptr @poolsize, align 8
  %175 = add nsw i64 %173, %174
  %176 = inttoptr i64 %175 to ptr
  store ptr %176, ptr @sp, align 8
  %177 = load ptr, ptr @sp, align 8
  %178 = getelementptr inbounds i64, ptr %177, i32 -1
  store ptr %178, ptr @sp, align 8
  store i64 37, ptr %178, align 8
  %179 = load ptr, ptr @sp, align 8
  %180 = getelementptr inbounds i64, ptr %179, i32 -1
  store ptr %180, ptr @sp, align 8
  store i64 13, ptr %180, align 8
  %181 = load ptr, ptr @sp, align 8
  store ptr %181, ptr %8, align 8
  %182 = load i32, ptr %4, align 4
  %183 = sext i32 %182 to i64
  %184 = load ptr, ptr @sp, align 8
  %185 = getelementptr inbounds i64, ptr %184, i32 -1
  store ptr %185, ptr @sp, align 8
  store i64 %183, ptr %185, align 8
  %186 = load ptr, ptr %5, align 8
  %187 = ptrtoint ptr %186 to i64
  %188 = load ptr, ptr @sp, align 8
  %189 = getelementptr inbounds i64, ptr %188, i32 -1
  store ptr %189, ptr @sp, align 8
  store i64 %187, ptr %189, align 8
  %190 = load ptr, ptr %8, align 8
  %191 = ptrtoint ptr %190 to i64
  %192 = load ptr, ptr @sp, align 8
  %193 = getelementptr inbounds i64, ptr %192, i32 -1
  store ptr %193, ptr @sp, align 8
  store i64 %191, ptr %193, align 8
  %194 = call i64 @eval()
  %195 = trunc i64 %194 to i32
  store i32 %195, ptr %3, align 4
  br label %196

196:                                              ; preds = %171, %170, %165, %150, %139, %94, %87, %80, %73, %65, %57
  %197 = load i32, ptr %3, align 4
  ret i32 %197
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { noreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 15.0.1-++20220924023217+8d802f78fa8c-1~exp1~20220924143229.65"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}
!15 = distinct !{!15, !7}
!16 = distinct !{!16, !7}
!17 = distinct !{!17, !7}
!18 = distinct !{!18, !7}
!19 = distinct !{!19, !7}
!20 = distinct !{!20, !7}
!21 = distinct !{!21, !7}
!22 = distinct !{!22, !7}
!23 = distinct !{!23, !7}
!24 = distinct !{!24, !7}
!25 = distinct !{!25, !7}
!26 = distinct !{!26, !7}
!27 = distinct !{!27, !7}
!28 = distinct !{!28, !7}
!29 = distinct !{!29, !7}
!30 = distinct !{!30, !7}
!31 = distinct !{!31, !7}
!32 = distinct !{!32, !7}
!33 = distinct !{!33, !7}
!34 = distinct !{!34, !7}
!35 = distinct !{!35, !7}
