// config.nim - default configs for cc

// short version string
#ifndef CC_VERSION
#define CC_VERSION "alpha"
#endif

#ifndef CC_URL
#define CC_URL "https://github.com/ianfun/xcc.git"
#endif

// long version string
#ifndef CC_VERSION_FULL
#define CC_VERSION_FULL "XCC version " CC_VERSION " (" CC_URL ")"
#endif

// true if long is 64 bit
#ifndef CC_LONG64
#ifdef _WIN32
#define CC_LONG64 0
#else
#define CC_LONG64 1
#endif
#endif

// true if wchar is 32 bit
#ifndef CC_WCHAR32
#ifdef _WIN32
#define CC_WCHAR32 0
#else
#define CC_WCHAR32 1
#endif
#endif

// true if use GNU Readline libary
#ifndef CC_NO_RAEADLINE
#define CC_NO_RAEADLINE 1
#endif

// buffer size when call read()
#ifndef STREAM_BUFFER_SIZE
#define STREAM_BUFFER_SIZE 4096
#endif

// prompt when reading from stdin
#ifndef STDIN_PROMPT
#define STDIN_PROMPT ">>> "
#define WSTDIN_PROMPT L">>> "
#endif

#if defined(_WIN32) || defined(WIN32)
#ifndef CC_MESSAGE_BOX
#define CC_MESSAGE_BOX 0
#endif
#include <windows.h>
#define WINDOWS 1
template <DWORD n> static constexpr inline DWORD cstrlen(const wchar_t (&str)[n]) noexcept { return n - 1; }
#else
#define WINDOWS 0
#endif

template <unsigned long long n> static constexpr inline unsigned long long cstrlen(const char (&str)[n]) noexcept {
    return n - 1;
}
#define lquote "'"
#define rquote "'"

#define CC_EXIT_SUCCESS EXIT_SUCCESS
#define CC_EXIT_FAILURE EXIT_FAILURE

#ifndef CC_DEBUG
#if defined RELEASE || NDEBUG
#define CC_DEBUG 0
#else
#define CC_DEBUG 1
#endif
#endif

#if CC_DEBUG
#undef NDEBUG
#else
#define NDEBUG
#endif

#ifndef CC_ARENA_BLOCK_SIZE
#define CC_ARENA_BLOCK_SIZE 8192
#endif

#ifndef CC_SHOW_TAB_SIZE
#define CC_SHOW_TAB_SIZE 4
#endif
