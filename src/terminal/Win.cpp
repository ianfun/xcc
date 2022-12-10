bool Terminal::isRealTermianl(){
	DWORD dummy;
	return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dummy) != 0;
}
Terminal::Terminal() {
	assert(isRealTermianl() && "not a real terminal!");
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	hStderr = GetStdHandle(STD_ERROR_HANDLE);
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD originConsoleMode;
	GetConsoleMode(hStdin, &originConsoleMode);
	DWORD newMode = originConsoleMode | ENABLE_PROCESSED_OUTPUT;
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hStdout, &info);
	default_attrs = info.wAttributes;
	cursorPos = info.dwCursorPosition;
	if (newMode != originConsoleMode) {
		SetConsoleMode(hStdout, newMode);
		SetConsoleMode(hStderr, newMode);
	}
}
void Terminal::moveTo(unsigned x, unsigned y) {
	cursorPos.X = static_cast<SHORT>(x);
	cursorPos.Y = cursorPos.Y + static_cast<SHORT>(y);
	do_move();
}
void Terminal::move(int x, int y) {
	cursorPos.X += static_cast<SHORT>(x);
	cursorPos.Y += static_cast<SHORT>(y);
	do_move();
}
void Terminal::do_move() {
	SetConsoleCursorPosition(hStdout, cursorPos);
}
void Terminal::setColor(const Color &c, bool bold, bool underline, bool reverse) {
	DWORD attrs = 0;
	if (underline)
		attrs |= BACKGROUND_INTENSITY | COMMON_LVB_UNDERSCORE;
	if (bold)
		attrs |= FOREGROUND_INTENSITY;
	if (reverse)
		attrs |= COMMON_LVB_REVERSE_VIDEO;
	if (c.R > 64)
		attrs |= FOREGROUND_RED;
	if (c.G > 64)
		attrs |= FOREGROUND_GREEN;
	if (c.B > 64)
		attrs |= FOREGROUND_BLUE;
	SetConsoleTextAttribute(hStdout, attrs);
}
void Terminal::moveUp() {
	if (cursorPos.Y > 0) {
		cursorPos.Y--;
		do_move();
	}
}
void Terminal::moveDown() {
	cursorPos.Y++;
	do_move();
}
void Terminal::moveLeft() {
	if (cursorPos.X > 0) {
		--cursorPos.X;
		do_move();
	}
}
void Terminal::moveRight() {
	cursorPos.X++;
	do_move();
}
void Terminal::resetColor() {
	SetConsoleTextAttribute(hStdout, default_attrs);
}
void Terminal::write(StringRef text) {
	WriteConsole(hStdout, text.data(), text.size(), nullptr, nullptr);
}
void Terminal::sleep(unsigned ms) {
	::Sleep(ms);
}
Terminal::~Terminal() {}
