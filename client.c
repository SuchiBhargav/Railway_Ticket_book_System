#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<time.h>

#include"clientUtils.h"

#define PORT 8082

int main(int argc,char *argv[])
{
	
	struct sockaddr_in server;
	int scktDesc;
	char buffer[100];
	
	//SOCKET
	scktDesc = socket(AF_INET, SOCK_STREAM,0);
	if(scktDesc == -1){
		printf("socket creation failed\n");
		exit(0);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	
	//CONNECT
	int retC=connect(scktDesc, (struct sockaddr *)(&server), sizeof(server));
	if(retC ==-1){
		printf("Connection failed\n");
		exit(0);
	}
	system("clear");
	printf("Connection Established\n");
	
	while(client_Entry(scktDesc)!=3);
	
	//CLOSE
	close(scktDesc);
	return 0;
}
