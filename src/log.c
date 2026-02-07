#include "log.h"
#include "arena.h"
#include "arena.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static THREAD_LOCAL Log_Context _log_context = { 0 };


static Log_Frame _log_create_frame(void)
{
    Log_Frame out = (Log_Frame) {
        .count = 0,
        .capacity = 4,
    };
    //out.messages = (Log_Message *)
    //    malloc(sizeof(Log_Message) * out.capacity);
    out.messages = arena_push_array(_log_context.arena, 
        Log_Message, out.capacity);

    return out;
}

void log_frame_begin(void)
{
    // Lazy initialization
    if (!_log_context.arena) {
        _log_context.arena = arena_alloc();
    }

    if (!_log_context.frames) {
        _log_context.capacity = 4;
        _log_context.count = 0;

        //_log_context.frames = (Log_Frame *)
        //    malloc(sizeof(Log_Frame) * _log_context.capacity);
        _log_context.frames = arena_push_array(_log_context.arena, Log_Frame, _log_context.capacity);
    }

    if (_log_context.count >= _log_context.capacity) {
        // _log_context.capacity *= 2;
        //_log_context.frames = (Log_Frame *)
        //    realloc(_log_context.frames,
        //    sizeof(Log_Frame) * _log_context.capacity);
        //
        arena_pop(_log_context.arena, _log_context.capacity);

        _log_context.capacity *= 2;
        _log_context.frames = arena_push_array(_log_context.arena, 
            Log_Frame, _log_context.capacity);
    }

    _log_context.frames[_log_context.count++] = _log_create_frame();
}



void log_emit(Log_Level level, String8 message)
{
    if (!_log_context.frames) {
        return;
    }

    // Last element from vector/stack
    Log_Frame *frame = &_log_context.frames[_log_context.count-1];
    if (frame->count >= frame->capacity) {
        //frame->capacity *= 2;
        //frame->messages = (Log_Message *)
        //    realloc(frame->messages,
        //    sizeof(Log_Message) * frame->capacity);
        arena_pop(_log_context.arena, frame->capacity);

        frame->capacity *= 2;
        frame->messages = arena_push_array(_log_context.arena, 
            Log_Message, frame->capacity);

    }

    frame->messages[frame->count++] = (Log_Message) {
        .msg = message,
        .level = level,
    };
}

String8 log_frame_peek(u32 level_mask)
{
    if (!_log_context.frames) {
        return (String8){ 0 };
    }

    Log_Frame *frame = &_log_context.frames[_log_context.count-1];

    u32 num_logs_in_mask = 0;
    u64 total_out_size = 0;

    for (u32 i = 0; i < frame->count; ++i) {
        if ((frame->messages[i].level & level_mask) == 0) {
            continue;
        }

        num_logs_in_mask++;
        total_out_size += frame->messages[i].msg.size;
    }

    if (num_logs_in_mask == 0) {
        return (String8){ 0 };
    }


    // Allocate extra bytes for null terminators
    total_out_size += num_logs_in_mask - 1;

    //String8 out = {
    //    .data = (u8 *) malloc(sizeof(u8) * total_out_size),
    //    .size = total_out_size,
    //};
    String8 out;
    out.size = total_out_size;
    out.data = arena_push_array(_log_context.arena, u8, total_out_size);

    u64 out_pos = 0;
    for (u32 i = 0; i < frame->count; ++i) {
        Log_Message *msg = &frame->messages[i];
        if ((msg->level & level_mask) == 0) {
            continue;
        }

        // Add newline to each log entry
        if (out_pos != 0) {
            out.data[out_pos++] = '\n';
        }

        memcpy(out.data + out_pos,
            msg->msg.data, msg->msg.size);

        out_pos += msg->msg.size;
    }

    

    return out;
}


String8 log_frame_end(u32 level_mask)
{
    if (!_log_context.frames) {
        return (String8){ 0 };
    }


    String8 out = log_frame_peek(level_mask);
    
    // Pop
    //free(_log_context.frames[_log_context.count-1].messages);
    if (LOG_FATAL & level_mask) {
        // We free the arena itself
        arena_release(_log_context.arena);
        _log_context.arena = NULL;
    } else {
        arena_clear(_log_context.arena);
    }
    
    _log_context.count--;

    return out;
}


void log_flush_level(u32 level_mask)
{
    String8 logs = log_frame_end(level_mask);
    if (!logs.size) {
        return;
    }

    // Multimasking
    if ((LOG_ERROR & level_mask)) {
        fprintf(stderr, "[ERROR]: %.*s\n", STR8_FMT(logs));
    } else if ((LOG_WARN & level_mask)) {
        fprintf(stdout, "[WARN]: %.*s\n", STR8_FMT(logs));
    } else {
        fprintf(stdout, "%.*s\n", STR8_FMT(logs));
    }
}
/*
 fprintf(stderr, "FATAL ERROR: " __VA_ARGS__); \
        fprintf(stderr, "\n At file %s:%d in %s()\n", __FILE__, __LINE__, __func__); \
*/
void log_flush()
{
    // -1 acts as the identity number for the
    // bitwise and (&) operation, because it's all 1s in bits
    // We exclude the log fatal enum
    String8 logs = log_frame_end(-1 ^ LOG_FATAL);
    if (!logs.size) {
        return;
    }

    fprintf(stdout, "%.*s\n", STR8_FMT(logs));
}
