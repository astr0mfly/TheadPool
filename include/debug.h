#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
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

#endif  //_DEBUG_H_