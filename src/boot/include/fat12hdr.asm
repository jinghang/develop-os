;********************************************************************************
RootDirSectors          equ 14          ;根目录占用的扇区数
FirstSectorOfRootDir    equ 19          ;根目录所在的第一个扇区号
DeltaSectorNo           equ 17          ;族号＋RootDirSectors+DeltaSectorNo=族号所对应的扇区编号
SectorNoOfFAT1          equ 1           ;FAT1的第一个扇区编号

;************************************************************************************


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
