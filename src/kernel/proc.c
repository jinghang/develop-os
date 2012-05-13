
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


PRIVATE void block(struct proc* p);
PRIVATE void unblock(struct proc* p);
PRIVATE int  msg_send(struct proc* current, int dest, MESSAGE* m);
PRIVATE int  msg_receive(struct proc* current, int src, MESSAGE* m);
PRIVATE int  deadlock(int src, int dest);

/*======================================================================*
    schedule   进程调度
 *======================================================================*/
PUBLIC void schedule()
{
    PROCESS* p;
    int greatest_ticks = 0;

    while(!greatest_ticks){
        for(p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++){
            if(p->ticks > greatest_ticks){
                greatest_ticks = p->ticks;
                p_proc_ready = p;
            }
        }

        if(!greatest_ticks){
            for(p = proc_table; p < proc_table+NR_TASKS+NR_PROCS; p++){
                p->ticks = p->priority;
            }
        }
    }
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*========================================================================*
    收发消息系统调用中断响应函数
 @param function 指明是发送消息还是接收消息
 @param src_dest 消息的发送者或消息的接收者
 @param m        消息指针
 @param p        发起调用的进程指针
 @param 如果成功就返回0
*========================================================================*/
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p)
{
    assert(k_reenter == 0); // make sure we are not in ring0
    assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS)) ||
           src_dest == ANY ||
           src_dest == INTERRUPT);

    int ret = 0;
    int caller = proc2pid(p);
    MESSAGE* mla = (MESSAGE*)va2la(caller, m);
    mla->source = caller;

    assert(mla->source != src_dest);

    if(function == SEND){
        ret == msg_send(p, src_dest, m);
        if(ret != 0){
            return ret;
        }
    }
    else if(function == RECEIVE){
        ret = msg_receive(p, src_dest, m);
        if(ret != 0){
            return ret;
        }
    }
    else{
        panic("{sys_sendrec} invalid function: "
              "%d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
    }

    return 0;
}

/*============================================================
 <Ring 0~1> Calculate the linear address of a certain segment
 of a given proc.

 @param p   whose (the proc ptr)
 @param idx which (one proc has more than one segments)

 @return the requird linear address.
*=============================================================*/
PUBLIC int ldt_set_linear(struct proc* p, int idx)
{
    struct descriptor* d = &p->ldts[idx];
    return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}


/*===========================================================
 <Ring 0~1> Virtual addr --> Linear addr.

 @param pid PID of the proc whose address is to be calculated.
 @param va  Virtual address

 @return The linear address for the given virtual address.
*===========================================================*/
PUBLIC void* va2la(int pid, void* va)
{
    struct proc* p = &proc_table[pid];

    u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
    u32 la = seg_base + (u32)va;

    if(pid < NR_TASKS + NR_PROCS){
        assert(la == (u32)va);
    }

    return (void*)la;
}


/*===========================================================
 <Ring 0~3> Clear up a MESSAGE by setting each byte to 0.
 @param p The message to be cleared.
*==========================================================*/
PUBLIC void reset_meg(MESSAGE* p)
{
    memset(p, 0, sizeof(MESSAGE));
}


/*==========================================================
 <Ring 0> This routine is called after 'p_flags' has been
 set (!=0), it call 'schedule()' to choose another proc as
 the 'proc_ready'.

 @attention This routine does not change 'p_flags'.
    Make sure the 'p_flags' of the proc to be blocked
    has been set properly.

 @param p The proc to be blocked.
*==========================================================*/
PRIVATE void block(struct proc* p)
{
    assert(p->flags);
    schedule();
}


/*==========================================================
 <Ring 0> This is a dummy routine. It does nothing actually.
 When it is called, the 'p_flags' should have been cleared.

 @param p The unblocked proc.
*==========================================================*/
PRIVATE void unblock(struct proc* p)
{
    assert(p->p_flags == 0);
}


/*==========================================================
 <Ring 0> Check whether it is safe to send a message from src
 to dest. The routine will detect if the messaging graph
 contains a cycle. For A->B->C->A, then a deadlock occurs,
 because all of them will wait forever. If no cycles detected,
 it is considered as safe.

 @param src  Who wants to send message.
 @param dest To whom the message is sent.

 @return Zero if success.
*==========================================================*/
PRIVATE int deadlock(int src, int dest)
{
    struct proc* p = proc_table + dest;
    while(1){
        if(p->p_flags & SENDING){
            if(p->p_sendto == src){
                // print the chain
                p = proc_table + dest;
                printl("=_=%s", p->name);
                do{
                    assert(p->p_msg);
                    p = proc_table + p->p_sendto;
                    printl("->%s", p->name);
                }while(p != proc_table + src);
                printl("=_=");
                return 1;
            }
            p = proc_table + p->p_sendto;
        }
        else{
            break;
        }
    }
    return 0;
}


/*======================================================
 <Ring 0> Send a message to the dest proc. If dest is blocked
 waiting for for the message, copy the message to it and
 unlock dest. Otherwise the caller will be blocked and appended
 to the dest's sending queue.
 @param current The caller, the sender.
 @param dest    To whom the message is sent.
 @param m       The message.

 @return Zero of success.
*======================================================*/
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m)
{
    struct proc* sender = current;
    struct proc* p_dest = proc_table + dest;

    assert(proc2pid(sender) != dest);

    // check for deadlock here
    if(deadlock(proc2pid(sender), dest)){
        panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);
    }

    if((p_dest->p_flags & RECEIVING) && // dest is waiting for the msg
       (p_dest->p_recvfrom == proc2pid(sender) || p_dest->p_recvfrom == ANY)){

        assert(p_dest->p_msg);
        assert(m);

        phys_copy(va2la(dest, p_dest->p_msg), va2la(proc2pid(sender), m), sizeof(MESSAGE));
        p_dest->p_msg = 0;
        p_dest->p_flags &= ~RECEVING; // dest has receive the msg
        p_dest->p_recvfrom = NO_TASK;
        unblock(p_dest);

        assert(p_dest->p_flags == 0);
        assert(p_dest->p_msg == 0);
        assert(p_dest->recvfrom == NO_TASK);
        assert(p_dest->p_sendto == NO_TASK);
        assert(sender->p_flags == 0);
        assert(sender->p_msg == 0);
        assert(sender->recvfrom == NO_TASK);
        assert(sender->p_sendto == NO_TASK);
    }
    else{ // dest is not waiting for the msg
        sender->p_flags |= SENDING;
        assert(sender->p_flags == SENDING);
        sender->p_sendto = dest;
        sender->p_msg = m;

        // append to the sending queue
        struct proc* p;
        if(p_dest->q_sending){
            p = p_dest->q_sending;
            while(p->next_sending){
                p = p->next_sending;
            }
            p->next_sending = sender;
        }
        else{
            p_dest->q_sending = sender;
        }
        sender->next_sending = 0;

        block(sender);

        assert(sender->p_flags == SENDING);
        assert(sender->p_msg != 0);
        assert(sender->p_recvfrom == NO_TASK);
        assert(sender->p_sendto == dest);

    }

}


/*===============================================================================
  <Ring 0> Try to get a message from the src proc. If src is blocked sending
  the message, copy the message from it and unblock src. Otherwise the caller
  will be blocked.

  @Param current The caller, the proc who wanna receive.
  @Param src     From whom the message will be received.
  @Param m       The message ptr to accept the message.

  @return Zero if success.
**=============================================================================*/
PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m)
{
    //
}











