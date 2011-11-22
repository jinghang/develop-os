/* *****************************************************************
文件名称: start.c
简要说明：
建立时间: 2011-11-21
***************************************************************** */


#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"


PUBLIC u8    gdt_ptr[6];
PUBLIC DESCRIPTOR gdt[GDT_SIZE];

/* ********************************************************
函数名：cstart()
作用  ：将LOADER.BIN中的GDT复制到新的GDT中，修改gdt_ptr使其指向新的GDT即gdt
参数  ：无
返回值：无
***********************************************************/
PUBLIC void cstart()
{
    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
             "--------\"cstart\" gegins --------\n");
    /*将LOADER.BIN中的GDT复制到新的GDT中*/
    memcpy(&gdt,
           (void*)(*((u32*)(&gdt_ptr[2]))),
           *((u16*)(&gdt_ptr[0]))+1
           );
    /*修改gdt_ptr使其指向新的GDT即gdt*/
    *((u16*)(&gdt_ptr[0])) = GDT_SIZE * sizeof(DESCRIPTOR) - 1;/*Limit of GDT*/
    *((u32*)(&gdt_ptr[2])) = (u32)&gdt;                        /*Base of GDT*/

}
