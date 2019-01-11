#include<stdio.h>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/mman.h>

#include "global.h"
#include "data.h"
#include "helper.h"

using namespace std;

#define BYTES 1024
char CommPort[4];


char* cnt_header = "Access-Control-Allow-Origin:http://localhost \n\rAccess-Control-Allow-Methods: POST, GET\r\nAccess-Control-Allow-Headers: X-PINGOTHER, Content-Type\r\nAccess-Control-Max-Age: 1728000\r\nContent-Type: application/json;\n\n";


int vport = -1;
int startServer(char *);
int CommHandle;

void Send(int, char*);
void Receive(int, char*);


void EndStream(int);
void EndLive(int);
void refine_and_decide(int);

int main(int argc, char* argv[])
{
	printf("Updated\n");
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	int c;
	
	printf("initialising globals\n");
	_init_Global();
	strcpy(CommPort,"10000");

	
	printf("starting servers\n");
	
	// Comm. server
	CommHandle = startServer(CommPort);
    
	printf("receiver server setup\n");
	
	
	// Receiver server listening
	if(fork() == 0)
	{
		printf("Comm. server online -- %d\n", CommHandle);
		// ACCEPT connections
		while (1)
		{
		    printf("waiting for comm. link\n");
			addrlen = sizeof(clientaddr);
			vport = accept (CommHandle, (struct sockaddr *) &clientaddr, &addrlen);
			printf("got reveiver\n");
			if (vport < 0)
				perror ("accept() error\n");
			else
			{
				if ( fork()==0 )
				{
					refine_and_decide(vport);
					exit(0);
				}
			}
		}
		exit(0);
	}
	
	while(1)
	{
	    printf("entter any number to exit\n");
    	scanf("%d", &c);
    	if(c == 1)
    	    DispTable();
    	else if(c == 1234)
    	    break;
	}

	// wait(NULL);
	// printf("something seems to have gone wrong. The Servers did not start\n");
	Dispose_Global();
	//_Dispose_Global;
	return 0;
}

//start server
int startServer(char *port)
{
	struct addrinfo hints, *res, *p;
	int handle;
	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		handle = socket (p->ai_family, p->ai_socktype, 0);
		if (handle == -1) continue;
		if (bind(handle, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (handle, 1000000) != 0 )
	{
		perror("listen() error");
		exit(1);
	}
	return handle;
}


void refine_and_decide(int vport)
{
    char mesg[99999], *reqline[3], *line;
	int rcvd, fd, bytes_read, foundAt, i;

	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(vport, mesg, 99999, 0);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected upexpectedly.\n");
	else    // message received
	{
		printf("%s", mesg);
		reqline[0] = strtok (mesg, " \t\n");
		printf("reqline : %s\n", reqline[0]);
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			while(line != NULL)
			{
                printf("%s\n", line);
                line = strtok(NULL, "\r\n");
            }
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				write(vport, "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
			    for(i = 0; i < 3; i++)
			        printf("reqline[%d] = %s\n", i, reqline[i]);
			
				// WAIT FOR MESSAGE
				
				if(startsWith(GETMSG_PATH, reqline[1]))
				{
					Receive(vport, reqline[1]);
					
				}
				// SEND MESSAGE
				
				else if(startsWith(SENDMSG_PATH, reqline[1]))
				{
   					Send(vport, reqline[1]);
				}
				else
				{
					write(vport, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
				}
			}
		}
	}
}


// Speaker
//Re-routs the message to receiver if receiver is online
void Send(int vport, char *reqline)
{
    char data_to_send[BYTES], query_data[50], *to_send, *line;
    int foundAt;
    printf("Sender: %s\n", reqline);
	strncpy(query_data, reqline + strlen(SENDMSG_PATH) + 5, 7);
    printf("%d", atoi(query_data));
	foundAt = Find(atoi(query_data));
	printf("Found At : %d", foundAt);
	if(foundAt == -1)
	{
		// Receiever is not online
		send(vport, "HTTP/1.0 200 OK\r\n", 17, 0);
		send(vport, cnt_header, strlen(cnt_header), 0);
		strcpy(data_to_send, "{ \"message\": \"Maybe the receiver is not online. Try after some time\"}");
		write(vport, data_to_send, strlen(data_to_send));
	}
	else
	{
		struct Live t = getLive(foundAt);
		send(vport, "HTTP/1.0 200 OK\r\nTransfer-Encoding: chunked\r\n ", 17, 0);
		send(vport, cnt_header, strlen(cnt_header), 0);
		strcpy(data_to_send, "{\"message\":\"sent\"}");
		write(vport, data_to_send, strlen(data_to_send));
		string data_to_receiver = "{\"message\": \"";
		printf(reqline);
		data_to_receiver.append(reqline + strlen(SENDMSG_PATH) + 12);
		data_to_receiver.append("\"}");
		
		printf("\nTo: %d -- at: %d\nDATA: %s\n\n", atoi(query_data), t.vport, data_to_receiver);
		printf("\nsending data: %s\nSize: %d", data_to_receiver, data_to_receiver.length());
		write(t.vport, data_to_receiver.c_str(), data_to_receiver.length());
		ShutLive(foundAt);
	}
	EndStream(vport);
}


//Receiver
// Long polls the client till he/she gets a message
void Receive(int vport, char* reqline)
{
    char data_to_send[BYTES], query_data[50];
    int foundAt;
	strncpy(query_data, reqline + strlen(GETMSG_PATH) + 5, 8);
	printf("Query Data: %s\n", query_data);
	if(!AddToLive(vport, atoi(query_data)))
	{
		send(vport, "HTTP/1.0 200 OK\r\n", 17, 0);
		send(vport, cnt_header, strlen(cnt_header), 0);
		strcpy(data_to_send, "CANNOT WAIT");
		write(vport, data_to_send, strlen(data_to_send));
	}
	else
	{
		send(vport, "HTTP/1.0 200 OK\r\n", 17, 0);
		send(vport, "Access-Control-Allow-Origin: http://localhost \r\n", strlen("Access-Control-Allow-Origin: http://localhost \r\n"), 0);
		send(vport, cnt_header, strlen(cnt_header), 0);
        // strcpy(data_to_send, "{ \"message=\"");
        // write(vport, data_to_send, strlen(data_to_send));
		printf("long polling client\n\n");
		return;
	}
	printf("ending connection\n");
	EndStream(vport);
	printf("connection terminated");
}

void EndStream(int vport)
{
	shutdown (vport, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(vport);
}

void EndLive(int vport)
{
	shutdown (vport, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(vport);
}
