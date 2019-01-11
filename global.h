#include<pthread.h>
#include<stdio.h>

#ifndef __GlobalData_h_

#define __GlobalData_h_

const int MaxConn = 100;

struct Live{
	int vport, uid;
	char rb, wb;
	
}*live, *start, *end, empty;

int* WriteBlock;
int* LiveSlot;

void _GC();

void _init_Global()
{
	int i = 0;
	struct Live *temp;
	WriteBlock = (int*)mmap(NULL, sizeof *WriteBlock, PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*WriteBlock = 0;
	LiveSlot = (int*)mmap(NULL, sizeof *LiveSlot, PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*LiveSlot = -1;
	live = (Live*)mmap(NULL, (sizeof *live) * MaxConn, PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	start = (Live*)mmap(NULL, sizeof *start, PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	start = live;
	end = (Live*)mmap(NULL, sizeof *end, PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	end = start;
	end += MaxConn;

	empty.uid = -1;
	empty.vport = -1;
	temp = start;
	while(i < MaxConn)
	{
		temp->vport = -1;
		temp->uid = -1;
		temp++;
		i++;
	}
}


void Dispose_Global()
{
	munmap(WriteBlock, sizeof *WriteBlock);
	munmap(LiveSlot, sizeof *LiveSlot);
	munmap(start, sizeof *start * MaxConn);
}

void nextLive()
{
	live++;
	if(live == end)
		live -= MaxConn;
}

struct Live getLive(int num)
{
	struct Live ret;
	struct Live *t = start;
	if(num >= MaxConn)
		return empty;
	t += num;
	ret.vport = t->vport;
	ret.uid = t->uid;
	return ret;
}

int ShutLive(int slot)
{
	struct Live *t = start;
	t += slot;
	printf("--shutting--\nvport = %d\nuid = %d\n\n", t->vport, t->uid);
	shutdown(t->vport, SHUT_RDWR); //All further send and recieve operations are DISABLED...
	close(t->vport);
	t->vport = -1;
	t->uid = -1;
}

int Find(int uid)
{
	struct Live *t = start;
	int i = 0;
	while(i < MaxConn)
	{
		if(t->uid == uid)
			return i;
		i++;
		t++;
	}
	return -1;
}

int AddToLive(int vport, int uid)
{
	printf("\n\npust data");
	struct Live *t = start;
	struct Live now;
	int slot = *LiveSlot;
	int i = 0;
	printf("\nwaiting for unlock: %d", *WriteBlock);
	while(*WriteBlock);
	printf("\nunlocked");
	*WriteBlock = 1;
	i = 0;
	if(live->uid == -1)
	{
		printf("\nslot available in first");
		*LiveSlot++;
		live->vport = vport;
		live->uid = uid;
		live++;
		*WriteBlock = 0;
		return 1;

	}
	printf("\nsearching for free spot");
	while(i < MaxConn && live->uid != -1)
	{
		nextLive();
		i++;
	}
	if(live->uid != -1)
	{
		printf("\nno free slot found");
		*WriteBlock = 0;
		return 0;
	}
	printf("\nfound slot and inserting\n\n");
	live->vport = vport;
	live->uid = uid;
	live++;
	*WriteBlock = 0;
	return 1;
}

void _GC()
{
    int i;
    struct Live *temp = start;
    while(1)
    {
        sleep(5);
        for(i = 0; i < MaxConn; i++)
        {
            if((temp + i)->vport != -1 && write((temp + i)->vport, " ", 1) == -1)
                printf("client :: uid = %d\t vport = %d\t terminated\n", (temp + i)->uid, (temp + i)->vport);
        }
    }
}

void DispTable()
{
	int i = 0;
	struct Live *temp = start;
	printf("\n\nVport\tUID\n");
	while(i < MaxConn && (temp + i)->vport != -1)
	{
		printf("%d\t%d\n", (temp + i)->vport, (temp + i)->uid);
		//temp++;
		i++;
	}
}

#endif
