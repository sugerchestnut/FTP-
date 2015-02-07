/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  服务器端程序
 *
 *        Version:  1.0
 *        Created:  2015年01月31日 12时40分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  潘璐 (fgm), mehner.fritz@fh-swf.de
 *   Organization:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h>
#include <signal.h>
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include"do_fun.h"

#define         SERV_PORT       	1111
#define         LISTEN_QUEEN    	20
#define         BUFFER_SIZE     	1024  //1KB
#define         FILE_NAME_MAXSIZE 	200

#define 		g_maxlen            0  		//记录某一目录下最长的文件名

void demand(int connect_socket)
{
	char 			choice[10];
	int 		        ret;//标志接收的字节数
		
        if((ret=recv(connect_socket,choice,sizeof(choice),0))<0){//接收客户端传来的请求
		my_err("receive choice",__LINE__);
		exit(1);
	}
	choice[ret-1]='\0';
					
	if((strcmp(choice,"get"))==0){//下载文件的请求
	        do_get(connect_socket);						
    }else if((strcmp(choice,"put"))==0){//上传文件的请求		                
                do_put(connect_socket);	
    }else if((strcmp(choice,"ls"))==0){
    		do_ls(connect_socket);
    }else if((strcmp(choice,"cd"))==0){
    	do_cd(connect_socket);
    }else if((strcmp(choice,"end"))==0){
        	return;
    }

        close(connect_socket);//关闭连接的套接字
        bzero(choice,sizeof(choice));
        exit(0);//结束子进程
}

int main()
{
        int                     server_socket,connect_socket;//服务器端创建的套接字和建立联接后的套接字
        int                     optval;//设置套接字属性
        struct sockaddr_in      client_addr,server_addr;		
	pid_t                   pid;	

        server_socket=socket(AF_INET,SOCK_STREAM,0);//创建一个套接字
        if(server_socket<0){
                my_err("socket",__LINE__);
        }

        optval=1;//设置套接字，使之可以重新绑定端口
        if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(int))<0){
                my_err("setsockopt",__LINE__);
        }

        memset(&server_addr,0,sizeof(struct sockaddr_in));//初始化服务器端地址结构
        server_addr.sin_family=AF_INET;
        server_addr.sin_port=htons(SERV_PORT);
        server_addr.sin_addr.s_addr=htonl(INADDR_ANY);//可以绑定到任何网络接口上
        if(bind(server_socket,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in))<0){
                //将套接字绑定到本地端口上
                my_err("bind",__LINE__);
        }

        if(listen(server_socket,LISTEN_QUEEN)<0){//将套接字转换为监听套接字
                my_err("listen",__LINE__);
        }

        //cli_len=sizeof(struct sockaddr_in);
	
		
		signal(SIGCHLD,SIG_IGN); //忽略子进程退出的消息，避免僵尸进程
        while(1){//服务器端要一直运行
                socklen_t  client_length=sizeof(client_addr);//客户端套接字地址长度

/* 接收连接请求，返回一个新的socket，这个新的socket用于同连接的客户端通信，accept函数会把连接到的客户端信息写入到client_addr中 */
                connect_socket=accept(server_socket,(struct sockaddr*) & client_addr, & client_length);
                	
                if(connect_socket<0){
                        my_err(" accept",__LINE__);
                        exit(1);
                }
                	
                printf("接收到一个新客户请求,IP:%s\n",inet_ntoa(client_addr.sin_addr));
                
                pid=fork();
                if(!pid){
                        demand(connect_socket);
                }else{//父进程关闭刚刚接收的连接请求，执行accept等待其他连接请求
                	close(connect_socket);
                }
        }
        close(server_socket);//关闭监听的套接字
}
