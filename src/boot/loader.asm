;******************************************************************
;文件名称: loader.asm
;编译方法: nasm loader.asm -o loader.bin
;编译说明：编译成二进制文件。
;主要功能: 加载内核入内存，跳入保护模式，跳入内核。
;建立时间: 2011-11-17
;******************************************************************
org 0100h
    jmp LABEL_START

%include "pm.inc"
;////////////////////////////////////////////////////////////////////
;一些常量
;////////////////////////////////////////////////////////////////////
BaseOfStack             equ 0100h
BaseOfKernelFile        equ 08000h
OffsetOfKernelFile      equ 0h
BaseOfLoader            equ 09000h
OffsetOfLoader          equ 0100h
BaseOfLoaderPhyAddr     equ BaseOfLoader*10h      ;LOADER.BIN被加载到的物理地址
BaseOfKernelFilePhyAddr equ BaseOfKernelFile*10h  ;KERNEL.BIN被加载到的物理地址
PageDirBase             equ 200000h               ;页目录开始地址： 2M
PageTblBase             equ 201000h               ;页表开始地址    2M＋4K
KernelEntryPointPhyAddr equ 030400h               ;kernel的入口点
;///////////////////////////////////////////////////////////////////

;/////////////////////////////////////////////////////////////////////
;全局描述符及其选择子
;////////////////////////////////////////////////////////////////////
;                                段基址    段界限    属性
LABEL_GDT:            Descriptor 0,       0,       0                           ;空描述符
LABEL_DESC_FLAT_C:    Descriptor 0,       0FFFFFh, DA_CR |DA_32|DA_LIMIT_4K    ;0-4G 代码段
LABEL_DESC_FLAT_RW:   Descriptor 0,       0FFFFFh, DA_DRW|DA_32|DA_LIMIT_4K    ;0-4G 数据读写段
LABEL_DESC_VIDEO:     Descriptor 0B8000h, 0FFFFh,  DA_DRW|DA_DPL3              ;显存首地址

GdtLen     equ $ - LABEL_GDT                     ;全局描述符表的长度
GdtPtr     dw  GdtLen - 1                        ;段段界限
           dd  BaseOfLoaderPhyAddr + LABEL_GDT   ;基地址

;选择子
SelectorFlatC    equ LABEL_DESC_FLAT_C  - LABEL_GDT
SelectorFlatRW   equ LABEL_DESC_FLAT_RW - LABEL_GDT
SelectorVideo    equ LABEL_DESC_VIDEO   - LABEL_GDT + SA_RPL3


;/////////////////////////////////////////////////////////////////////


LABEL_START:
    ;设置栈
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,BaseOfStack

    ;获取内存信息
    call GetMemInfo

    ;软驱复位
    xor ah,ah
    xor dl,dl
    int 13h

    ;清屏
    mov ax,0600h
    mov bx,0700h         ;黑底白字
    mov cx,0             ;左上角(0,0)
    mov dx,0184fh        ;右下角(80,50)
    int 10h

    ;显示字符串 Booting
    mov dh,0
    call DispStrRealMode

    ;在根目录找内核文件然后复制到指定内存区域
    push word OffsetOfKernelFile      ;BaseOfKernelFile:OffsetOfKernelFile -> 数据缓冲区
    push word BaseOfKernelFile
    push word KernelFileName          ;ds:si -> 内核文件名
    push word ds
    push word 14                      ;根目录扇区总数
    push word 19                      ;根目录起始扇区编号
    call ReadFileInRoot               ;从根目录读取内核到指定内存区域
    add sp,12                         ;恢复栈
    cmp ax,1                          ;返回值为1则表示函数执行正确

    jz FileFound                      ;找到文件

    NotFound:                         ;没有找到文件
    mov dh,2
    call DispStrRealMode
    jmp $

    FileFound:

    call StopMotor                   ;关闭马达
    mov dh,1
    call DispStrRealMode                     ;显示字符串"Ready."

    ;准备跳入保护模式

    ;加载GDTR
    lgdt [GdtPtr]
    ;关中断
    cli

    ;打开地址线A20
    in al,92h
    or al,00000010b
    out 92h,al

    ;准备切换到保护模式
    mov eax,cr0
    or eax,1
    mov cr0,eax

    jmp dword SelectorFlatC:(BaseOfLoaderPhyAddr+ProtectModeEntry)

    jmp $



;=========================================================================
;字符串
;*************************************************************************
KernelFileName db  "KERNEL  BIN",0    ;KERNEL.BIN文件名
MsgLen         equ 9
BootMsg:       db  "Loading  "        ;9字节，不够用空格补齐
Msg1           db  "Ready.   "        ;
Msg2           db  "No KERNEL"        ;
;=========================================================================

;//////////////////////////////////////////////////////////////////////////
;函数名：DispStrRealMode
;作用：  显示一个字符串，函数数开始时，dh是字符串的序号
;/////////////////////////////////////////////////////////////////////////
DispStrRealMode:
    mov ax,MsgLen
    mul dh
    add ax,BootMsg
    mov bp,ax         ;ES:BP＝串地址
    mov ax,ds
    mov es,ax
    mov cx,MsgLen     ;串长度
    mov ax,01301h     ;
    mov bx,0007h      ;页号为0(BH = 0) 黑底白字(BL = 07h)
    mov dl,0
    int 10h
    ret
;//////////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////////
;函数名：StopMotor
;作用：  关闭软驱马达
;/////////////////////////////////////////////////////////////////////////
StopMotor:
    push dx
    mov dx, 03F2h
    mov al, 0
    out dx, al
    pop dx
    ret
;/////////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////////
;函数名：GetMemInfo
;作用：  获取内存信息保存到_MemChkbf指向的内存区域
;/////////////////////////////////////////////////////////////////////////
GetMemInfo:
    mov ebx,0               ;ebx=后续值，开始需设为0
    mov di,_MemChkBuf       ;es:di-> ARDS起始地址
    .MemChkLoop:
    mov eax,0E820h
    mov ecx,20
    mov edx,0534D4150h
    int 15h
    jc .MemChkFail              ;CF==1表示出错
    add di,20                   ;指针移到下一个ARDS
    inc dword [_dwMCRNumber]    ;_dwMCRNumber = ARDS的个数
    cmp ebx,0                   ;ebx==0表示是最后一个
    jne .MemChkLoop
    jmp .MemChkOK
    .MemChkFail:
    mov dword [_dwMCRNumber],0  ;出错，将ARDS的个数置0
    .MemChkOK:
    ret
;////////////////////////////////////////////////////////////////////////

%include "ReadFileInRoot.asm"

;/////////////////////////////////////////////////////////////////////////


;///////////////////////////////////////////////////////////////////////
;//////////////////////////////////////////////////////////////////////
;从此以后的代码在保护模式下执行
;32 位代码段. 由实模式跳入
[SECTION .s32]

ALIGN 32

[BITS 32]

ProtectModeEntry:

    mov ax,SelectorVideo
    mov gs,ax

    mov ax,SelectorFlatRW
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov ss,ax
    mov esp,TopOfStack

    mov ah,0Fh
    mov al,'P'
    mov [gs:((80*0+39)*2)],ax

    ;获取并显示内存信息
    call DispMemInfo
    ;启动分页
    call SetupPaging
    ;初始化内存
    call InitKernel

    jmp SelectorFlatC:KernelEntryPointPhyAddr

    jmp $
;//////////////////////////////////////////////////////////////////////
%include "lib.inc"
;/////////////////////////////////////////////////////////////////////

;/////////////////////////////////////////////////////////////////////
;函数名：DispMemInfo
;作用：  显示内存信息
;/////////////////////////////////////////////////////////////////////
DispMemInfo:

    push esi
    push edi
    push ecx

    ;显示标题
    push szMemChkTitle
    call DispStr
    add esp,4

    mov esi,MemChkBuf               ;保存ARDS的起始地址
    mov ecx,[dwMCRNumber]           ;ecx保存ARDS的个数

        NextARDS:
        mov edx,5                   ;每个ARDS有5个子项
        mov edi,ARDStruct           ;ARDS结构体变量的首地址，用以计算总内存数
            NextARDSItem:
            push dword [esi]
            call DispInt            ;显示ARDS的一个子项
            pop eax
            stosd                   ;将ARDS的一个子项存到ARDS结构体变量
            add esi,4               ;esi -> 下一个ARDS子项
            dec edx                 ;ARDS的子项个数减一
            cmp edx,0
            jnz NextARDSItem
        call DispReturn             ;显完一个ARDS就回车
        cmp dword [dwType],1        ;如果是可用内存，就将其加到内存总数
        jnz .NA
        mov eax,[dwBaseAddrLow]
        add eax,[dwLengthLow]
        cmp eax,[dwMemSize]
        jb .NA
        mov [dwMemSize],eax

        .NA:
        loop NextARDS

    call DispReturn
    push szRAMSize
    call DispStr
    add esp,4
    push dword [dwMemSize]
    call DispInt
    add esp,4

    pop ecx
    pop edi
    pop esi
    ret

;/////////////////////////////////////////////////////////////////////

;/////////////////////////////////////////////////////////////////////
;函数名：SetupPaging
;作用：  启动分页
;/////////////////////////////////////////////////////////////////////
SetupPaging:
    ;根据内存大小计算应初始化多少PDE及多少页表
    xor edx,edx
    mov eax,[dwMemSize]
    mov ebx,400000h                 ;400000h = 4M = 4096 * 1024，一个页表对应的内存大小
    div ebx
    mov ecx,eax                     ;此时ecx为页表的个数，也为PDE的个数
    test edx,edx
    jz .no_remainder
    inc ecx                         ;如果余数不为0就增加一个页表
    .no_remainder:
    push ecx                        ;暂存页表个数

    ; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞

    ; 首先初始化页目录
    mov ax,SelectorFlatRW
    mov es,ax
    mov edi,PageDirBase
    xor eax,eax
    mov eax,PageTblBase | PG_P |PG_USU | PG_RWW
        NextPageTbl:
        stosd
        add eax,4096                ;下一个页表的偏移，每个页表占4096个字节，每人页表中的项占4字节
        loop NextPageTbl
    ;初始化页表
    pop eax                         ;页表个数
    mov ebx,1024                    ;每个页表有1024个PTE
    mul ebx
    mov ecx,eax                     ;ecx = PTE个数 = 页表个数*1024
    mov edi,PageTblBase             ;页表的首地址
    xor eax,eax
    mov eax,PG_P | PG_USU | PG_RWW  ;从地址为0开始
        NextPTE:
        stosd
        add eax,4096                ;线性地址中，每一个页占4096个字节
        loop NextPTE
    mov eax,PageDirBase
    mov cr3,eax
    mov eax,cr0
    or  eax,80000000h
    mov cr0,eax
    jmp short .3
    .3:
    nop

    ret
;///////////////////////////////////////////////////////////////////////

;///////////////////////////////////////////////////////////////////////
;函数名：InitKernel
;作用： 将KERNEL.BIN的内容经过整理后对齐后放到新的位置
;//////////////////////////////////////////////////////////////////////
InitKernel:
    xor esi,esi
    mov cx,word [BaseOfKernelFilePhyAddr+2Ch]   ;ecx = pELFHdr->e_phnum
    movzx ecx,cx
    mov esi,[BaseOfKernelFilePhyAddr+1Ch]       ;esi = pELFHdr->e_phoff
    add esi,BaseOfKernelFilePhyAddr
    .Begin:
    mov eax,[esi+0]
    cmp eax,0
    jz  .NoAction
    push dword [esi+10h]
    mov eax,[esi+04h]
    add eax,BaseOfKernelFilePhyAddr
    push eax
    push dword [esi+08h]
    call MemCpy
    add esp,12
    .NoAction:
    add esi,20h                                 ;一个Program Header占20个字节
    dec ecx
    jnz .Begin

    ret

;/////////////////////////////////////////////////////////////////////

;///////////////////////////////////////////////////////////////////////
;数据段
;//////////////////////////////////////////////////////////////////////
[SECTION .data1]
ALIGN 32
LABEL_DATA:

;实模式下使用这些符号
_szMemChkTitle:         db "BaseAddrL BaseAddrH LengthLow LengthHigh   Type", 0Ah, 0
_szRAMSize:             db "RAM size:", 0
_szReturn:              db 0Ah, 0
;变量
_dwMCRNumber:           dd 0                ;内存信息结构体个数
_dwDispPos:             dd (80*6+0)*2       ;屏幕位置
_dwMemSize:             dd 0
_ARDStruct:                                 ;内存信息结构体
    _dwBaseAddrLow:     dd 0
    _dwBaseAddrHigh:    dd 0
    _dwLengthLow:       dd 0
    _dwLengthHigh:      dd 0
    _dwType:            dd 0
_MemChkBuf: times 256   db 0

;保护模式下用到这些符号
szMemChkTitle           equ BaseOfLoaderPhyAddr + _szMemChkTitle
szRAMSize               equ BaseOfLoaderPhyAddr + _szRAMSize
szReturn                equ BaseOfLoaderPhyAddr + _szReturn
dwDispPos               equ BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize               equ BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber             equ BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct               equ BaseOfLoaderPhyAddr + _ARDStruct
    dwBaseAddrLow       equ BaseOfLoaderPhyAddr + _dwBaseAddrLow
    dwBaseAddrHigh      equ BaseOfLoaderPhyAddr + _dwBaseAddrHigh
    dwLengthLow         equ BaseOfLoaderPhyAddr + _dwLengthLow
    dwLengthHigh        equ BaseOfLoaderPhyAddr + _dwLengthHigh
    dwType              equ BaseOfLoaderPhyAddr + _dwType
MemChkBuf               equ BaseOfLoaderPhyAddr + _MemChkBuf


;//////////////////////////////////////////////////////////////////////

;////////////////////////////////////////////////////////////////////////
;堆栈在数据末端
;///////////////////////////////////////////////////////////////////////
StackSpace: times 1024 db 0
TopOfStack  equ BaseOfLoaderPhyAddr + $ ;堆栈顶
