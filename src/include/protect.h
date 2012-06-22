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

/* 门描述符 */
typedef struct s_gate
{
    u16 offset_low;          /* offset low */
    u16 selector;           /* selector */
    u8 dcount;
    u8 attr;                /* P(1) DPL(2) DT(1) TYPE(4) */
    u16 offset_high;      /* offset high */
}GATE;

typedef struct s_tss{
    u32	backlink;
	u32	esp0;	/* stack pointer to use during interrupt */
	u32	ss0;	/*   "   segment  "  "    "        "     */
	u32	esp1;
	u32	ss1;
	u32	esp2;
	u32	ss2;
	u32	cr3;
	u32	eip;
	u32	flags;
	u32	eax;
	u32	ecx;
	u32	edx;
	u32	ebx;
	u32	esp;
	u32	ebp;
	u32	esi;
	u32	edi;
	u32	es;
	u32	cs;
	u32	ss;
	u32	ds;
	u32	fs;
	u32	gs;
	u32	ldt;
	u16	trap;
	u16	iobase;	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
}TSS;


/* GDT descriptor index */
/* LOADER 里面已经确定了 */
#define INDEX_DUMMY     0
#define INDEX_FLAT_C    1
#define INDEX_FLAT_W    2
#define INDEX_VIDEO     3

#define INDEX_TSS       4
#define INDEX_LDT_FIRST 5

/* Selector */
/* LOADER 里面已经确定了 */
#define SELECTOR_DUMMY      0
#define SELECTOR_FLAT_C     0X08
#define SELECTOR_FLAT_RW    0X10
#define SELECTOR_VIDEO      (0X18+3)    /* RPL = 3 */

#define SELECTOR_TSS        0x20        /* TSS */
#define SELECTOR_LDT_FIRST  0x28

#define SELECTOR_KERNEL_CS  SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS  SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS  SELECTOR_VIDEO


/* 每个任务有单独的LDT ，每个LDT中的描述符个数: */
#define LDT_SIZE        2
// descriptor indices in LDT
#define INDEX_LDT_C     0
#define INDEX_LDT_RW    1

/* 选择子类型说明 SA : Selector Attribute */
#define SA_RPL_MASK 0xFFFC
#define SA_RPL0     0
#define SA_RPL1     1
#define SA_RPL2     2
#define SA_RPL3     3

#define SA_TI_MASK  0xFFFB
#define SA_TIG      0
#define SA_TIL      4


/* 描述符类型说明 */
#define DA_32           0X4000  /*32位段*/
#define DA_LIMIT_4K     0X8000  /* 段界限粒度为4K字节 */
#define DA_DPL0         0X00    /* DPL = 0 */
#define DA_DPL1         0X20    /* DPL = 1 */
#define DA_DPL2         0X40    /* DPL = 2 */
#define DA_DPL3         0X60    /* DPL = 3 */
/* 存储段描述符类型值说明 */
#define DA_DR           0X90    /* 存在的只读数据段类型值 */
#define DA_DRW          0X92    /* 存在的可读写数据段属性值 */
#define DA_DRWA         0X93    /* 存在的已访问的可读写的数据段类型值 */
#define DA_C            0X98    /* 存在的只执行的代码段 */
#define DA_CR           0X9A    /* 存在的可执行可读代码段 */
#define DA_CC0          0X9C    /* 存在的只执行的一致的代码段 */
#define DA_CC0R         0X9E    /* 存在的可执行的可读的一致的代码段 */
/* 系统段描述符类型说明 */
#define DA_LDT          0X82    /* 局部描述符表段类型值 */
#define DA_TaskGate     0X85    /* 任务门类型值 */
#define DA_386TSS       0X89    /* 可用386任务状态段类型值 */
#define DA_386CGate     0X8C    /* 386 调用门类型值 */
#define DA_386IGate     0X8E    /* 386 中断门类型值 */
#define DA_386TGate     0X8F    /* 386 陷阱门类型值 */


/* 中断向量 */
#define INT_VECTOR_DIVIDE           0X0
#define INT_VECTOR_DEBUG            0X1
#define INT_VECTOR_NMI              0X2
#define INT_VECTOR_BREAKPOINT       0X3
#define INT_VECTOR_OVERFLOW         0X4
#define INT_VECTOR_BOUNDS           0X5
#define INT_VECTOR_INVAL_OP         0X6
#define INT_VECTOR_COPROC_NOT       0X7
#define INT_VECTOR_DOUBLE_FAULT     0X8
#define INT_VECTOR_COPROC_SEG       0X9
#define INT_VECTOR_INVAL_TSS        0XA
#define INT_VECTOR_SEG_NOT          0XB
#define INT_VECTOR_STACK_FAULT      0XC
#define INT_VECTOR_PROTECTION       0XD
#define INT_VECTOR_PAGE_FAULT       0XE
#define INT_VECTOR_COPROC_ERR       0X10

/*中断向量*/
#define INT_VECTOR_IRQ0             0x20
#define INT_VECTOR_IRQ8             0x28

/* 系统调用 */
#define INT_VECTOR_SYS_CALL         0x90

/* 宏 */
/* 线性地址－> 物理地址 */
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))

#endif /* _PROTECTED_H */
