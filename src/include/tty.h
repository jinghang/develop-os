#ifndef _TTY_H
#define _TTY_H

#define TTY_IN_BYTES    256     // tty input queue size

struct s_console;

typedef struct s_tty
{
    u32   in_buf[TTY_IN_BYTES];    // TTY输入缓冲区
    u32*  p_inbuf_head;            // 指向下一个空闲位置
    u32*  p_inbuf_tail;            // 指向键盘任务处理的键值
    int   inbuf_count;             // 缓冲区已经填充了多少
    struct s_console* p_console;   //
} TTY;

#endif //_TTY_H
