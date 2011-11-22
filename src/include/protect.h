#ifndef _PROTECTED_H
#define _PROTECTED_H

/*存储段描述符或系统描述符*/
typedef struct s_descriptor /*共8字节*/
{
    u16 limit_low;          /* */
    u16 base_low;           /* */
    u8  base_mid;           /* */
    u8  attr1;              /* */
    u8  limit_high_attr2;   /* */
    u8  base_high;          /* */
}DESCRIPTOR;

/*中断向量*/
#define INT_VECTOR_IRQ0 0x20
#define INT_VECTOR_IRQ8 0x28

#endif
