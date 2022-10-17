#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include"serverUtils.h"

#define PORT 8082

int main()
{

	printf("Initializing connection...\n");
	struct sockaddr_in server, client;
	int client_addr_len, client_sd;
	
	//-----------------------------SOCKET------------------------------
	
	int sd = socket(AF_INET, SOCK_STREAM,0);
        if(sd==-1)
        {	perror("Socket creation failed\n");
		exit(0);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	
	//-----------------------------BIND---------------------------------
	
	printf("Binding socket...\n");
	int retB=bind(sd,(struct sockaddr *)(&server), sizeof(server));
	if(retB<0){
		perror("binding port failed\n");
		exit(0);
	}
	
	//----------------------------LISTEN--------------------------------
	
	int retL=listen(sd,10);
	if(retL==-1){
		perror("listen failed\n");
		exit(0);
	}
	printf("Listening...\n");
	client_addr_len = sizeof(client);
	
	//-----------------------------ACCEPT-------------------------------
	
	while(1){
		client_sd = accept(sd, (struct sockaddr *)&client, &client_addr_len);
		if(!fork()){

			close(sd);
			client_service(client_sd);
			exit(0);

		}
		else
		{
			close(client_sd);
		}

	}
	//CLOSE
	close(client_sd);
	close(sd);
	printf("Connection Closed..\n");
	return 0;
}

