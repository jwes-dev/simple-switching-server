#ifndef _paths_reg_h_
#define _paths_reg_h_

//Start into the server
char CONN_PATH[15] = "/connect";

//get chats
//usage: /tellme?uid=<num>
char GETMSG_PATH[15] = "/tellme";

//send a message
char SENDMSG_PATH[15] = "/hanasu";

static int *liveAdd;



void InitShareMem()
{
    liveAdd = (int*)mmap(NULL, sizeof *liveAdd, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

#endif
