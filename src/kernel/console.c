
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "tty.h"
#include "console.h"

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PUBLIC void out_char(CONSOLE* p_con, char ch);
PRIVATE void flush(CONSOLE* p_con);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);

//初始化控制台屏幕结构体
PUBLIC void init_screen(TTY* p_tty)
{
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    //显存总大小
    int v_mem_size = V_MEM_SIZE >> 1;

    //单个控制台的显存大小
    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr = con_v_mem_size * nr_tty;
    p_tty->p_console->v_mem_limit = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

    //默认光标位置在最开始处
    p_tty->p_console->cursor = p_tty->p_console->original_addr;

    if(nr_tty == 0){
        //第一个控制台用原来的光标位置
        p_tty->p_console->cursor = disp_pos / 2;
        disp_pos = 0;
    }
    else{
        out_char(p_tty->p_console, nr_tty + '1');
        out_char(p_tty->p_console, '#');
    }

    set_cursor(p_tty->p_console->cursor);
}

// 检查传入的控制台是否是当前活动控制台
PUBLIC int is_current_console(CONSOLE* p_con)
{
    return (p_con == &console_table[nr_current_console]);
}

// 将字符输出到控制台屏幕
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
    //光标在显存中的地址
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor *2);

    switch(ch){

    case '\n':
        if(p_con->cursor < p_con->original_addr +
           p_con->v_mem_limit - SCREEN_WIDTH){
            p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
            ((p_con->cursor - p_con->original_addr)/
             SCREEN_WIDTH + 1);
        }
        break;

    case '\b':
        if(p_con->cursor > p_con->original_addr){
            p_con->cursor--;
            *(p_vmem-2) = ' ';
            *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        }
        break;

    default:
        if(p_con->cursor <
           p_con->original_addr + p_con->v_mem_limit - 1){
            *p_vmem++ = ch;
            *p_vmem++ = DEFAULT_CHAR_COLOR;
            p_con->cursor++;
        }
        break;
    }

    while(p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE){
        scroll_screen(p_con, SCR_DN);
    }

    flush(p_con);
}

//刷新控制台屏幕
PRIVATE void flush(CONSOLE* p_con)
{
    if(is_current_console(p_con)){
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
    }
}

//设置光标位置
PRIVATE void set_cursor(unsigned int position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, ((position)>>8)&0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, ((position)&0xFF));
    enable_int();
}

//设置屏幕起始显存位置
PRIVATE void set_video_start_addr(u32 addr)
{
    disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

//选择控制台
PUBLIC void select_console(int nr_console)
{
    if((nr_console < 0) || (nr_console >= NR_CONSOLES)){
        return;
    }

    nr_current_console = nr_console;

    set_cursor(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);

}

//滚屏
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
    if(direction == SCR_UP){
        if(p_con->current_start_addr > p_con->original_addr){
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    }
    else if(direction == SCR_DN){
        if(p_con->current_start_addr + SCREEN_SIZE <
           p_con->original_addr + p_con->v_mem_limit){
            p_con->current_start_addr += SCREEN_WIDTH;
           }
    }
    else{}

    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}
