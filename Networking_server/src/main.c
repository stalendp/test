#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "sockets.h"

int main(void)
{
	int listenfd = 0,connfd = 0;
	struct sockaddr_in serv_addr;
	char sendBuff[1025];
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(55555);
	
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
	printf("server start successfully!\n");
	
	if(listen(listenfd, 10) == -1){
		printf("Failed to listen\n");
		return -1;
	}
    
	while(1) {
		connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request
		
		strcpy(sendBuff, "[server] welcome client!");
		write(connfd, sendBuff, strlen(sendBuff));
		
		close(connfd);
		sleep(1);
	}
	
	return 0;
}
