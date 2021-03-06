#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include "mylib.h"

int
main(int argc, char *argv[])
{
	int listenfd;
	struct sockaddr_in servaddr;

/******************************************************/
	if( argc != 1 ){
		printf("Format:%s\n",argv[0]);
		exit(-1);
	}
/******************************************************/
	if( (listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ){
		printf("socket error\n");
		exit(-1);
	}
	int optval = 1; //open the opt 
	if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR 
#ifdef SO_REUSEPORT	
			| SO_REUSEPORT
#endif
			, &optval, sizeof(optval)) != 0){
		printf("setsockopt error\n");
		exit(-1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
		printf("bind error\n");
		perror("bind error");
		exit(-1);
	}

	if( listen(listenfd, 5) < 0){
		printf("listen error\n");
		exit(-1);
	}
//	sigaction(SIGCHLD, SIGCHLD_Handler);
/***************************I/O复用（select）***************************/
	int i, maxfd, nready, connfd, maxi, r_byte, client[FD_SETSIZE];  //FD_SETSIZE//select.h / types.h
	fd_set readset;
	struct timeval timeout;
	char recvdata[MAXBUFFSIZE];
	
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;

	maxfd = listenfd + 1;//
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; ++i){
		client[i] = -1;
	}
	FD_ZERO(&readset);
	
	for(;;){
		printf("into for(;;);\n");
		FD_SET(listenfd, &readset);
		if( (nready = select(maxfd, &readset, NULL, NULL ,&timeout) ) <= 0){			
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
		printf("select return\n");
		if(FD_ISSET(listenfd, &readset) == 1 ){//listenfd are ready
			if( (connfd = accept(listenfd, NULL, NULL)) < 0 ){//因为listenfd就绪，所以不会阻塞
				if( EINTR == errno )
					continue;
				else{
					printf("accept error\n");  //error
					exit(-1);	
				}			
			}
			printf("accept successful\n");
			//将不同客户的已连接fd放到client[]
			for( i = 0; i < FD_SETSIZE; ++i){
				if( client[i] < 0 ){//初始化client[] = -1，当-1时表示这个位还没被使用
					client[i] = connfd;
					break;
				}
			}//save i
			//
			if( i == FD_SETSIZE ){
				printf("client[] have not space to use\n");
				exit(-1);
			}
			FD_SET(connfd, &readset);
			//更新maxfd
			printf("maxfd = %d   connfd = %d\n",maxfd,connfd);
			maxfd = (maxfd > connfd)?maxfd:(connfd+1);
			//更新maxi（client当前可使用的最大下标）
			maxi = (maxi > i)?maxi:i;
			printf("nready = %d\n",(int)nready);
			if( --nready <= 0 ){
				continue;//就绪描述符用完，返回select				
			}
		}
		printf("into two for(;;);\n");
		//依据maxi，遍历所有客户是否有数据发来
		for(i = 0; i <= maxi; ++i){
			if( client[i] < 0 ){//当处理完一个客户端的请求就把那个客户链接关闭，并从client中移除并初始化为-1
				continue;
			}
			else{//该客户仍在
				printf("start check the client\n");
				if( FD_ISSET(client[i],&readset) == 1 ){
					//printf("accept return\n");
					if( (r_byte = Read(client[i], recvdata, MAXBUFFSIZE)) < 0){
						printf("read error\n");
						exit(-1);  //Read里面只做调用、检查和返回，并不quit the process
					}else{
						if( r_byte == 0 ){//client terminal close这里必须做处理
							close(client[i]);
							FD_CLR(client[i], &readset);
							client[i] = -1;
						}	
						
						//printf("read successful\n");
						if( Write(client[i], recvdata, strlen(recvdata)) < 0){
							printf("read error\n");
							exit(-1);  //Read里面只做调用、检查和返回，并不quit the process
						}
						//printf("write return\n");			
					}
						
					if( --nready <= 0 ){
						break;  //break "for" to select
					}
				}
			}
		}
	
	}
	
	exit(0);	
}

