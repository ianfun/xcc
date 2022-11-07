//===--- Options.h - Option info & table ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include <llvm/ADT/STLExtras.h>
#include <llvm/Option/OptTable.h>
#include <llvm/Option/Option.h>

namespace options {
  using namespace llvm::opt;
/// Flags specifically for clang options.  Must not overlap with
/// llvm::opt::DriverFlag.
enum ClangFlags {
  NoXarchOption = (1 << 4),
  LinkerInput = (1 << 5),
  NoArgumentUnused = (1 << 6),
  Unsupported = (1 << 7),
  CoreOption = (1 << 8),
  CLOption = (1 << 9),
  CC1Option = (1 << 10),
  CC1AsOption = (1 << 11),
  NoDriverOption = (1 << 12),
  LinkOption = (1 << 13),
  FlangOption = (1 << 14),
  FC1Option = (1 << 15),
  FlangOnlyOption = (1 << 16),
  DXCOption = (1 << 17),
  CLDXCOption = (1 << 18),
  Ignored = (1 << 19),
};

enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR, VALUES)                                      \
  OPT_##ID,
#include "clang/Driver/Options.inc"
    LastOption
#undef OPTION
  };

//===--- DriverOptions.cpp - Driver Options Table -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define PREFIX(NAME, VALUE) static const char *const NAME[] = VALUE;
#include "clang/Driver/Options.inc"
#undef PREFIX

static const OptTable::Info InfoTable[] = {
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM,  \
               HELPTEXT, METAVAR, VALUES)                                      \
  {PREFIX, NAME,  HELPTEXT,    METAVAR,     OPT_##ID,  Option::KIND##Class,    \
   PARAM,  FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS, VALUES},
#include "clang/Driver/Options.inc"
#undef OPTION
};

class DriverOptTable : public OptTable {
public:
  DriverOptTable()
    : OptTable(InfoTable) {}
};


const auto &getDriverOptTable() {
  static const DriverOptTable *Table = []() {
    auto Result = std::make_unique<DriverOptTable>();
    OptTable &Opt = *Result;
#define OPTTABLE_ARG_INIT
#include "clang/Driver/Options.inc"
#undef OPTTABLE_ARG_INIT
    return Result.release();
  }();
  return *Table;
}
}

