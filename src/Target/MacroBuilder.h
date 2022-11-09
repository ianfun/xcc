 class MacroBuilder {
   raw_ostream &Out;
 public:
   MacroBuilder(raw_ostream &Output) : Out(Output) {}
  
   /// Append a \#define line for macro of the form "\#define Name Value\n".
   void defineMacro(const Twine &Name, const Twine &Value = "1") {
     Out << "#define " << Name << ' ' << Value << '\n';
   }
  
   /// Append a \#undef line for Name.  Name should be of the form XXX
   /// and we emit "\#undef XXX".
   void undefineMacro(const Twine &Name) {
     Out << "#undef " << Name << '\n';
   }
  
   /// Directly append Str and a newline to the underlying buffer.
   void append(const Twine &Str) {
     Out << Str << '\n';
   }
 };
 