#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <math.h>  //max
#include "mylib.h"



int
main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in servaddr;
/******************************************************************/
	if( argc != 2 ){
		printf("you can:%s --help\n",argv[0]);
		exit(-1);
	}else{
		if( strcmp(argv[1],"--help") == 0 ){
			printf("you can:%s destaddr\n",argv[0]);
			exit(0);	
		}
	}

/******************************************************************/
	if( (sockfd = socket(PF_INET, SOCK_STREAM, 0) ) < 0 ){
		printf("socket error\n");
		exit(0);
	}
	int optval = 1;//open
	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR 
#ifdef SO_REUSEPORT
		| SO_REUSEPORT
#endif
			, &optval, sizeof(optval)) != 0 ){
		printf("setsockopt error\n");
		exit(0);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5000);
	if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr) < 0 ){
		printf("error\n");
		exit(0);
	}
	
	if( connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
		perror("connect");
		exit(0);
	}
	printf("connect successful\n");	
/******************************************************************/
	int maxfd, nready, r_byte;
	fd_set readset;
	FILE *fp = stdin;
	//struct timeval timeout;
	char recvdata[MAXBUFFSIZE], senddata[MAXBUFFSIZE];
	if( sockfd < fileno(fp) )
		maxfd = fileno(fp) + 1;
	else
		maxfd = sockfd + 1;
	//maxfd = max(sockfd, fileno(fp) ) + 1;//
	FD_ZERO(&readset);
	
	//timeout.tv_sec = 30;
	//timeout.tv_usec = 0;

	for(;;){
		FD_SET(sockfd, &readset);
		FD_SET(fileno(fp), &readset);
		if( (nready = select(maxfd, &readset, NULL, NULL ,NULL) ) <= 0){			
			if( nready == 0 ){//timeout
				printf("select timeout\n");
				exit(0);
			}else{//error or EINTR
				if( errno == EINTR)   //EINTR
					continue;
				printf("select error\n");  //error
				exit(-1);	
			}
		}
		//printf("nready = %d\n",nready);
		if( FD_ISSET(fileno(fp), &readset) == 1 ){//stdin are ready
			//printf("stdin are ready\n");
			if( Read(fileno(fp), senddata, MAXBUFFSIZE) <= 0){
				//if(  )
				printf("read error\n");
				exit(-1);  //Read里面只做调用、检查和返回，并不quit the process
			}
			//printf("read from stdin successful\n");
			if( Write(sockfd, senddata, strlen(senddata)) < 0){
				printf("read error\n");
				exit(-1);  
			}
			//printf("write to serv successful\n");
			if( --nready <= 0 ){//没有就绪描述符可用
				//printf("nready = %d\n",nready);
				continue;
			}			
		}
		
		if(FD_ISSET(sockfd, &readset) == 1 ){//sockfd are ready
			//printf("sockfd are ready\n");
			if( (r_byte = Read(sockfd, recvdata, MAXBUFFSIZE)) < 0){
				printf("read error\n");
				exit(-1);  //Read里面只做调用、检查和返回，并不quit the process
			}else{
				if(r_byte == 0){//serv terminal close这里必须做处理，否则客户端会持续输出来自上一次的数据
					printf("serv terminal close\n");
					exit(0);					
				}	
			
			}
			//printf("read from serv successful\n");
			if( Write(fileno(stdout), recvdata, strlen(recvdata)) < 0){
				printf("read error\n");
				exit(-1);  //Read里面只做调用、检查和返回，并不quit the process
			}
			//printf("write to stdout successful\n");
			if( --nready <= 0 )//没有就绪描述符可用
				continue;	
		}
	
	}
	exit(0);	
	
}
