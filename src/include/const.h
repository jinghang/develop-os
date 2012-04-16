#ifndef _CONST_H
#define _CONST_H

/* EXTERN is defined as extern except in global.c */
#define EXTERN extern

/*函数类型*/
#define PUBLIC
#define PRIVATE static

/* Boolean */
#define TRUE 1
#define FALSE 0

/*GDT和IDT中描述符的个数 */
#define GDT_SIZE 128
#define IDT_SIZE 256

/* 权限 */
#define PRIVILEGE_KRNL  0
#define PRIVILEGE_TASK  1
#define PRIVILEGE_USER  3

/* RPL */
#define RPL_KRNL    SA_RPL0
#define RPL_TASK    SA_RPL1
#define RPL_USER    SA_RPL3

/*8259A interrupt controller ports*/
#define INT_M_CTL       0x20    /*IO port for interrupt controller           master*/
#define INT_M_CTLMASK   0x21    /*setting bits in this port disables ints    master*/
#define INT_S_CTL       0xA0    /*IO port for second interrupt controller    slave*/
#define INT_S_CTLMASK   0xA1    /*setting bits in this port disables ints    slave*/


#endif
