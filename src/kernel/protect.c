

#include "type.h"
#include "const.h"
#include "protect.h"
#include "global.h"
#include "proto.h"

PUBLIC void init_prot()
{
    //
}

PUBLIC void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
    int i;
    int text_color = 0x74; /* 灰底红字 */

    char * err_msg[] = {
        "#DB RESEVED",
        "--  NMI Interrupt",
        "#BP Breadpoint",
        "#OF Overflow",
        "#BR BOUND Range Exceeded",
        "#UD Invalid Opcode (Undefined Opcode)",
        "#NM Device Not Available (No Math Coprocessor)",
        "#DF Double Fault",
        "    Coprocessor Segment Overrun (reserved)",
        "#TS Invalid TSS",
        "#NP Segment Not Present",
        "#SS Stack-Segment Fault",
        "#GP General Protection",
        "#PF Page Fault",
        "--  (Intel reserved. Do not use.)",
        "#MF x87 FPU Floating-Point Error (Math Fault)",
        "#AC Alignment Check",
        "#MC Machine Check",
        "#XF SIMD Floating-Point Exception"
    };

    /* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
    disp_pos = 0;
    for(i = 0; i < 80*5; i++){
        disp_str(" ");
    }
    disp_pos = 0;

    disp_color_str("Exception! --> ", text_color);


}
