#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <mutex>
#include <stdio.h>
#include <cstdio>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define cleanErrno() (errno == 0 ? "None" : strerror(errno))

#define logErr(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define logWarn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define logInfo(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { logErr(M, ##__VA_ARGS__); errno=0; goto ERROR; }

#define sentinel(M, ...)  { logErr(M, ##__VA_ARGS__); errno=0; goto ERROR; }

#define checkMem(A) check((A), "Out of memory.")

#define checkDebug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto ERROR; }



enum LogType { DEBUG, INFO, WARN, ERROR };
static const std::string LogTypeStrings[] = { "DEBUG", "INFO", "WARN", "ERROR" };
static std::mutex log_mutex;
extern LogType LOG_LEVEL;//防止多处引用

template<typename... Args>
void cdebug(const char* format, Args... args) {
    clog(DEBUG, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void cinfo(const char* format, Args... args) {
    clog(INFO, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void cwarn(const char* format, Args... args) {
    clog(WARN, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void cerror(const char* format, Args... args) {
    clog(ERROR, format, args...);
}

/**
 * Internal function. Print to stdout using the given log level, in a
 * thread-safe manner.
 */
template<typename... Args>
static void clog(LogType level, const char* format, Args... args) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (level < LOG_LEVEL) return;
    printf("[%s] ", LogTypeStrings[level].c_str());
    printf(format, args...);
    printf("\n");
}


#endif  //_DEBUG_H_