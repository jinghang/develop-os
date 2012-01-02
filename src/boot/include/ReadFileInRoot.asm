
;********************************************************************************
RootDirSectors          equ 14          ;根目录占用的扇区数
FirstSectorOfRootDir    equ 19          ;根目录所在的第一个扇区号
DeltaSectorNo           equ 17          ;族号＋RootDirSectors+DeltaSectorNo=族号所对应的扇区编号
SectorNoOfFAT1          equ 1           ;FAT1的第一个扇区编号
BPB_BytesPerSec         equ 512
BPB_SecPerTrk           equ 18
BS_DrvNum               equ 0
;************************************************************************************


;//////////////////////////////////////////////////////////////////////////
;函数名：ReadFileInRoot
;作用：  从根目录中读取文件放到指定的内存缓冲区中
;参数：  第0个参数：根目录的起始扇区；第1个参数：根目录扇区总数；
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
    mov ax,0                   ;失败返回
    jmp Return

    StartReadFile:
    ;读取文件，此时es:bx所指向的缓冲区,es:di -> 根目录项
    add di,01Ah
    mov ax,word [es:di]         ;文件的第一个族号
    .GoOn:
    push ax                     ;暂存族号
    add ax,RootDirSectors       ;
    add ax,DeltaSectorNo        ;此时，ax为族号所对应的扇区号
    mov cl,1
    call ReadSector             ;读取一个扇区放到es:bx所指向的地址
    pop ax                      ;当前族号,根据当前扇区的族号可在FAT中找到下一个扇区的族号
    push bx                     ;暂存地址
    push es
    call GetFatEntry            ;找下一个族号
    pop es                      ;还原地址
    pop bx
    cmp ax,0FFFh
    jz .EndOfFile               ;读取文件结束
    add bx,BPB_BytesPerSec    ;缓冲区指针前移一个扇区
    jmp .GoOn
    .EndOfFile:
    mov ax,1                    ;成功返回

    Return:
    pop bp
    ret

;//////////////////////////////////////////////////////////////////////////


;//////////////////////////////////////////////////////////////////////////
;函数名：ReadSector
;作用：  从第AX个Sector开始，将CL个Ssector读入es:bx中
;/////////////////////////////////////////////////////////////////////////
ReadSector:

    push bp
    mov bp,sp
    sub esp,2                  ;开辟两个字节的局部储存区

    mov byte [bp-2],cl         ;暂存要读取的扇区数
    push bx                    ;保存bx
    mov bl,BPB_SecPerTrk     ;
    div bl                     ;ax/bl  扇区号/每磁道扇区数，商在al，余数在ah
    inc ah
    mov cl,ah                  ;起始扇区号
    mov dh,al                  ;磁头号
    shr al,1
    mov ch,al                  ;磁道号
    and dh,1

    pop bx
    mov dl,BS_DrvNum         ;驱动器号，0表示A盘
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
;函数名：GetFatEntry
;作用：  根据保存在ax中的族号在FAT中找到相应的条目，结果放在ax中，结果就是下一个族号
;参数：  ax存放当前族号，es:bx指向数据缓冲区，调用前es:bx已经被暂存
;返回值：ax=下一个族号
;//////////////////////////////////////////////////////////////////////////
GetFatEntry:
    push bp
    mov bp,sp
    sub esp,2                  ;开辟两个字节的局部储存区用于储存奇偶值

    push ax
    mov ax,es
    sub ax,0100h               ;在es:bx指向的数据缓冲区前开辟4K空间用于存放FAT
    mov es,ax
    pop ax
    mov word [bp-2],0          ;默认为偶数
    mov bx,3
    mul bx
    mov bx,2
    div bx                     ;dx_ax *3/2 结果：商在ax，余数在dx
    cmp dx,0
    jz LABEL_EVEN              ;余数为0
    mov word [bp-2],1          ;余数不为0则为奇数
    LABEL_EVEN:
    xor dx,dx
    mov bx,BPB_BytesPerSec
    div bx                     ;执行后，ax保存FATEntry项相对于FAT的扇区号
                               ;       dx保存FATEntry在扇区内的偏移
    push dx
    mov bx,0                   ;es:bx -> （BaseOfLoader-100):00
    add ax,SectorNoOfFAT1      ;计算出FATEntry所在的扇区
    mov cl,2
    call ReadSector            ;一次读两个扇区
    pop dx
    add bx,dx
    mov ax,[es:bx]             ;一次读两个字节，一个FATEntry占1.5个字节
    cmp word [bp-2],1          ;根据奇偶修正所得出的结果
    jnz LABEL_EVEN_2
    shr ax,4                   ;如果为奇数就右移4位
    LABEL_EVEN_2:
    and ax,0FFFh

    add esp,2
    pop bp

    ret
;//////////////////////////////////////////////////////////////////////////
