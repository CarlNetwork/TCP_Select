#include <unistd.h>
#include <errno.h>
#include "mylib.h"
#include <stdio.h>

int 
Write(int sockfd, const void *buf, size_t n)
{
	ssize_t w_byte;
	size_t nleft;
	const char *ptr;
	int save_errno;

	save_errno = errno;
	ptr = buf;	
	nleft = n;	
	while( nleft > 0 ){
		w_byte = write(sockfd, ptr, nleft);
		//printf("w_byte = %d\n",(int)w_byte);
		if( w_byte <= 0 ){
			if( (w_byte < 0) && (errno == EINTR) ){
				continue;  //w_byte = 0;
			}else
				return (-1);
		}
		nleft -= w_byte;
		ptr += w_byte;
	}
	errno = save_errno;

	return n;	
}

int 
Read(int sockfd, void *buf, size_t n)
{
	ssize_t r_byte;
	size_t nleft;
	char *ptr;
	int save_errno;

	save_errno = errno;
	errno = 0;
	ptr = buf;
	nleft = n;
	printf("1\n");
	while( nleft > 0 ){
		r_byte = read(sockfd, ptr, nleft);
		//printf("2   r_byte = %d\n",(int)r_byte);
		if( r_byte <= 0){
			if( r_byte == 0 )
				break;
			else
				if( (r_byte < 0) && (errno == EINTR) )
					continue;
				else{
					return -1;
				}
		}
		
		nleft -= r_byte;
		ptr += r_byte;
		if( nleft < n )
			break;
	}
	errno = save_errno;
	//printf("read %d byte\n",(int)(n-nleft));
	return (n-nleft);
}
