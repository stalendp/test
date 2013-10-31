#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "common.h"
#include "sockets.h"

int main(int argc, char** argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	
	if(argc!=2) {
		printf("usage: tcpcli <IPaddress>");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	
	connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	str_cli_nonbocking(stdin, sockfd);

	return 0;
}
