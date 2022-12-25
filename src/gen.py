#! /usr/bin/env python3

# the super C++ code generator!

import sys
from genBuiltins import C_LIB_BUILTINS, COMPILER_BUILTINS

type_tags = (
	"TYAUTO",
	"TYCONST",
	"TYCONSTEXPR",
	"TYRESTRICT",
	"TYVOLATILE",
	"TYATOMIC",
	"TYINLINE",
	"TYSTATIC",
	"TYNORETURN",
	"TYEXTERN",
	"TYREGISTER",
	"TYTHREAD_LOCAL",
	"TYTYPEDEF",
	"TYLVALUE",
	"TYVOID",
	"TYCOMPLEX",
	"TYIMAGINARY",
	"TYNULLPTR",
	"TYREPLACED_CONSTANT",
	"TYPAREN"
)

keywords = (
# begin declaration-specifiers
	"extern",
	"short",	
	"register",
	"_Thread_local",
	"static",
	"auto",
	"constexpr",
	"inline",
	"typedef",
	"_Noreturn",

	"_Decimal32",
	"_Decimal128",
	"_Decimal64",
	"__int128",
	"_Float16",
	"_Accum",
	"_Fract",
	"_Sat",
	"__float128",
	"__float80",
	"__bf16",
	"half",
	"__ibm128",
	"__thread",
	"__auto_type",

	"_Imaginary",
	"_Complex",
	"_Atomic",
	"_Bool",
	
	"union",
	"struct",
	"enum",
	"unsigned",
	"signed",
	"void",
	"int",
	"char",
	"long",
	"float",
	"double",
	"typeof_unqual",
	"typeof",
	"_BitInt",

	"_Alignas",
	"const",
	"restrict",
	"volatile",

# end declaration-specifiers
  "__func__",
	"__declspec",
	"__attribute",
	"__builtin_choose_expr",
	"__builtin_offsetof",
	"__builtin_FILE",
	"__builtin_FUNCTION",
	"__builtin_LINE",
	"__builtin_COLUMN",
	"__builtin_types_compatible_p",
	"__builtin_va_arg",
	"__builtin_bit_cast",
	"__builtin_available",

	"__cdecl",
	"__stdcall",
	"__fastcall",
	"__thiscall",
	"__regcall",
	"__vectorcall",
	"__pascal",

	"__PRETTY_FUNCTION__",
	"__extension__",
  "__real",     # GNU extension
  "__imag",     # GNU extension
  "__label__",
	"asm",
	"nullptr",    # since C23
	"true",       # since C23
	"false",      # since C23
	"__objc_yes", # clang Objective-C extension
	"__objc_no",  # clang Objective-C extension
	"return",
	"for",
	"case",
	"switch",
	"goto",
	"default",
	"if",
	"else",
	"do",
	"sizeof",
	"continue",
	"break",
	"while",
	"_Alignof",
	"_Static_assert",
	"_Generic"
)
keywords_alias = (
	("__FUNCTION__", "__func__"),
	("__alignof__", "_Alignof"),
	("__alignof", "_Alignof"),
	("__builtin_alignof", "_Alignof"),
	("__asm", "asm"),
	("__asm__", "asm"),
	("_asm", "asm"),
	("_cdecl", "__cdecl"),
	("_fastcall", "__fastcall"),
	("_stdcall", "__stdcall"),
	("_thiscall", "__thiscall"),
	("_vectorcall", "__vectorcall"),
	("__attribute__", "__attribute"),
	("__complex", "_Complex"),
	("__complex__", "_Complex"),
	("__const", "const"),
	("__const__", "const"),
	("__imag__", "__imag"),
	("__real__", "__real"),
	("__inline", "inline"),
	("__inline__", "inline"),
	("_inline", "inline"),
	("__nullptr", "nullptr"),
	("__restrict", "restrict"),
	("__volatile", "volatile"),
	("__volatile__", "volatile"),
	("__signed", "signed"),
	("__signed__", "signed"),
	("__typeof", "typeof"),
	("__typeof__", "typeof"),
	("alignas", "_Alignas"),
	("alignof", "_Alignof"),
	("bool", "_Bool"),
	("static_assert", "_Static_assert"),
	("thread_local", "_Thread_local"),
	("__fp16", "half"),
	("_declspec", "__declspec"),
)
stmts = {
	"SHead": (),
	"SCompound": ("Stmt inner",),
	"SLabel": ("label_t label",),
	"SNamedLabel": ("label_t label2", "IdentRef labelName", "location_t labelLoc", ),
	"SGotoWithLocName": ("label_t location3", "location_t goto_loc3", "IdentRef goto_name", ),
	"SGotoWithLoc": ("label_t location2", "location_t goto_loc",),
	"SGoto": ("label_t location",),
	"SCondJump": ("Expr test", "label_t T", "label_t F",),
	"SSwitch": ("Expr itest", "xvector<SwitchCase> switchs", "xvector<GNUSwitchCase> gnu_switchs", "label_t sw_default", "location_t sw_default_loc",),
	"SIndirectBr": ("Expr jump_addr",),
	"SReturn": ("Expr ret", "location_t ret_loc",),
	"SExpr": ("Expr exprbody",),
	"SNoReturnCall": ("Expr call_expr",),
	"SAsm": ("xstring asms",),
	"SVarDecl": ("xvector<VarDecl> vars",),
	"SDecl": ("unsigned decl_idx", "CType decl_ty",),
	"SFunction": (
		"unsigned func_idx", 
		"IdentRef funcname", 
		"CType functy", 
		"Stmt funcbody", 
		"unsigned numLabels", 
		"label_t *indirectBrs",
		"unsigned localStart",
		"unsigned localSize",
		"location_t funcDefLoc"
	),
}
exprs = {
	"EConstant": ("llvm::Constant* C", "location_t constantLoc", "location_t constantEndLoc",),
	"EBin": ("Expr lhs", "enum BinOp bop", "Expr rhs",),
	"EUnary": ("Expr uoperand", "enum UnaryOp uop", "location_t opLoc",),
	"EString": ("llvm::Constant *string", "location_t stringLoc", "location_t stringEndLoc"),
	"EConstantArraySubstript": ("llvm::Constant *array", "uint64_t cidx", "location_t casLoc", "location_t casEndLoc",),
	"EInitList": ("location_t initStartLoc", "location_t initEndLoc", "xvector<Initializer> inits"),
	"EVoid": ("Expr voidexpr", "location_t voidStartLoc"),
	"EVar": ("unsigned sval", "IdentRef varName", "location_t varLoc",),
	"ECondition": ("Expr cond, cleft, cright",),
	"ECast": ("enum CastOp castop", "Expr castval",),
	"ECall": ("Expr callfunc", "xvector<Expr> callargs", "location_t callEnd", ),
	"ESubscript": ("Expr left, right",),
	"EMemberAccess": ("Expr obj", "xvector<unsigned> idxs", "location_t memberEndLoc",),
	"EArrToAddress": ("Expr arr3",),
	"EPostFix": ("enum PostFixOp pop", "Expr poperand", "location_t postFixEndLoc", ),
	"ESizeof": ("CType theType", "location_t sizeof_loc_begin", "location_t sizeof_loc_end",),
	"EBlockAddress": ("label_t addr", "location_t block_loc_begin", "IdentRef labelName"),
	"EBuiltinCall": ("IdentRef builtin_func_name", "llvm::Intrinsic::ID ID", "location_t builtin_call_start_loc", "xvector<Expr> buitin_call_args", "llvm::FunctionType *builtin_func_type"),
}
ctypes = {
	"TYPRIM": (),
	"TYPOINTER": ("CType p",),
	"TYTAG": ("unsigned idx", "enum TagKind tag", "IdentRef tag_name", "TagDecl tag_decl",),
	"TYBITFIELD": ("CType bittype", "unsigned bitsize", "unsigned bitoffset"),
	"TYARRAY": ("CType arrtype", "bool hassize", "unsigned arrsize",),
	"TYFUNCTION": ("CType ret", "xvector<Param> params", "bool isVarArg",),
	"TYVLA": ("CType vla_arraytype", "Expr vla_expr"),
	"TYBITINT": ("CType bitint_base", "unsigned bits")
}
def assertTuple(x: dict):
	assert(isinstance(x, dict))
	assert all(map(lambda it: isinstance(it, tuple), x.values())), "expect tuple"

assertTuple(ctypes)
assertTuple(exprs)
assertTuple(stmts)

def verbose(msg):
	sys.stdout.write(msg)

def switchGen(d, f, de):
	class Tree:
		def __init__(self):
			self.childs = {}

	root = Tree()
	for (h, v) in d.items():
		ptr = root
		for c in h:
			if c not in ptr.childs:
				ptr.childs[c] = Tree()
			ptr = ptr.childs[c]
		ptr.childs['\0'] = v
	def printer(n, i):
		tab = '\t' * i
		f.write(tab + "switch(*s++){\n")
		tab2 = '\t' + tab
		for k, v in n.childs.items():
			if isinstance(v, Tree):
				f.write(tab2 + "case '" + k + "':\n")
				printer(v, i + 1)
			else:
				f.write(tab2 + "case '\\0': return " + v + ";\n")
		f.write(tab2 + "default: return " + de + ";\n" + tab + "}\n")
	printer(root, 1)

def gen_keywords():
	verbose("generating keywords...")
	f = open("keywords.inc", "w")
	f.write("Token getKeyword(const char *s){\n")
	d = dict()
	for i in keywords:
		d[i] = 'K' + i
	switchGen(d, f, "TNul")
	f.write('}\n')
	f.close()
	verbose("done.\n")

def get_builtins_names():
	M = []
	for (_, value_list) in C_LIB_BUILTINS.items():
		for (name, _) in value_list:
			yield name
	for (name, _, _) in COMPILER_BUILTINS:
		yield name

def gen_tokens():
	def cstr(s):
		return s.replace('\\', '\\\\').replace('"', '\\"')
	verbose("generating tokens...")
	builtin_names = tuple(get_builtins_names())
	f = open("tokens.inc", "w")
	f.write("// C tokens -- generated by gen.py\n")
	ops = []
	ppkeywords = []
	def T(name, show, s = None):
		ops.append((name, show, s))
	def P(name, show, b = False):
		ppkeywords.append((name, show, b))
	T("TNul", "\\0")
	T("TNewLine", "\\n")
	T("TSpace", " ")
	T("TNot", "!")
	T("TDoubleQ", "\"")
	T("TBash", "~")
	T("TDolor", "$")
	T("TPercent",  "%")
	T("TBitAnd",  "&")
	T("TSignleQ",  "'")
	T("TLbracket",  "(")
	T("TRbracket",  ")")
	T("TMul",  "*")
	T("TAdd",  "+")
	T("TComma",  ",")
	T("TDash",  "-")
	T("TDot",  ".")
	T("TSlash",  "/")
	T("TColon",  ":")
	T("TSemicolon",  ";")
	T("TLt",  "<")
	T("TAssign",  "=")
	T("TGt",  ">")
	T("TQuestionMark",  "?")
	T("TMouse",  "@")
	T("TLSquareBrackets",  "[")
	T("TBackslash",  "\\")
	T("TRSquareBrackets",  "]")
	T("TXor",  "^")
	T("TUnderscore",  "_")
	T("TGraveAccent",  "`")
	T("TLcurlyBracket",  "{")
	T("TBitOr",  "|")
	T("TRcurlyBracket",  "}")
	T("TBitNot",  "~")
	T("TAddAdd", "++")
	T("TSubSub", "--")
	T("TArrow", "->")
	T("Tshl", "<<")
	T("Tshr", ">>")
	T("TGe", ">=")
	T("TLe", "<=")
	T("TNe", "!=")
	T("TEq", "==")
	T("TLogicalOr", "||")
	T("TLogicalAnd", "&&")
	T("TAsignAdd", "+=")
	T("TAsignSub", "-=")
	T("TAsignMul", "*=")
	T("TAsignDiv", "/=")
	T("TAsignRem", "%=")
	T("TAsignShl", "<<=")
	T("TAsignShr",">>=")
	T("TAsignBitAnd", "&=")
	T("TAsignBitOr", "|=")
	T("TAsignBitXor", "^=")
	T("TEllipsis2", "..")
	T("TEllipsis","...")
	T("TCharLit", "<char>")
	T("TStringLit", "<string>")
	T("PPlaceholder", "<placeholder>")
	T("PPSharp", "#")
	T("PPNumber", "<pp-number>")
	T("PPSharpSharp", "##")
	T("PPMacroPop", "<macro-pop>")
	T("PPMacroTraceLoc", "<macro-trace-loc>")
	T("TEOF", "<EOF>")

	P("TIdentifier", "<TIdentifier>") # name(identifier), after macro processing
	P("PPIdent", "<PPIdent>") # identifier, may be a macro
	P("PP_main", "main", True)
	P("PP__VA_ARGS__", "__VA_ARGS__", True)
	# built-in macros
	P("PP__LINE__", "__LINE__", True)
	P("PP__FILE__", "__FILE__", True)
	P("PP__DATE__", "__DATE__", True)
	P("PP__TIME__", "__TIME__", True)
	P("PP__COUNTER__", "__COUNTER__", True)
	P("PP_defined", "defined", True)
	P("PP_Pragma", "_Pragma", True)
	# preprocessor directives
	P("PPifdef", "ifdef", True)
	P("PPifndef", "ifndef", True)
	P("PPelif", "elif", True)
	P("PPendif", "endif", True)
	P("PPdefine", "define", True)
	P("PPundef", "undef", True)
	P("PPline", "line", True)
	P("PPinclude", "include", True)
	P("PPwarning", "warning", True)
	P("PPerror", "error", True)
	P("PPpragma", "pragma", True)

	f.write("enum Token: uint16_t {\n  ")
	f.write(",\n  ".join(x[0] for x in ops) + ",\n  ")
	f.write(",\n  ".join(('K' + i for i in keywords)) + ",\n  ")
	f.write(",\n  ".join((i[0] for i in ppkeywords)) + ",\n")
	f.write(",\n  ".join('K' + i for i in builtin_names))
	f.write("\n};\n")
	f.write("static const char *show(Token o){\n  switch(o) {\n")
	f.write('\n'.join(("    case K%s: return \"%s\";" % (k, k)) for k in keywords) + '\n')
	f.write('\n'.join(("    case %s: return \"%s\";" % (x[0], x[1])) for x in ppkeywords) + '\n')
	f.write('\n'.join(("    case %s: return \"%s\";" % (x[0], cstr(x[1]))) for x in ops))
	f.write('\n'.join(("    case K%s: return \"%s\";" % (b, b) for b in builtin_names)))
	f.write("\n    default: return \"(unknown token)\";\n  }\n}\n")
	f.close()
	f = open("IdentifierTableInit.inc", "w")
	f.write(',\n'.join(
		('{"%s", K%s}' % (i, i)) for i in keywords)
	)
	f.write(',\n')
	f.write(',\n'.join(
		('{"%s", K%s}' % (i, j) for (i, j) in keywords_alias))
	)
	f.write(',\n')
	f.write(',\n'.join(
		('{"%s", %s}' % (i[1], i[0])) for i in filter(lambda x: x[2], ppkeywords))
	)
	f.write(',\n')
	f.write(',\n'.join(
		('{"%s", K%s}' % (i, i)) for i in builtin_names)
	)
	f.write('\n')
	f.close()
	verbose("done.\n")

def gen_type_tags():
	verbose("generating type tags...")
	f = open("types.inc", "w")
	f.write("// optional type tags -- generated by gen.py\n")
	i = 0
	f.write("constexpr type_tag_t\n")
	for t in type_tags:
		x = 1 << i
		f.write("  %s=0x%x,\n" % (t, x))
		i += 1
	f.write("TYINVALID = 0;\n")
	f.close()
	verbose("done.\n")

def sizeof(l):
	for i in l:
		yield "sizeof(%s)" % i

def gen_expr():
	verbose("generating expressions...")
	f = open("expressions.inc", "w")
	f.write("// C expressions -- generated by gen.py\n")
	f.write("enum ExprKind: uint8_t {\n    " + ',\n    '.join(exprs.keys()) + '\n};\n')
	f.write("""\
struct OpaqueExpr {
OpaqueExpr()=delete;
~OpaqueExpr()=delete;
bool isSimple() const {
  switch (k) {
    case EVar:
    case EString:
    case EConstantArraySubstript:
    case EConstant:
      return true;
    default:
      return false;
  }
}
const location_t *getParenLLoc() const;
const location_t *getParenRLoc() const;
location_t *getParenLLoc();
location_t *getParenRLoc();
location_t getBeginLoc() const ;
location_t getEndLoc() const;
SourceRange getSourceRange() const {return SourceRange(getBeginLoc(), getEndLoc());}
ExprKind k;
CType ty;
union alignas(void*) {
""")
	for decls in exprs.values():
		if decls:
			f.write("    struct alignas(void*) {\n      " + ';\n        '.join(decls) + ';\n    };\n')
	f.write("""};\n};\n""")
	l = []
	for name, decls in exprs.items():		
		realname = name[1::] + "Expr"
		l.append(realname)
		if decls:
			f.write("struct " + realname + " {\n    enum ExprKind k=" + name + ";\n    CType ty;\n    struct alignas(void*) {\n    ")
			f.write('      ' + ';\n        '.join(decls) + ';')
			f.write("\n  };\n};\n")
		else:
			f.write("    struct " + realname + " {\n    enum ExprKind k=" + name + ";\n    CType ty;\n};\n")
	f.write("static uint8_t expr_size_map[] = {\n    " + 
		',\n    '.join(sizeof(l)) + 
		"\n};\n")
	f.write("static constexpr size_t expr_max_size = std::max({" + ', '.join(sizeof(l)) + "});\n")
	f.close()
	verbose("done.\n")

def gen_stmt():
	verbose("generating statements...")
	f = open("statements.inc", "w")
	f.write("// C statements and declarations -- generated by gen.py\n")
	f.write("enum StmtKind: uint8_t {\n    " + ',\n    '.join(stmts.keys()) + '\n};\n')
	f.write("""\
struct NullStmt {
    Stmt next = nullptr;
    enum StmtKind k;
};
template <typename T>
struct StmtIterator {
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	T s;
	StmtIterator(): s{nullptr} {}
	StmtIterator(std::nullptr_t): s{nullptr} {}
	StmtIterator(T Node) {
		switch (Node->k) {
			case SCompound:
				this->s = Node->inner;
				break;
			case SHead:
				this->s = Node->next;
				break;
			case SFunction:
				this->s = Node->funcbody->next;
				break;
    		default:
    			this->s = nullptr;
    	}
	}
	static StmtIterator get_raw(T Node) {
		StmtIterator I;
		I.s = Node;
		return I;
	}
	T operator*() {
		return s;
	}
	StmtIterator<T> &operator++();
	T operator->() const {
		return s;
	}
	bool operator !=(StmtIterator<T> other) const {
		return this->s != other.s;
	}
	bool operator ==(StmtIterator<T> other) const {
		return this->s == other.s;
	}
};
} // end namespace xcc

template <> struct std::iterator_traits<xcc::StmtIterator<xcc::Stmt>> {
    using difference_type = std::ptrdiff_t;
    using value_type = xcc::Stmt;
    using reference = xcc::Stmt&;
    using size_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
};
template <> struct std::iterator_traits<xcc::StmtIterator<xcc::const_Stmt>> {
    using difference_type = std::ptrdiff_t;
    using value_type = xcc::const_Stmt;
    using reference = xcc::const_Stmt&;
    using size_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
};

namespace xcc {
struct OpaqueStmt {
    Stmt next = nullptr;
    StmtKind k;
    using iterator = StmtIterator<Stmt>;
    using const_iterator = StmtIterator<const_Stmt>;
    iterator child_begin() { return iterator(this); }
    const_iterator child_begin() const { return const_iterator(this); }
	void viewAST() const;
	raw_ostream &writeGraph(raw_ostream &, const llvm::Twine &Title = "AST") const;
	iterator child_end() { return iterator(); }
	const_iterator child_end() const { return const_iterator(); }
    bool isTerminator() const {
        switch (k) {
            case SGoto:
            case SReturn:
            case SCondJump:
            case SIndirectBr:
            case SNoReturnCall:
            case SGotoWithLoc:
            case SGotoWithLocName:
                return true;
            default:
                return false;
        }
    }
union alignas(void*) {
""")
	for decls in stmts.values():
		if decls:
			f.write("    struct alignas(void*) {\n        " + ';\n        '.join(decls) + ';\n    };\n')
	f.write("""};\n};\n""")
	l = []
	for name, decls in stmts.items():
		realname = name[1::] + "Stmt"
		l.append(realname)
		if decls:
			f.write("struct " + realname + " {\n    Stmt next = nullptr;\n    enum StmtKind k=" + name + ";\n    struct alignas(void*) {\n    ")
			f.write('    ' + ';\n        '.join(decls) + ';')
			f.write("\n    };\n};\n")
		else:
			f.write("struct " + realname + " {\n    Stmt next = nullptr;\n    enum StmtKind k=" + name + ";\n};\n")

	f.write("static uint8_t stmt_size_map[] = {\n    " + 
		',\n    '.join(sizeof(l)) + 
		"\n};\n")
	f.write("static constexpr size_t stmt_max_size = std::max({" + ', '.join(sizeof(l)) + "});\n")
	f.write("""
template <typename T>
StmtIterator<T> &StmtIterator<T>::operator++() {
	s = s->next;
	return *this;
}
""")
	f.close()
	verbose("done.\n")

def gen_ctypes():
	verbose("generating C types...")
	f = open("ctypes.inc", "w")
	f.write("// C types -- generated by gen.py\n")
	f.write("enum CTypeKind: uint8_t {\n    " + ',\n    '.join(ctypes.keys()) + '\n};\n')
	f.write("""
#include "CType.cpp"
    union alignas(void*) {
""")
	for decls in ctypes.values():
		if decls:
			f.write("    struct alignas(void*) {\n        " + ';\n        '.join(decls) + ';\n    };\n')
	f.write("    };\n};\n")
	l = []
	for name, decls in ctypes.items():
		realname = name[2:3] + name[3::].lower() + "Type"
		l.append(realname)
		if decls:
			f.write("struct " + realname + "{\n    uint64_t tags;\n    struct alignas(void*) {\n    ")
			f.write('    ' + ';\n        '.join(decls) + ';')
			f.write("\n    };\n};\n")
		else:
			f.write("struct " + realname + " {\n    uint64_t tags;\n    };\n")
	f.write("static size_t ctype_size_map[] = {\n    " + 
		',\n    '.join(sizeof(l)) + 
		"\n};\n")
	f.write("static constexpr size_t ctype_max_size = std::max({" + ', '.join(sizeof(l)) + "});\n")
	f.close()
	verbose("done.\n")

class VBuf:
	def __init__(self, path, mode = 'wb'):
		self.path = path
		self.buf = bytearray()
		self.mode = mode
	def write(self, s):
		if isinstance(s, str):
			s = s.encode('utf-8')
		self.buf += s
	def pop_back(self):
		return self.buf.pop()
	def close(self):
		with open(self.path, self.mode) as f:
			f.write(self.buf)

def gen_builtins_table():
	verbose("generating builtins table...")
	f = VBuf("compiler_builtins.inc")
	f.write("// builtin header and attributes table - generated by gen.py\n")
	f.write(b"""\
enum CLibHeader: unsigned char {
	LIB_STDIO_H,
	LIB_STDLIB_H,
	LIB_STRING_H,
	LIB_STRINGS_H,
	LIB_MATH_H,
	LIB_WCHAR_H,
	LIB_CTYPE_H,
	LIB_MALLOC_H,
	LIB_SETJMP_H,
	LIB_UNISTD_H,
	LIB_COMPLEX_H,
	LIB_BLOCKS_H,
	LIB_STDARG_H
};
""")
	f.write("static const enum CLibHeader builtin_header_table[] = {\n")
	for (header, value_list) in C_LIB_BUILTINS.items():
		for (name, attr) in value_list:
			f.write('    LIB_%s, // %s\n' % (header.replace('.', '_').upper(), name))
	f.pop_back()
	f.write('\n};\n')
	f.write(b"""\
const char *getLibHeader(size_t index) {
    switch (builtin_header_table[index]) {
        case LIB_STDIO_H: return "stdio.h";
        case LIB_STDLIB_H: return "stdlib.h";
        case LIB_STRING_H: return "string.h";
        case LIB_STRINGS_H: return "strings.h";
        case LIB_MATH_H: return "math.h";
        case LIB_COMPLEX_H: return "complex.h";
        case LIB_WCHAR_H: return "wchar.h";
        case LIB_CTYPE_H: return "ctype.h";
        case LIB_MALLOC_H: return "malloc.h";
        case LIB_SETJMP_H: return "setjmp.h";
        case LIB_UNISTD_H: return "unistd.h";
	    case LIB_STDARG_H: return "stdarg.h";
        case LIB_BLOCKS_H: return "Blocks.h";
    }
    llvm_unreachable("bad header");
}
""")
	f.write("static const char *const builtin_attr_table[] = {\n")
	for (header, value_list) in C_LIB_BUILTINS.items():
		for (name, attr) in value_list:
			f.write('    "%s", // %s\n' % (attr, name))
	f.write('\n')
	for (name, args, attr) in COMPILER_BUILTINS:
		f.write('    "%s", // %s\n' % (attr, name))
	f.pop_back()
	f.write('\n};\n')
	f.close()
	verbose("done.\n")

class Monad:
	def __init__(self, fn):
		self.fn = fn
	def run(self):
		return self.fn()
	def __rshift__(self, other):
		if not isinstance(other, Monad):
			other = Monad(other)
		return Monad(lambda: (self.fn() or True) and other.fn())

def main()->Monad:
	import argparse
	parser = argparse.ArgumentParser(description="the super C++ code generator\nselect targets to generate, or '--all' generate all")
	parser.add_argument("-all", action='store_true', help="generate all targets")
	parser.add_argument("-keywords", action='store_true', help="generate keywords")
	parser.add_argument("-tokens", action='store_true', help="generate tokens")
	parser.add_argument("-type_tags", action='store_true', help="generate type_tags")
	parser.add_argument("-expr", action='store_true', help="generate Expr")
	parser.add_argument("-stmt", action='store_true', help="generate Stmt")
	parser.add_argument("-ctypes", action='store_true', help="generate ctypes")
	parser.add_argument("-directive", action='store_true', help="generate directives")
	parser.add_argument("-builtins", action="store_true", help="generate builtins table")
	if len(sys.argv) == 1:
		return Monad(parser.print_help) >> (lambda: parser.error("no targets to generate"))
	args = parser.parse_args()
	if args.all:
		return (Monad(gen_keywords) >> 
				gen_tokens >> 
				gen_type_tags >> 
				gen_expr >> 
				gen_stmt >> 
				gen_ctypes >> 
				gen_builtins_table)
	if args.keywords:
		return Monad(gen_keywords)
	if args.expr:
		return Monad(gen_expr)
	if args.stmt:
		return Monad(gen_stmt)
	if args.ctypes:
		return Monad(gen_ctypes)
	if args.type_tags:
		return Monad(gen_type_tags)
	if args.tokens:
		return Monad(gen_tokens)
	if args.builtins:
		return Monad(gen_builtins_table)
	assert False

if __name__ == '__main__':
  main().run()
