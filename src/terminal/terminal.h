#ifdef TEST_TERMINAL
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/raw_ostream.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#endif

using llvm::SmallString;
using llvm::StringRef;
using llvm::raw_svector_ostream;
using llvm::raw_ostream;
#endif

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
    static BOOL WINAPI signal_handler(DWORD);
#else
    int tty_fd;
    unsigned cur_line = 0, cur_col = 0;
    static void signal_handler(int, siginfo_t *, void *);
#endif
    Terminal();
    void moveUp();
    void moveDown();
    void moveRight();
    void moveLeft();
    void changeColor(const Color &c, bool bold = false, bool underline = false, bool reverse = false);
    void resetColor();
    void moveTo(unsigned x, unsigned y);
    void move(int x, int y);
    void write(StringRef Text);
    void sleep(unsigned ms);
    void installSignalHandlers();
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
    terminal.installSignalHandlers();
    for (unsigned i = 0;i < 10;++i) {
        getchar();
    }
    terminal.write("Hello terminal!\n");
    terminal.changeColor(FUCHSIA);
    terminal.write("Hello terminal with color!\n");
    terminal.resetColor();
    for (unsigned i = 1;i < 50;++i) {
        for (unsigned j = 1;j < 50;++j) {
            terminal.moveTo(i, j);
            terminal.sleep(30);
            terminal.write("A");
        }
    }
}
#endif
