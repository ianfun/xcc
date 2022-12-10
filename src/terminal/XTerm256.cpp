bool Terminal::isRealTermianl(){
	return isatty(STDIN_FILENO);
}
Terminal::Terminal() {
	tty_fd = STDOUT_FILENO;
	if (!isatty(STDOUT_FILENO)) {
		tty_fd = open("/dev/tty", O_WRONLY);
		if (tty_fd == -1)
			tty_fd = STDOUT_FILENO;
	}
}
// https://tldp.org/HOWTO/Bash-Prompt-HOWTO/x361.html
void Terminal::moveTo(unsigned x, unsigned y) {
	SmallString<12> str;
	raw_svector_ostream OS(str);
	OS << "\33[" << x << ';' << y << 'H';
	write(str.str());
}
void Terminal::move(int x, int y) {
	cur_line += x;
	cur_col += y;
	moveTo(cur_line, cur_col);
}
// https://www.ditig.com/256-colors-cheat-sheet
void Terminal::setColor(const Color &c, bool bold, bool underline, bool reverse) {
	SmallString<12> str;
	raw_svector_ostream OS(str);
	OS << "\33[38;2;" << unsigned(c.R) << ';' << unsigned(c.G) << ';' << unsigned(c.B) << 'm';
	if (bold)
		OS << "\33[1m";
	if (underline)
		OS << "\33[4m";
	if (reverse)
		OS << "\33[7m";
	write(str.str());
}
void Terminal::moveUp() {
	if (cur_line > 0) {
		cur_line--;
		write("\x1b[A");
	}
}
void Terminal::moveDown() {
	cur_line++;
	write("\x1b[B");
}
void Terminal::moveLeft() {
	if (cur_col > 0) {
		cur_col--;
		write("\x1b[C");
	}
}
void Terminal::moveRight() {
	cur_col++;
	write("\x1b[D");
}
void Terminal::resetColor() {
	write("\33[0m");
}
void Terminal::write(StringRef text) {
	::write(tty_fd, text.data(), text.size());
}
void Terminal::sleep(unsigned ms) {
	int res;
	struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = ms;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}
Terminal::~Terminal() {}
