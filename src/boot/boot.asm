;******************************************************************
;文件名称: boot.asm
;编译方法: nasm boot.asm -o boot.bin
;编译说明：编译成二进制文件，写入引导扇区。
;主要功能: 实现FAT12文件系统和引导盘功能，找出loader.bin并加载入内存。
;建立时间: 2011-11-13
;******************************************************************

;********************************************************************************
BaseOfStack             equ 07c00h      ;Boot状态下堆栈基地址(栈底, 从这个位置向低地址生长)
BaseOfLoader            equ 09000h      ;LOADER.BIN 被加载到的位置 ----  段地址
OffsetOfLoader          equ 0100h       ;LOADER.BIN 被加载到的位置 ---- 偏移地址

RootDirSectors          equ 14          ;根目录占用的扇区数
FirstSectorOfRootDir    equ 19          ;根目录所在的第一个扇区号

;************************************************************************************

org 07c00h
    jmp short LABEL_START
    nop
;*****************FAT12文件系统头********************************
    BS_OEMName        db 'ForrestY'      ;OEM字符串，必须为8个字节
    BPB_BytesPerSec   dw 512             ;每个扇区字节数
    BPB_SecPerClus    db 1               ;每族多少扇区
    BPB_RsvdSecCnt    dw 1               ;Boot记录占用多少扇区
    BPB_NumFATs       db 2               ;共有多少FAT表
    BPB_RootEntCnt    dw 224             ;根目录文件数最大值
    BPB_TotSec16      dw 2880            ;逻辑扇区总数
    BPB_Media         db 0xf0            ;媒体描述符
    BPB_FATSz16       dw 9               ;每个FAT占用的扇区数
    BPB_SecPerTrk     dw 18              ;每磁道扇区数
    BPB_NumHeads      dw 2               ;磁头数
    BPB_HideSec       dd 0               ;隐藏扇区数
    BPB_TotSec32      dd 0               ;如果BPB_TotSec16为0，这个记录扇区数
    BS_DrvNum         db 0               ;中断13的中断号
    BS_Reserved1      db 0               ;未使用
    BS_BootSig        db 29h             ;扩展引导标记
    BS_VolID          dd 0               ;卷序列号
    BS_VolLab         db 'OrangeS0.02'   ;卷标，必须11个字节
    BS_FileSysType    db 'FAT12   '      ;文件系统类型，必须8个字节
    ;*******************************************************************

LABEL_START:
    ;设置栈
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,BaseOfStack

    ;清屏
    mov ax,0600h
    mov bx,0700h         ;黑底白字
    mov cx,0             ;左上角(0,0)
    mov dx,0184fh        ;右下角(80,50)
    int 10h

    ;显示字符串 Booting
    mov dh,0
    call DispStr

    ;软驱复位
    xor ah,ah
    xor dl,dl
    int 13h

    push word OffsetOfLoader      ;BaseOfLoader:OffsetOfLoader -> 数据缓冲区
    push word BaseOfLoader
    push word LoaderFileName      ;ds:si -> 所要寻找的文件名
    push ds
    push word 14                  ;要读取的扇区总数
    push word 19                  ;要读取的扇区的起始编号
    call ReadFileInRoot
    add sp,12                     ;恢复栈
    cmp ax,1
    jnz NotFound
    mov dh,1
    call DispStr
    jmp $
    NotFound:
    mov dh,2
    call DispStr
    jmp $

;=========================================================================
;变量
;*************************************************************************
wSectorNo    dw 0                ;保存要读取的扇区号
;=========================================================================

;=========================================================================
;字符串
;*************************************************************************
LoaderFileName db  "LOADER  BIN",0    ;LOADER.BIN文件名
MsgLen         equ 9
BootMsg:       db  "Booging  "        ;9字节，不够用空格补齐
Msg1           db  "Ready.   "        ;
Msg2           db  "No LOADER"        ;
;=========================================================================

;//////////////////////////////////////////////////////////////////////////
;函数名：DispStr
;作用：  显示一个字符串，函数数开始时，dh是字符串的序号
;/////////////////////////////////////////////////////////////////////////
DispStr:
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

;=========================================================================

;//////////////////////////////////////////////////////////////////////////
;函数名：ReadSector
;作用：  从第AX个Sector开始，将CL个Ssector读入es:bx中
;/////////////////////////////////////////////////////////////////////////
ReadSector:

    push bp
    mov bp,sp
    sub esp,2

    mov byte [bp-2],cl         ;暂存要读取的扇区数
    push bx                    ;保存bx
    mov bl,[BPB_SecPerTrk]     ;
    div bl                     ;ax/bl  扇区号/每磁道扇区数，商在al，余数在ah
    inc ah
    mov cl,ah                  ;起始扇区号
    mov dh,al                  ;磁头号
    shr al,1
    mov ch,al                  ;磁道号
    and dh,1

    pop bx
    mov dl,[BS_DrvNum]         ;驱动器号，0表示A盘
    .GoOnReading:
    mov ah,2                   ;表示要进行读操作
    mov al, byte [bp-2]        ;要读取的扇区数

    int 13h                    ;读中断
    jc  .GoOnReading           ;如果读取错误，CF==1，重新读取

    add esp,2
    pop bp

    ret
;/////////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////////
;函数名：SearchFileInSector
;作用：  从es:bx所指向的一个扇区缓冲区中寻找文件名为ds:si所指向的字符串的根目录项
;参数：  ds:si指向文件名字符串，es:bx指向根目录的某个扇区缓冲区
;返回值：ax==0表示没有找到，ax==1表示找到，es:di指向所要找的根目录项
;/////////////////////////////////////////////////////////////////////////
SearchFileInSector:
    mov cx,16        ;每一个扇区有16个根目录项
    mov di,bx        ;di指向第一个条目

    NextAntry:
    push cx
    push si
    push di
    mov cx,11        ;文件名长度为11个字节
    repe cmpsb       ;比较ds:si和es:di所身向的11个字节是否相等
    pop di
    pop si
    pop cx
    jz FoundFile     ;相等，表示找到文件名
    add di,32        ;没有找到，di指向下一个条目
    loop NextAntry
    mov ax,0         ;循环结束，没有找到
    ret              ;失败返回

    FoundFile:
    mov ax,1
    ret              ;成功返回
;///////////////////////////////////////////////////////////////////////////

;//////////////////////////////////////////////////////////////////////////
;函数名：ReadFileInRoot
;作用：  从根目录中读取文件放到指定的内存缓冲区中
;参数：  第0个参数：根目录的起始扇区；第1个参数：要读取的扇区数；
;       第2个参数：文件名的段地址；  第3个参数：文件名的偏移地址；
;       第4个参数：缓冲区段地址；    第5个参数：缓冲区偏移地址。
;返回值：ax==0执行失败，ax==1执行成功，
;//////////////////////////////////////////////////////////////////////////
ReadFileInRoot:
    push bp
    mov bp,sp
    xor ecx,ecx
    mov cx,word [bp+6]         ;第1个参数：要读取的扇区总数

    NextSectort:
    push cx                    ;保存循环次数
    mov ax,word [bp+4]         ;要读取的扇区号
    mov cl,1                   ;要读取的扇区个数
    mov bx,word [bp+14]
    mov es,word [bp+12]
    call ReadSector            ;读取一个扇区到es:bx所指向的缓冲区中
    mov si,word [bp+10]
    mov ds,word [bp+8]         ;ds:si -> 文件名
    call SearchFileInSector    ;在读到的扇区的缓冲区检索文件
    pop cx
    cmp ax,1
    jz StartReadFile           ;ax为1表示找到文件，es:di -> 根目录项
    add word [bp+4],1          ;下一个扇区
    loop NextSectort
    mov ax,0
    pop bp
    ret                        ;失败返回

    StartReadFile:
    ;读取文件
    mov ax,1
    pop bp
    ret                        ;成功返回

;//////////////////////////////////////////////////////////////////////////

;*************************************************************************
times 510 - ($ - $$) db 0
dw 0xaa55







