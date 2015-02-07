/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  客户端的程序
 *
 *        Version:  1.0
 *        Created:  2015年01月31日 14时12分12秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  潘璐 (fgm), mehner.fritz@fh-swf.de
 *   Organization:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include"do_fun.h"

#define         SERVER_PORT  1111
#define         BUFFER_SIZE  1024//1KB
#define         FILE_NAME_MAXSIZE 200
#define         CWDMAXSIZE      50//存放目录的最大容量

int main(int argc , char **argv)
{
        int i;
        int connect_socket;//连接的套接字
        struct sockaddr_in server_addr;
        
        int server_port;

        memset(&server_addr,0,sizeof(struct sockaddr_in));
        server_addr.sin_family=AF_INET;

        //从命令行获取服务器端的端口和IP地址
        for(i=1 ; i<argc ; i++){
                if(strcmp("-p",argv[i]) == 0){
                        server_port=atoi(argv[i+1]);
                        if(server_port < 0 || server_port > 65535){
                                printf("输入的服务器端口不正确！\n");
                                exit(1);
                        }else{
                                server_addr.sin_port=htons(server_port);
                        }
                        continue;
                }
                if(strcmp("-a",argv[i]) == 0){
                        if(inet_aton(argv[i+1],&server_addr.sin_addr)==0){
                                printf("输入的服务器IP不正确！\n");
                                exit(1);
                        }
                        continue;
                }
        }

        //检测是否少输入了某项参数
        if(server_addr.sin_port == 0 || server_addr.sin_addr.s_addr == 0){
                printf("格式: [-p] [server_addr.sin_port] [-a] [server_IP]");
                exit(1);
        }
        
        char choice[10];
        printf("请选择:put(上传文件)   get(下载文件)   cd(改变目录)  ls(-a -l -al)(查询目录)   end(结束):");
        scanf("%s",choice);
        //创建一个socket



        //gets(choice);
        while((strcmp(choice,"end"))!=0)
        {
        	connect_socket=socket(AF_INET,SOCK_STREAM,0);
        	if(connect_socket<0){
                my_err("socket",__LINE__);
       		}

        	//向服务器端发送链接请求,连接成功后connect_socket代表了客户端与服务器端的一个socket连接
        	if(connect(connect_socket,(struct sockaddr*)&server_addr,sizeof(struct sockaddr_in))<0){//有问题
                		my_err("connect",__LINE__);
        	}
        	
        	if(send(connect_socket,choice,sizeof(choice),0)<0){
                	my_err("send choice",__LINE__);
                	exit(1);
        	}
        	
        	printf("\n%ld\n",sizeof(choice));
        	if((strcmp(choice,"get"))==0){
        	   		do_get(connect_socket);
			}else if((strcmp(choice,"put"))==0){
			   		do_put(connect_socket);		
			}else if((strcmp(choice,"ls"))==0){
					do_ls(connect_socket);
			}else if((strcmp(choice,"cd"))==0){
					do_cd(connect_socket);
			}
			
			close(connect_socket);//关闭连接的套接字
			
			bzero(choice,sizeof(choice));//清除刚才的选择记录
			printf("请选择:put(上传文件)   get(下载文件)   cd(改变目录)  ls(-a -l -al)(查询目录)   end(结束):");
        	scanf("%s",choice);
		//gets(choice);
		}
		
}
