;******************************************************************
;文件名称: kernel.asm
;编译方法:
;         nasm -f elf -o kernel.o kernel.asm
;         nasm -f elf -o string.o string.asm
;         nasm -f elf -o kliba.o kliba.asm
;         gcc -c -fno-builtin -o start.o start.c
;         ld -s -Ttext 0x30400 -o kernel.bin kernel.o string.o kliba.o start.o
;编译说明：内核映像。
;建立时间: 2011-11-13
;******************************************************************

%include "sconst.inc"

;//////////////////////////////////////////////////////////////////
;导入函数
;//////////////////////////////////////////////////////////////////
extern cstart
extern exception_handler
extern spurious_irq
extern kernel_main
extern disp_str
extern disp_int
extern delay
extern k_reenter
extern clock_handler
extern irq_table
extern sys_call_table

;/////////////////////////////////////////////////////////////////

;/////////////////////////////////////////////////////////////////
;导出函数
;////////////////////////////////////////////////////////////////
global _start
global restart

;异常处理//////
global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error
;硬件中断处理
global hwint00
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11
global hwint12
global hwint13
global hwint14
global hwint15

; 系统调用
global sys_call


;///////////////////////////////////////////////////////////////

;------------------------------------------------------------------------------------

;/////////////////////////////////////////////////////////////////
;导入全局变量
;/////////////////////////////////////////////////////////////////
extern gdt_ptr
extern idt_ptr
extern disp_pos
extern p_proc_ready
extern tss

;/////////////////////////////////////////////////////////////////


;-------------------------------------------------------------------------------------

;///////////////////////////////////////////////////////////////
;变量区
;/////////////////////////////////////////////////////////////////
[SECTION .bss]
StackSpace    resb 2*1024
StackTop:                          ;栈顶
;////////////////////////////////////////////////////////////////

;-------------------------------------------------------------------------------------

;///////////////////////////////////////////////////////////////
;数据区
;/////////////////////////////////////////////////////////////////
[SECTION .data]
clock_int_msg   db  "^",0
;////////////////////////////////////////////////////////////////

;-------------------------------------------------------------------------------------

;////////////////////////////////////////////////////////////////
;代码段
;///////////////////////////////////////////////////////////////
[section .text]

_start:
    mov esp,StackTop
    mov dword [disp_pos],0  ;disp_pos定义在global.h
    sgdt [gdt_ptr]          ;将gdtr寄存器中的值存到gdt_ptr所指的地址，gdt_ptr定义在global.h
    call cstart             ;cstart定义在start.c
    lgdt [gdt_ptr]          ;将；gdt_ptr所指向的地址的内容加载到gdtr寄存器
    lidt [idt_ptr]          ;idt_ptr定义在global.h

    jmp SELECTOR_KERNEL_CS:csinit

csinit:
    ;push 0
    ;popfd

    ;ud2 ; 产生一个UD异常，Indtel内置异常

    xor eax, eax
    mov ax, SELECTOR_TSS
    ltr ax      ;加载TSS

    jmp kernel_main

    ;jmp $
    ;sti
    ;hlt ; 进入暂停模式，时钟信号也停，当有中断产生时再恢复执行


;/////////////////////////////////////////////////////////////////////
;中断和异常 -- 硬件中断
;////////////////////////////////////////////////////////////////////

;----------------------------------------
%macro hwint_master 1

    call save

    in al, INT_M_CTLMASK        ;|屏蔽当前中断
    or al, (1 << %1)            ;|
    out INT_M_CTLMASK, al       ;|

    mov al, EOI                 ;|置EOI位
    out INT_M_CTL, al           ;|

    sti                         ;开中断， CPU在响应中断后会关中断，

    push %1                     ;|中断处理程序
    call [irq_table + 4 * %1]   ;|
    pop ecx                     ;|

    cli

    in al, INT_M_CTLMASK        ;|恢复接受当前中断
    and al, ~(1 << %1)          ;|
    out INT_M_CTLMASK, al       ;|

    ret

%endmacro
;---------------------------------------

;//////////////////////////////////////////////////////////////////////
;时钟中断例程
ALIGN 16
hwint00:        ; Interrupt routine for irq 0 (the clock)
    hwint_master 0

ALIGN 16
hwint01:        ; irq 1 (keybord)
    hwint_master 1

ALIGN 16
hwint02:        ; irq 2 (cascade)
    hwint_master 2

ALIGN 16
hwint03:        ; irq 3, second serial
    hwint_master 3

ALIGN 16
hwint04:        ; irq 4, first serial
    hwint_master 4

ALIGN 16
hwint05:        ; irq 5, XT winchester
    hwint_master 5

ALIGN 16
hwint06:        ; irq 6, floppy
    hwint_master 6

ALIGN 16
hwint07:        ; irq 7, printer
    hwint_master 7


;------------------------------------------------
%macro hwint_slave 1
    push %1
    call spurious_irq
    add esp, 4
    hlt
%endmacro
;------------------------------------------------

ALIGN 16
hwint08:        ; irq 8, realtime clock
    hwint_slave 8

ALIGN 16
hwint09:        ; irq 9, irq 2 redirected
    hwint_slave 9

ALIGN 16
hwint10:        ; irq 10
    hwint_slave 10

ALIGN 16
hwint11:        ; irq 11
    hwint_slave 11

ALIGN 16
hwint12:        ; irq 12
    hwint_slave 12

ALIGN 16
hwint13:        ; irq 13 , FPU exception
    hwint_slave 13

ALIGN 16
hwint14:        ; irq 14, AT winchester
    hwint_slave 14

ALIGN 16
hwint15:        ; irq 15
    hwint_slave 15


;/////////////////////////////////////////////////////////////////////
;中断和异常 -- 异常
;////////////////////////////////////////////////////////////////////
divide_error:
    push 0xFFFFFFFF ; no err code
    push 0          ; vector_no = 0
    jmp exception

single_step_exception:
    push 0xFFFFFFFF ; no err code
    push 1          ; vector_no = 1
    jmp exception

nmi:
    push 0xFFFFFFFF ; no err code
    push 2          ; vector_no = 2
    jmp exception

breakpoint_exception:
    push 0xFFFFFFFF ; no err code
    push 3          ; vector_no = 3
    jmp exception

overflow:
    push 0xFFFFFFFF ; no err code
    push 4          ; vector_no = 4
    jmp exception

bounds_check:
    push 0xFFFFFFFF ; no err code
    push 5          ; vector_no = 5
    jmp exception

inval_opcode:
    push 0xFFFFFFFF ; no err code
    push 6          ; vector_no = 6
    jmp exception

copr_not_available:
    push 0xFFFFFFFF ; no err code
    push 7          ; vector_no = 7
    jmp exception

double_fault:
    push 8          ; vector_no = 8
    jmp exception

copr_seg_overrun:
    push 0xFFFFFFFF ; no err code
    push 9          ; vector_no = 9
    jmp exception

inval_tss:
    push 10         ; vector_no = A
    jmp exception

segment_not_present:
    push 11         ; vector_no = B
    jmp exception

stack_exception:
    push 12         ; vector_no = C
    jmp exception

general_protection:
    push 13         ; vector_no = D
    jmp exception

page_fault:
    push 14         ; vector_no = E
    jmp exception

copr_error:
    push 0xFFFFFFFF ; no err code
    push 16          ; vector_no = 10h
    jmp exception



;中断或异常发生时，EIP、CS、EFLAGS已经被压栈，如果有错误码，错误码也被压栈
exception:
    call exception_handler
    add esp, 4*2    ;让栈顶指向 EIP， 栈中从顶向下依次是：EIP、CS、EFLAGS
    hlt             ;休眠等等中断


;////////////////////////////////////////////////////////////////


; ====================================================================================
;                                   save
; ====================================================================================
save:
    pushad
    push ds
    push es
    push fs
    push gs

    mov esi, edx            ;保存edx，edx在syscall.asm被用到

    mov dx, ss
    mov ds, dx
    mov es, dx

    mov edx, esi            ;恢复edx

    mov esi, esp            ; esi == 进程表首地址

    inc dword [k_reenter]               ;k_reenter++;
    cmp dword [k_reenter], 0            ;if(k_reenter == 0)
    jne .1                              ;{
    mov esp, StackTop                   ;   mov esp, StackTop  //切换到内核栈
    push restart                        ;   push restart
    jmp [esi + RETADR - P_STACKBASE]    ;   return;
                                        ;}
    .1:                                 ;else{ //已经在内核栈，不需要切换
    push restart_reenter                ;   push restart_reenter;
    jmp [esi +RETADR - P_STACKBASE]     ;   return ;
                                        ;}

; ====================================================================================
;                                 sys_call
; ====================================================================================
sys_call:
    call save

    sti

    push dword [p_proc_ready]
    push edx
    push ecx
    push ebx
    call [sys_call_table + eax * 4]
    add esp, 4*4
    mov  [esi + EAXREG - P_STACKBASE], eax  ; 返回值

    cli

    ret

;/////////////////////////////////////////////////////////////////////
; restart
;////////////////////////////////////////////////////////////////////
restart:
    mov esp, [p_proc_ready]
    lldt [esp+P_LDT_SEL]
    lea eax, [esp + P_STACKTOP]
    mov dword [tss + TSS3_S_SP0], eax

    restart_reenter:
    dec dword [k_reenter]
    pop gs
    pop fs
    pop es
    pop ds
    popad

    add esp, 4

    iretd




