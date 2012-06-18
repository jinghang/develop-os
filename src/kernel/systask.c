#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

// <Ring 1> The main loop of Task SYS
PUBLIC void task_sys()
{
    MESSAGE msg;
    while(1){
        send_rec(RECEIVE, ANY, &msg);
        int src = msg.source;

        switch(msg.type){
        case GET_TICKS:
            msg.RECEIVE = ticks;
            send_recv(SEND, src, &msg);
            break;
        default:
            panic("unknown msg type");
            break;
        }
    }
}