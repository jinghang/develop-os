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
#include "global.h"

/* ********************************************************
函数名：cstart()
作用  ：将LOADER.BIN中的GDT复制到新的GDT中，修改gdt_ptr使其指向新的GDT即gdt
参数  ：无
返回值：无
***********************************************************/
PUBLIC void cstart()
{
    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
             "--------\"cstart\" begins --------\n");
    /*将LOADER.BIN中的GDT复制到新的GDT中*/
    memcpy(&gdt,
           (void*)(*((u32*)(&gdt_ptr[2]))),
           *((u16*)(&gdt_ptr[0]))+1
           );
    /*修改gdt_ptr使其指向新的GDT即gdt*/
    *((u16*)(&gdt_ptr[0])) = GDT_SIZE * sizeof(DESCRIPTOR) - 1;/*Limit of GDT*/
    *((u32*)(&gdt_ptr[2])) = (u32)&gdt;                        /*Base of GDT*/


    /* idt_ptr[6] 共6个字节 ：0~15:Limit  16~47: Base
        用作sidt/lidt的参数
     */
     u16* p_idt_limit = (u16*)(&idt_ptr[0]);
     u32* p_idt_base = (u32*)(&idt_ptr[2]);
     /* 填充IDI数据结构 */
     *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;
     *p_idt_base = (u32)&idt;


     init_prot();

     disp_str("-----\"cstart\" ends-----");

}
