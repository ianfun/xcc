#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/raw_ostream.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif

using llvm::SmallString;
using llvm::StringRef;
using llvm::raw_svector_ostream;
using llvm::raw_ostream;

struct Color {
	unsigned char R, G, B;
	constexpr Color(unsigned char R = 0, unsigned char G = 0, unsigned char B = 0): R{R}, G{G}, B{B} {}
};
constexpr static Color 
  BLACK = Color(0, 0, 0),
  WHITE = Color(255, 255, 255),
  RED = Color(255, 0, 0),
  GREEN = Color(0, 255, 0),
  BLUE = Color(0, 0, 255),
  YELLOW = Color(255, 255, 0),
  CYAN = Color(0, 255, 255),
  FUCHSIA = Color(255, 0, 255);

struct Terminal
{
	static bool isRealTermianl();
#ifdef _WIN32
	HANDLE hStdout, hStdin, hStderr;
	DWORD default_attrs;
	COORD cursorPos;
	void do_move();
#else
	int tty_fd;
	unsigned cur_line = 0, cur_col = 0;
#endif
	Terminal();
	void changeColor(const Color &c);
	void resetColor();
	void moveUp();
	void moveDown();
	void moveRight();
	void moveLeft();
	void setColor(const Color &c, bool bold = false, bool underline = false, bool reverse = false);
	void moveTo(unsigned x, unsigned y);
	void move(int x, int y);
	void write(StringRef Text);
	void sleep(unsigned ms);
	~Terminal();
};

#ifdef _WIN32
#include "Win.cpp"
#else
#include "XTerm256.cpp"
#endif

#ifdef TEST_TERMINAL
int main() {
	Terminal terminal;
	terminal.write("Hello terminal!\n");
	terminal.setColor(FUCHSIA);
	terminal.write("Hello terminal with color!\n");
	terminal.resetColor();
	for (unsigned i = 1;i < 50;++i) {
		for (unsigned j = 1;j < 50;++j) {
			terminal.moveTo(i, j);
			terminal.sleep(100);
			terminal.write("A");
		}
	}
}
#endif
