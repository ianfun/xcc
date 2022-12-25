## The XCC C Compiler

A C23(C2X) C compiler.

![](screenshots/screenshots.png)

## JIT

Just In Time Compilation demo

![](screenshots/jit.png)

## Setting up your environment - Install LLVM

The main driver is tests/main.cpp.

XCC provides makefiles in tests directory.

In Windows, a MSYS2 installation is recommended for convenience(you can download binarys without building from source!).

you can install LLVM via

```bash
# MSYS2
$ pacman -S mingw-w64-x86_64-llvm
```

Also you install gdb debug tool.

```bash
$ pacman -S mingw-w64-x86_64-gdb
```

In Debian and Ubuntu, https://apt.llvm.org/ provides `llvm.sh` can help you install LLVM and Clang.

Or use apt

```bash
sudo apt install llvm-15
```

## Installing and Buliding

* Windows - MSYS2

Install LLD include headers and static libary files(.a)

```bash
$ pacman -S mingw-w64-x86_64-lld
```

Build XCC (assume you MSYS2 is installed in C:/msys64)

```bash
$ clang++ main.cpp -DCC_HAS_LLD -fno-exceptions -fno-rtti -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS C:/msys64/mingw64/lib/liblldWasm.a C:/msys64/mingw64/lib/liblldCOFF.a C:/msys64/mingw64/lib/liblldELF.a C:/msys64/mingw64/lib/liblldMachO.a C:/msys64/mingw64/lib/liblldMinGW.a C:/msys64/mingw64/lib/liblldCommon.a -lLLVM-15 -g C:/msys64/mingw64/bin/zlib1.dll
```

* Linux - GNU Make

Install LLD via apt/dpkg

```bash
$ sudo apt install liblld-15 liblld-15-dev
```

Or building from source

```bash
$ git clone https://github.com/llvm/llvm-project llvm-project
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=lld -DCMAKE_INSTALL_PREFIX=/usr/local ../llvm-project/llvm
$ make install
```

Finally, build XCC with GNU make.

```bash
$ make
$ make main
$ make testLexer
$ make testCpp
$ make testParser
$ make testCodeGen
$ make all
$ make all -j
```

## Features

Dump colored AST

![](screenshots/ast-dump-colored.png)

cdecl syntax

![](screenshots/cdecl.png)

Expressive diagnostics

![](screenshots/diagnostic.png)

Include stack information

![](screenshots/diagnostic2.png)

Supports Windows, WSL, Linux ...

![](screenshots/run.png)

Fix-it Hints

![](screenshots/fixit.png)

`_Complex`, `_Imaginary` support.

![](screenshots/complex.png)

* trigraphs

Support trigraphs(both two and three charactors) in command line option `-trigraphs`.

## Author

ianfun

* 015006@mst.hlis.hlc.edu.tw
* ianfun.github.io@gmail.com

## References

* C23: https://open-std.org/JTC1/SC22/WG14/www/docs/n3054.pdf

## Optmizations

* remove unused variables

  remove
  - global variables/functions which not used and have static linkage
  - local variables not used, if has initializer and the initializer has side-effects, replace with the initializer

![remove unused variables](screenshots/unused-variable.png)

* constant folding

  constant folding using LLVM's APInt, APFloat and `llvm::Constant` ...

## Alignment

Alignment affects size of objects.

```C++
root [0] struct alignas(void*) bar {};
root [1] alignof(bar)
(unsigned long) 8
root [2] struct foo {};
root [3] alignof(foo)
(unsigned long) 1
root [4] sizeof(foo)
(unsigned long) 1
root [5] sizeof(bar)
(unsigned long) 8

```

Alignments must be power of 2 and must less or equal than the object's size.

```C++
root [1] struct alignas(2) bar { int a; };
ROOT_prompt_1:1:8: error: requested alignment is less than minimum alignment of 4 for type 'bar'
struct alignas(2) bar { int a; };
```

## Memory Leaks

Use AddressSanitizer to detect memory leaks.

## TODO List

Feature          | Status
---------------- | ----------------
Atomic           | ❌
Align            | ❌
Ast Dump (Text)  | ✔
Ast Dump (JSON)  | ❌
Ast to .dot      | ❌
LLVM IR Reader   | ❌
LLVM BC Reader   | ❌
LLVM Assembly Reader | ❌
LLVM Machine Code Writer | ✔
LLVM IR Writer  | ✔
LLVM BC Writer  | ✔
LLVM Assembly Writer  | ✔
Complex Number   | ✔
VLA              | ✔
VLA in structure | ❌
GNU inline Assembly | ❌
Linking          | ❌
Target builtin   | ✔
Compiler(Other) builtin | ❌
Bit field        | ❌
Record Layout    | ❌
Union            | ❌
OpenCL           | ❌
VA_OPT           | ❌
C23 `_BitInt`      | ❌
C23 `constexpr`    | ✔
C23 `typeof`       | ✔
C23 `nullptr`      | ✔
C23 attribute    | ❌
GNU `__attribute__` | ✔
Interpreter/REPL | ❌
JIT Interpreter  | ❌
Recover parsing error | ❌
Diagnostic       | ✔
Source Manager   | ✔
Macro expansion location | ❌
Constant Folding | ✔
Vararg (All platforms) | ✔
