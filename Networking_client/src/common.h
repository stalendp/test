/*
 * common.h
 *
 *  Created on: Oct 14, 2013
 *      Author: stalendp
 */

#ifndef COMMON_H_
#define COMMON_H_

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
#include	<errno.h>		/* for definition of errno */
#include	<stdarg.h>		/* ANSI C header file */

#define MAXLINE 1024
#define SERV_PORT 55555
#define LISTENQ 30

int max(int l, int r) {
	return l>r ? l : r;
}

ssize_t Readline(int fd, void* vptr, size_t maxlen) {
	ssize_t n, rc;
	char c, *ptr;

	ptr = vptr;
	for(n=1; n<maxlen; n++) {
	again:
		if( (rc=read(fd, &c, 1))==1) {
			*ptr++ = c;
			if(c=='\n')
				break;  // newline is stored, like fgets()
		} else if (rc==0) {
			*ptr = 0;
			return (n-1);  // EOF, n-1 bytes were read
		} else {
			if(errno == EINTR)
				goto again;
			return -1;  // error, errno set by read()
		}
	}
	*ptr = 0;  // null terminate like fgets()

	return (n);
}

ssize_t readn(int fd, void* vptr, size_t n) {
	size_t nleft = n;
	ssize_t nread = 0;
	char* ptr = vptr;

	while(nleft>0) {
		if((nread=read(fd, ptr, nleft))<0) {
			if(errno == EINTR)
				nread = 0; // and call read() again
			else
				return -1;
		} else if (nread == 0) {
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return n-nleft;
}

ssize_t Written(int fd, const void* vptr, size_t n) {
	size_t nleft = n;
	ssize_t nwritten = 0;
	const char* ptr = vptr;

	while(nleft>0) {
		if ((nwritten = write(fd, ptr, nleft))<=0) {
			if(nwritten<0 && errno == EINTR)
				nwritten = 0; // and call write() again
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}


static void	err_doit(int, const char *, va_list);

char	*pname = NULL;		/* caller can set this from argv[0] */

/* Nonfatal error related to a system call.
 * Print a message and return. */

void
/* $f err_ret $ */
err_ret(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, fmt, ap);
	va_end(ap);
	return;
}

/* Fatal error related to a system call.
 * Print a message and terminate. */

void
/* $f err_sys $ */
err_sys(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */

void
/* $f err_dump $ */
err_dump(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, fmt, ap);
	va_end(ap);
	abort();		/* dump core and terminate */
	exit(1);		/* shouldn't get here */
}

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */

void
/* $f err_msg $ */
err_msg(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, fmt, ap);
	va_end(ap);
	return;
}

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */

void
/* $f err_quit $ */
err_quit(const char *fmt, ...)
{
	va_list		ap;

	va_start(ap, fmt);
	err_doit(0, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Print a message and return to caller.
 * Caller specifies "errnoflag". */

static void
err_doit(int errnoflag, const char *fmt, va_list ap)
{
	int		errno_save;
	char	buf[MAXLINE];

	errno_save = errno;		/* value caller might want printed */
	vsprintf(buf, fmt, ap);
	if (errnoflag)
		sprintf(buf+strlen(buf), ": %s", strerror(errno_save));
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(stderr);		/* SunOS 4.1.* doesn't grok NULL argument */
	return;
}


char* gf_time(void) {
	struct timeval	tv;
	time_t			t;
	static char		str[30];
	char			*ptr;

	if (gettimeofday(&tv, NULL) < 0)
		err_sys("gettimeofday error");

	t = tv.tv_sec;	/* POSIX says tv.tv_sec is time_t; some BSDs don't agree. */
	ptr = ctime(&t);
//	strcpy(str, &ptr[11]);
		/* Fri Sep 13 00:00:00 1986\n\0 */
		/* 0123456789012345678901234 5  */
//	snprintf(str+8, sizeof(str)-8, ".%06ld", tv.tv_usec);

	return("time");
}




#endif /* COMMON_H_ */
