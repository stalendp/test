/*
 * sockets.h
 *
 *  Created on: Oct 14, 2013
 *      Author: stalendp
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"


int tcpservselect() {
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(listenfd, (struct socket_in*)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);

	maxfd = listenfd;
	maxi = -1;
	for (i=0; i<FD_SETSIZE; i++) {
		client[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for(;;) {
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(listenfd, &rset)) { // new client connection
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr_in*)&cliaddr, &clilen);

			for(i=0; i<FD_SETSIZE; i++) {
				if (client[i]<0) {
					client[i] = connfd; // save descriptor
					break;
				}
			}

			if(i==FD_SETSIZE)
				err_quit("too many clients");
			FD_SET(connfd, &allset);
			if(connfd>maxfd)
				maxfd = connfd; // for select
			if(i>maxi)
				maxi = i;  // max index in client[] array
			if(--nready<=0)
				continue;   // no more readable descriptors
		}

		for(i=0; i<maxi; i++) { // check all clients for data
			if((sockfd = client[i])<0)
				continue;
			if(FD_ISSET(sockfd, &rset)) {
				if( (n=read(sockfd, buf, MAXLINE))==0) {
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else {
					Written(sockfd, buf, n);
				}
				if(--nready<=0)
					break;   // no more readable descriptors
			}
		}
	}
}


#endif /* SOCKETS_H_ */
