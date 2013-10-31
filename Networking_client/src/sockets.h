/*
 * sockets.h
 *
 *  Created on: Oct 14, 2013
 *      Author: stalendp
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

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
#include <fcntl.h>

#include "common.h"

void str_cli_nonbocking(FILE* fp, int sockfd) {
	int maxfdp1, val, stdineof;
	ssize_t n, nwritten;
	fd_set rset, wset;
	char to[MAXLINE], fr[MAXLINE];
	char *toiptr, *tooptr, *friptr, *froptr;

	val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	val = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

	val = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	toiptr = tooptr = to;  // initialize buffer pointers
	friptr = froptr = fr;
	stdineof = 0;

	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
	for (;;) {
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		if (stdineof == 0 && toiptr < &to[MAXLINE])
			FD_SET(STDIN_FILENO, &rset); // read from stdin
		if (friptr < &fr[MAXLINE])
			FD_SET(sockfd, &rset); // read from socket
		if (tooptr != toiptr)
			FD_SET(sockfd, &wset); // data to write to socket
		if (froptr != friptr)
			FD_SET(STDOUT_FILENO, &wset); // data to write to stdout

		select(maxfdp1, &rset, &wset, NULL, NULL);

		if (FD_ISSET(STDIN_FILENO, &rset)) {
			if ((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
				if(errno!=EWOULDBLOCK)
					err_sys("read error on stdin");
			} else if (n == 0) {
				fprintf(stderr, "%s: EOF on stdin\n", gf_time());
				stdineof = 1; // all done with stdin
				if (tooptr == toiptr)
					shutdown(sockfd, SHUT_WR); // send FIN
			} else {
				fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
				toiptr += n; // just read
				FD_SET(sockfd, &wset); // try and write to socket below
			}
		}

		if (FD_ISSET(sockfd, &rset)) {
			if ((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("read error on socket");
			} else if (n == 0) {
				fprintf(stderr, "%s, EOF on socket\n", gf_time());
				if (stdineof)
					return; // normal termination
				else
					err_quit("str_cli: server terminated prematurely");
			} else {
				fprintf(stderr, "%s: read %d bytes from socket\n", gf_time(),
						n);
				friptr += n; // just read
				FD_SET(STDOUT_FILENO, &wset); // try and write below
			}

		}

		if (FD_ISSET(STDOUT_FILENO, &wset) && ((n = friptr - froptr) > 0)) {
			if ((nwritten = write(STDOUT_FILENO, froptr, n)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("write error to stdout");
			} else {
				fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(),
						nwritten);
				froptr += nwritten; // just written
				if (froptr == friptr)
					froptr = friptr = fr; // back to begining of buffer
			}

		}

		if (FD_ISSET(sockfd, &wset) && ((n = toiptr - tooptr) > 0)) {
			if ((nwritten = write(sockfd, tooptr, n)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("write error to socket");
			} else {
				fprintf(stderr, "%s: wrote %d bytes to socket\n", gf_time(),
						nwritten);
				tooptr += nwritten; // just written
				if (tooptr == toiptr) {
					toiptr = tooptr = to; //back to beginning of buffer
					if (stdineof)
						shutdown(sockfd, SHUT_WR); // send FIN
				}
			}
		}
	}
}

void str_cli(FILE* fp, int sockfd) {
	int maxfdp1;
	fd_set rset;
	char sendline[MAXLINE], recvline[MAXLINE];

	FD_ZERO(&rset);
	for (; ; ) {
		FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) { // Socket is readable
			if(Readline(sockfd, recvline, MAXLINE) ==0) {
				//fprint("str_cli: server terminated prematurely");
				exit(-1);
			}
			fputs(recvline, stdout);
		}
		if (FD_ISSET(fileno(fp), &rset)) { // input is readable
			if(fgets(sendline, MAXLINE, fp) == NULL)
				return; // all done
			Written(sockfd, sendline, strlen(sendline));
		}
	}
}

#endif /* SOCKETS_H_ */
