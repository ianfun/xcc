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
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}
void Terminal::signal_handler(int signo, siginfo_t *info, void *context) {
	switch (signo) {
	case SIGINT:
		{
		StringRef text = "Ctrl+C\n";
		::write(STDOUT_FILENO, text.data(), text.size());
		break;
		}
	default:
		break;
	}
}
void Terminal::installSignalHandlers() {
    struct sigaction act = { 0 };

    act.sa_flags = 0;

    act.sa_sigaction = &signal_handler;

    sigaction(SIGINT, &act, NULL);
}
Terminal::~Terminal() {}
