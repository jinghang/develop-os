#ifndef _CONST_H
#define _CONST_H

/* EXTERN is defined as extern except in global.c */
#define EXTERN extern

/*函数类型*/
#define PUBLIC
#define PRIVATE static

/*GDT和IDT中描述符的个数 */
#define GDT_SIZE 128
#define IDT_SIZE 256

/*8259A interrupt controller ports*/
#define INT_M_CTL       0x20    /*IO port for interrupt controller           master*/
#define INT_M_CTLMASK   0x21    /*setting bits in this port disables ints    master*/
#define INT_S_CTL       0xA0    /*IO port for second interrupt controller    slave*/
#define INT_S_CTLMASK   0xA1    /*setting bits in this port disables ints    slave*/


#endif
