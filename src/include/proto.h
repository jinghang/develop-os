#ifndef _PROTO_H
#define _PROTO_H

#include "proc.h"

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void milli_delay(int milli_sec);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();

/* 以下是系统调用相关 */

/* proc.c */
PUBLIC int sys_get_ticks();         /* 系统中断向量表中的函数 */
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc);

/* syscall.asm */
PUBLIC void sys_call();             /* 系统调用中断统一入口 */

//系统调用 用户级
PUBLIC int  get_ticks();            /* 引发中断 */
PUBLIC void write(char* buf, int len);


#endif
