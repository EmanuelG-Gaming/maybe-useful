#ifndef LOG_H_
#define LOG_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// This implements a logging system with journaling,
// for efficiently batching messages, in order to be sent
// to the standard output or to a file.
// This is done without calling individual printf() functions
// on each message, because it's usually slower.
//
// It defines 5 logging levels with increasing severity:
// LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR and LOG_FATAL.
///////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "arena.h"


///////////////////////////////////////////////////////////////////////////////
// Compiler checks
// Note that Clang supports both GCC and MSVC, so we need to separate them.
///////////////////////////////////////////////////////////////////////////////

// MSVC
#if !defined(HAS_MSVC)
#if defined(_MSC_VER) && !defined(__clang__)
#define HAS_MSVC 1
#else
#define HAS_MSVC 0
#endif
#endif

// Clang
#if !defined(HAS_CLANG)
#if defined(__clang__)
#define HAS_CLANG 1
#else
#define HAS_CLANG 0
#endif
#endif

// GCC
#if !defined(HAS_GCC)
#if defined(__GNUC__) && !defined(__clang__)
#define HAS_GCC 1
#else
#define HAS_GCC 0
#endif
#endif


#if HAS_MSVC
#define THREAD_LOCAL __declspec(thread)
#elif HAS_CLANG || HAS_GCC || HAS_TCC
#define THREAD_LOCAL __thread
#else
#error "Unsupported compiler!"
#endif

#ifndef FAIL_MESSAGE
#define FAIL_MESSAGE(...) \
    do { \
        fprintf(stderr, "FATAL ERROR: " __VA_ARGS__); \
        fprintf(stderr, "\n At file %s:%d in %s()\n", __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)
#endif // FAIL_MESSAGE 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef enum Log_Level {
    LOG_NONE = 0,

    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
} Log_Level;

///////////////////////////////////////////////////////////////////////////////
// Strings
///////////////////////////////////////////////////////////////////////////////

// Length-based string
typedef struct String8 {
    u8 *data;
    u64 size;
} String8;

#define STR8_LIT(s) (String8) { (u8 *) s, sizeof(s) - 1 }
#define STR8_FMT(s) (int)(s).size, (char *)(s).data

///////////////////////////////////////////////////////////////////////////////
// Logging
///////////////////////////////////////////////////////////////////////////////

typedef struct Log_Message {
    String8 msg;
    Log_Level level;
} Log_Message;

typedef struct Log_Frame {
    u32 count;
    u32 capacity;
    Log_Message *messages;
} Log_Frame;

typedef struct Log_Context {
    Arena *arena;

    u32 count;
    u32 capacity;
    Log_Frame *frames; 
} Log_Context;

void log_frame_begin(void);

// Returns a buffer
String8 log_frame_end(u32 level_mask);
String8 log_frame_peek(u32 level_mask);

void log_emit(Log_Level level, String8 message);
//void log_emitf(Log_Level level, const char *fmt, ...);

// Print to standard output
void log_flush_level(u32 level_mask);
void log_flush();

#define log_error_s(msg) log_emit(LOG_ERROR, (msg))
#define log_warn_s(msg) log_emit(LOG_WARN, (mesg))
#define log_info_s(msg) log_emit(LOG_INFO, (msg))

#define log_error(msg) log_emit(LOG_ERROR, STR8_LIT(msg))
#define log_warn(msg) log_emit(LOG_WARN, STR8_LIT(msg))
#define log_info(msg) log_emit(LOG_INFO, STR8_LIT(msg))

/*
#define log_fatal(...) \
    do { \
        log_frame_begin(); \
        log_emit(LOG_FATAL, STR8_FORMATTED("FATAL ERROR: " __VA_ARGS__)); \
        log_emit(LOG_FATAL, STR8_FORMATTED("\n At file %s:%d in %s()\n", __FILE__, __LINE__, __func__)); \
        log_flush_level(LOG_FATAL); \
        abort(); \
    } while (0)
*/
#define log_fatal(msg) \
    do { \
        log_frame_begin(); \
        log_emit(LOG_FATAL, STR8_LIT(msg)); \
        log_flush_level(LOG_FATAL); \
        abort(); \
    } while (0)
#ifdef __cplusplus
} // extern "C"
#endif

#endif // LOG_H_
