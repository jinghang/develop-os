

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

#define TTY_FIRST (tty_table)
#define TTY_END   (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

// 处理键盘扫描码
PUBLIC void task_tty()
{
    TTY* p_tty;

    init_keyboard();

    for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++){
        init_tty(p_tty);
    }
    select_console(0);
    while(1){
        for(p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++){
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

//初始化传入的TTY
PRIVATE void init_tty(TTY* p_tty)
{
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);
}

//输出键盘字符
PUBLIC void in_process(TTY* p_tty, u32 key)
{
    //如果是可输出字符则将字符放到相应的TTY缓冲区
    if(!(key & FLAG_EXT)){
        put_key(p_tty, key);
    }
    else{//非输出字符
        int raw_code = key & MASK_RAW;
        switch(raw_code){

        case ENTER://回车
            put_key(p_tty, '\n');
            break;

        case BACKSPACE://退格
            put_key(p_tty, '\b');
            break;

        case UP: // 向上箭头键被按下
            if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
                scroll_screen(p_tty->p_console, SCR_DN);
            }
            break;

        case DOWN:
            if((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)){
                scroll_screen(p_tty->p_console, SCR_UP);
            }
            break;

        case F1:
        case F2:
        case F3:
        case F4:
        case F5:
        case F6:
        case F7:
        case F8:
        case F9:
        case F10:
        case F11:
        case F12:
            // Alt + F1 ~ F12
            if((key & FLAG_ALT_L) || (key && FLAG_ALT_R)){
                select_console(raw_code - F1);
            }
            break;

        default:
            break;
        }
    }
}

//将可输出字符放到TTY的缓冲区
PRIVATE void put_key(TTY* p_tty, u32 key)
{
    //TTY是否还有可用的缓冲空间
    if(p_tty->inbuf_count < TTY_IN_BYTES){
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        //如果到末尾
        if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES){
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        //缓冲区中字符个数
        p_tty->inbuf_count++;
    }
}

//读取键盘缓冲区的数据
PRIVATE void tty_do_read(TTY* p_tty)
{
    if(is_current_console(p_tty->p_console)){
        keyboard_read(p_tty);
    }
}

//将TTY中的数据输出到控制台屏幕
PRIVATE void tty_do_write(TTY* p_tty)
{
    if(p_tty->inbuf_count){
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        //如果读到缓冲区末尾
        if(p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES){
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;

        out_char(p_tty->p_console,ch); // console.c
    }
}

//
PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
    char* p = buf;
    int i = len;

    while(i){
        out_char(p_tty->p_console, *p++);
        i--;
    }
}

PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
    tty_write(&tty_table[p_proc->nr_tty], buf, len);
    return 0;
}

/*=================================================================

*================================================================*/
PUBLIC int sys_printx(int _unused1, int unused2, char* s, struct proc* p_proc)
{
    const char* p;
    char ch;

    char reenter_err[] = "? k_reenter is incorrect for unknown reason";
    reenter_err[0] = MAG_CH_PANIC;

    /*
	 * @note Code in both Ring 0 and Ring 1~3 may invoke printx().
	 * If this happens in Ring 0, no linear-physical address mapping
	 * is needed.
	 *
	 * @attention The value of `k_reenter' is tricky here. When
	 *   -# printx() is called in Ring 0
	 *      - k_reenter > 0. When code in Ring 0 calls printx(),
	 *        an `interrupt re-enter' will occur (printx() generates
	 *        a software interrupt). Thus `k_reenter' will be increased
	 *        by `kernel.asm::save' and be greater than 0.
	 *   -# printx() is called in Ring 1~3
	 *      - k_reenter == 0.
	 */
	 if(k_reenter == 0) // printx() called in Ring<1~3>
        p = va2la(proc2pid(p_proc),s);
     else if(k_reenter > 0) // printx() called in Ring<0>
        p = s;
     else // this should NOT happen
        p = reenter_err;

    /*
    @Note: If assertion fils in any TASK, the system will be halted;
    if it fails in a USER PROC, it will return like any normal syscall does.
    */
    if((*p == MAG_CH_PANIC) ||
       (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])){

        disable_int();
        char* v = (char*)V_MEM_BASE;
        const char* q = p + 1; // skip the magic char

        while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)){
            *v++ = *q++;
            *v++ = RED_CHAR;
            if(!*q){
                while(((int)v - V_MEM_BASE) % (SCR_WIDTH * 16)){
                    v++; // *v++ = ' '
                    *v++ = GRAY_CHAR;
                }
                q = p + 1;
            }
        }
        __asm__ __volatile__("hlt");
    }

    while((ch == *p++) != 0){
        if(ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT){
            continue; // skip the magic char
        }
        out_char(tty_table[p_proc->nr_tty].p_console, ch);
    }

    return 0;
}
