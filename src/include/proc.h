#ifndef _PROC_H
#define _PROC_H

typedef struct s_stackframe{
    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 retaddr;
    u32 eip;
    u32 cs;
    u32 eflags;
    u32 esp;
    u32 ss;
}STACK_FRAME;

typedef struct proc{
    STACK_FRAME regs;           /* process registers saved in stack frame */
    u16 ldt_sel;                /* gdt selector giving ldt base and limit */
    DESCRIPTOR ldts[LDT_SIZE];  /* local decriptors for code and data */
    int ticks;
    int priority;
    u32 pid;                    /* process id passed in from MM */
    char p_name[16];            /* name of the process */
    int p_flags;                /* a proc is runnable if p_flags==0 */
    MESSAGE* p_msg;
    int p_recvfrom;
    int p_sendto;
    int has_int_msg;            /* nonzero if an interrupt occurred when the task is not ready to deal with it */
    struct proc * q_sending;    /* queue of procs sending message to this proc */
    struct proc * next_sending; /* next proc in the sending queue (q_sending) */
    int nr_tty;
}PROCESS;

typedef struct s_task{
    task_f initial_eip; //task_f 在 type.h
    int stacksize;
    char name[32];
}TASK;

#define proc2pid(x) (x - proc_table)

/* number of task & procs */
#define NR_TASKS 2
#define NR_PROCS 3
#define FIRST_PROC  proc_table[0]
#define LAST_PROC   proc_table[NR_TASKS + NR_PROCS -1]

/* stacks of tasks */
#define STACK_SIZE_TESTA    0x8000
#define STACK_SIZE_TESTB    0x8000
#define STACK_SIZE_TESTC    0x8000
#define STACK_SIZE_TTY      0x8000
#define STACK_SIZE_SYS      0x8000


#define STACK_SIZE_TOTAL (STACK_SIZE_TESTA + \
                          STACK_SIZE_TESTB + \
                          STACK_SIZE_TESTC + \
                          STACK_SIZE_TTY   + \
                          STACK_SIZE_SYS)

#endif //_PROC_H
