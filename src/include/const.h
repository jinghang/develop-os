#ifndef _CONST_H
#define _CONST_H

// assert macro
#define  ASSERT
#ifdef ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp) if (exp) ; \
                    else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

/* EXTERN is defined as extern except in global.c */
#define EXTERN extern

/*函数类型*/
#define PUBLIC
#define PRIVATE static

/* Boolean */
#define TRUE 1
#define FALSE 0

#define STR_DEFAULT_LEN 1024

/* Color */
/*
*e.g. MAKE_COLOR(BLUE, RED)
*     MAKE_COLOR(BLACK, RED) | BRIGHT
*     MAKE_cOLOR(BLACK, RED) | BRIGHT | FLASH
*/
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

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

// Process
#define SENDING    0x02  // set when proc trying to send
#define RECEIVING  0x04  // set when proc trying to recv

// TTY
#define NR_CONSOLES 3          // count of console

/*8259A interrupt controller ports*/
#define INT_M_CTL       0x20    /*IO port for interrupt controller           master*/
#define INT_M_CTLMASK   0x21    /*setting bits in this port disables ints    master*/
#define INT_S_CTL       0xA0    /*IO port for second interrupt controller    slave*/
#define INT_S_CTLMASK   0xA1    /*setting bits in this port disables ints    slave*/

/* 8253/8254 Programmable Interval Timer */
#define TIMER0          0x40    /* Timer0 端号 */
#define TIMER_MODE      0x43    /* 8253模式寄存器端口号 */
#define RATE_GENERATOR  0x34    /* 要写到模式寄存器中的数据 */

#define TIMER_FREQ      1193182L    /* 计数器的输入频率 */
#define HZ              100     /* 计数器输入频率 */

//AT keyboard
// 8042 ports
#define KB_DATA         0x60    // I/O port for keyboard data
                                // Read : Read Oput Buffer
                                // Write: Write Input Buffer(8042 Data & 8048 Command)

#define KB_CMD          0x64    // I/O port for keyboard command
                                // Read : Read Status Register
                                // Write: Write Input Buffer(8042 Command)

#define LED_CODE        0xED
#define KB_ACK          0xFA


/* VGA */
#define CRTC_ADDR_REG   0x3D4   // CRT Controller Register - Addr Register
#define CRTC_DATA_REG   0x3D5   // CRT Controller Register - Data Register
#define START_ADDR_H    0xC     // reg index of video mem start addr (MSB)
#define START_ADDR_L    0xD     // reg index of video mem start addr (LSB)
#define CURSOR_H        0xE     // reg index of cursor position (MSB)
#define CURSOR_L        0xF     // reg index of cursor position (LSB)
#define V_MEM_BASE      0xB8000 // base of color video memory
#define V_MEM_SIZE      0x8000  // 32K: B8000H -> BFFFFH

/* Hardware interrupts */
#define NR_IRQ          16  /* Number of IRQs */
#define	CLOCK_IRQ       0
#define	KEYBOARD_IRQ    1
#define	CASCADE_IRQ	    2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	    3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	    4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	    5	/* xt winchester */
#define	FLOPPY_IRQ	    6	/* floppy disk */
#define	PRINTER_IRQ	    7
#define	AT_WINI_IRQ	    14	/* at winchester */

// tasks
// 注意 TASK_XXX 的定义要与 global.c 中的对应
#define INVALID_DRIVER -20
#define INTERUPT       -10
#define TASK_TTY        0
#define TASK_SYS        1
#define ANY             (NR_TASKS + NR_PROCES + 10)
#define NO_TASK         (NR_TASKS + NR_PROCES + 20)

/* system call */
#define NR_SYS_CALL     3

// IPC
#define SEND            1
#define RECEIVE         2
#define BOTH            3

// magic chars used by 'printx'
#define MAG_CH_PANIC    '\002'
#define MAG_CH_ASSERT   '\003'

// when hard interupt occurs, a msg(with type == HARD_INT)
// will be send to some tasks
enum msgtype{
    HARD_INT = 1,
    GET_TICKS,     // sys task
};

#define RETVAL          u.m3.m3il

#endif //_CONST_H
