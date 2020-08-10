/*================================================================
*   Copyright (C) 2020 hqyj Ltd. All rights reserved.
*   
*   文件名称：server.c
*   创 建 者：Chens
*   创建日期：2020年04月23日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "msg.h"

#define  SERV_PORT    "6666"
#define  SERV_IP_ADDR "192.168.1.106"
#define  QUIT         "quit"

#define BACKLOG       10


int main(int argc, char *argv[])
{
	/*创建套接字*/
	int fd = -1;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd <0){
		perror("socket");
		exit(1);
	}
	puts("----创 建 套 接 字 成 功----");

	//地址快速重用
	int b_reuse = 1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int)) <0){
		perror("setsockopt");
		exit(1);
	}
	puts("----设置地址快速重用成功----");

	//创建结构体
	struct sockaddr_in sin;
	bzero(&sin,sizeof(sin));
	//填充结构体
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons(atoi(SERV_PORT));
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	puts("----创建设置 结构体 成功----");

	/*绑定本机（服务器）IP和端口*/
	if( bind(fd,(struct sockaddr*)&sin,sizeof(sin)) <0){
		perror("bind");
		exit(1);
	}
	puts("----IP端口绑定成功-----");

	/*把主动套接字变为被动套接字*/
	/*监听客户端链接*/
	if(listen(fd,BACKLOG) <0){
		perror("listen");
		exit(1);
	}
	puts("----开  始  监  听----");

	/*等待接收连接请求*/
	int newfd = -1;
	//newfd = accept(fd,NULL,NULL);
	//如果不关心谁连接了，那么填写NULL
	
	//关心是谁链接了，创建结构体存放已建立链接客户端信息
	struct sockaddr_in cin;
	bzero(&cin,sizeof(cin));
	socklen_t addrlen = sizeof(cin);
	newfd = accept(fd,(struct sockaddr*)&cin,&addrlen);
	if(newfd < 0){
		perror("accept");
		exit(1);
	}
	puts("----接受客户端成功----");
	//打印建立链接的客户端
	//取出客户端ip
	char ipv4_addr[16]={};
	if(!inet_ntop(AF_INET,(void*)&cin.sin_addr.s_addr,ipv4_addr,sizeof(cin))){
		perror("inet_ntop");
		exit(1);
	}
	printf("++客户端(%s:%d)已连接！++\n",ipv4_addr,ntohs(cin.sin_port));

	/*读写*/
	int ret =-1;
	int key = 1;
	while(1){
		do{
			ret = recv(newfd,&msg,sizeof(msg)-1,0);
		}while(ret<0 && EINTR==errno);
		if(ret<0){
			perror("recv");
			exit(1);
		}
		if(!ret)break;
		
		printf("1.name:%s\n",    msg.name    );
		printf("2.age:%d\n",     msg.age     );
		printf("3.sex:%c\n",     msg.sex     );
		printf("4.password:%s\n",msg.password);
		printf("5.number:%d\n",  msg.number  );
		printf("6.salary:%d\n",  msg.salary  );
		printf("7.dept:%s\n",    msg.dept    );

		printf("%d\n",msg.cmd);
		
		if(key == 1){
			msg.status = OK;
		}else{
			msg.status = NO;
		}
		key = ~key;
        send(newfd,&msg,sizeof(msg),0);
		
	}
	
	shutdown(fd,SHUT_RDWR);//相当于close(fd)
	shutdown(newfd,2);//相当于close(fd)

	return 0;
}

