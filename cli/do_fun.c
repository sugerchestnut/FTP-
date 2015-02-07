#define DO_FUN_C

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<grp.h>
#include<pwd.h>
#include<dirent.h>
#include<time.h>
#include<limits.h>
#include"do_fun.h"

#define         FILE_NAME_MAXSIZE 200
#define         CWDMAXSIZE      50//存放目录的最大容量

#define 		PARAM_NONE 		0
#define         PARAM_A         1
#define         PARAM_L         2

void my_err(const char *err_string,int line)//错误处理函数
{
        fprintf(stderr,"line:%d",line);
        perror(err_string);
        exit(1);
}

void do_put(int connect_socket)
{
		int alive1=1;//代表上传的客户端文件是存在的
		int alive2=1;//代表上传的服务器端文件是不存在的
		int flag=0;//用来判断是否需要切换目录
		char pathname[40];
        bzero(pathname,sizeof(pathname));
        printf("输入上传文件所在的位置:");
        scanf("%s",pathname);
        
		char file_name[FILE_NAME_MAXSIZE];//文件名
		bzero(file_name,FILE_NAME_MAXSIZE);
		printf("输入要上传的客户端的文件名:");
        scanf("%s",file_name);
        
        char buffer[BUFFER_SIZE];//缓冲区
        bzero(buffer,BUFFER_SIZE);        
        strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));//将file_name里面的文件名拷贝到buffer中
        
        
        //向服务器发送buffer中的数据,即文件名
        if(send(connect_socket,buffer,BUFFER_SIZE,0)<0)
        {
                my_err("send file_name",__LINE__);
                exit(1);
        }
        
        char cwd[40];//存放当前目录
        if(getcwd(cwd,CWDMAXSIZE)<0){
                my_err("getcwd",__LINE__);
                exit(1);
        }
        printf("当前工作目录:%s\n",cwd);
        printf("存放文件的工作目录:%s\n",pathname);
        
		if((strcmp(pathname,cwd))!=0)//不在同一个目录下
		{
			flag=1;
			if((chdir(pathname))<0){//切换到即将上传的文件的目录下
                	my_err("chdir",__LINE__);
                	exit(1);
        	}
        }
        
        int fd;
        if((fd=open(file_name,O_RDONLY))<0)//以只读的方式打开文件
        {
        		alive1=0;
                printf("该文件不存在，无法上传！\n");                
        }
        send(connect_socket,&alive1,sizeof(alive1),0);//即将上传的文件客户端的情况
        recv(connect_socket,&alive2,sizeof(alive2),0);//即将上传的文件客户端
        
        if(alive1 && alive2){
        		printf("文件存在，可以上传!\n");
        		
                int length=0;
                
                //length=read(fd,buffer,BUFFER_SIZE);
                //printf("%s\n\n\n",buffer);
                //length=0;
                bzero(buffer,BUFFER_SIZE);
                while((length=read(fd,buffer,BUFFER_SIZE))>0){//从文件中读取数据
                		if((send(connect_socket,buffer,length,0))<0){//发送数据
                      		my_err("send",__LINE__);
                      		break;
                		}
                	bzero(buffer,BUFFER_SIZE);//每传输一次需要将buffer清零
       			}
       			close(fd);//文件传输完毕，关闭文件
                printf("Send file:%s successful!\n",file_name);
                if(flag==1){
                		chdir(cwd);//返回到之前的工作目录
                }                
        }else{
        		return;//若文件不存在，则此次调用返回
        }   
}

void do_get(int connect_socket)
{
		int alive1=1;//判断即将下载的文件客户端不存在
		int alive2=1;//判断即将下载的文件服务器端存在
		int flag=0;//用来判断当前目录与保存文件目录的一致性
      //输入文件名，并放入到缓冲区buffer中等待发送
        char file_name[FILE_NAME_MAXSIZE];
        bzero(file_name,FILE_NAME_MAXSIZE);//清零操作
        printf("输入要下载的服务器端的文件名:");
        scanf("%s",file_name);
        
        char pathname[40];
        bzero(pathname,sizeof(pathname));
        printf("输入文件要保存到的地方:");
        scanf("%s",pathname);
        //printf("当前工作目录的长度:%ld\n",sizeof(pathname));
        
		char buffer[BUFFER_SIZE];
        bzero(buffer,FILE_NAME_MAXSIZE);        
        strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));
        	
        //向服务器发送buffer中的数据,即将要下载的文件名
        if(send(connect_socket,buffer,BUFFER_SIZE,0)<0){
                my_err("send file_name",__LINE__);
                exit(1);
        }
	
        char cwd[40];//存放当前目录
        if(getcwd(cwd,CWDMAXSIZE)<0){
                my_err("getcwd",__LINE__);
                exit(1);
        }
        
		if((strcmp(pathname,cwd))!=0){	
				flag=1;
			if((chdir(pathname))<0){           //切换到写文件的目录下
                my_err("chdir",__LINE__);
                exit(1);
        	}
    	}
        //打开文件，准备写入
        int fd;
        int fd1;
        if(fd1=open(file_name,O_RDWR)>0)
        {
                printf("该文件已存在，不需要下载\n");
                alive1=0;
                //exit(1);
        }
        
        send(connect_socket,&alive1,sizeof(alive1),0);//若要下载的文件客户端存在
		recv(connect_socket,&alive2,sizeof(alive2),0);//即将下载的文件服务器端没有

		if(alive1 && alive2){
        //if((fd=open(file_name,O_CREAT | O_EXCL,S_IRUSR | S_IWUSR))==-1){
          	if((fd=creat(file_name,S_IRWXU))==-1){
                	my_err("creat",__LINE__);
               		exit(1);
       	  	}
		
		
        	//从服务器接受数据到buffer中，每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
        	bzero(buffer,BUFFER_SIZE);
        	int length=0;
       		while((length=recv(connect_socket,buffer,BUFFER_SIZE,0))>0){
        			if((write(fd,buffer,length))<length){
                    	printf("File:\t%s write failed\n",file_name);
                    	break;
                    }
                	bzero(buffer,BUFFER_SIZE);
        	}

        	//接收成功后，关闭文件，关闭socket
        	printf("Receeived file:%s from server IP successful!\n",file_name);
        	close(fd);
        	if(flag==1){
        		chdir(cwd);//返回到之前的工作目录
        	}
        }else{
        		return;
        }
}

void display_attri(struct stat buf,char * name)
{
        char			 buf_time[100];
        struct passwd 	*psd;
        struct group 	*grp;

        /* 获取并打印文件属性 */
        if(S_ISLNK(buf.st_mode)){
                printf("l");
        }else if(S_ISREG(buf.st_mode)){
                printf("-");
        }else if(S_ISDIR(buf.st_mode)){
                printf("d");
        }else if(S_ISCHR(buf.st_mode)){
                printf("c");
        }else if(S_ISBLK(buf.st_mode)){
                printf("b");
        }else if(S_ISFIFO(buf.st_mode)){
                printf("f");
        }else if(S_ISSOCK(buf.st_mode)){
                printf("s");
        }

        /* 获取并打印文件所有者的权限 */
        if(buf.st_mode & S_IRUSR){
                printf("r");
        }else{
                printf("-");
        }
        
        if(buf.st_mode & S_IWUSR){
                printf("w");
        }else{
                printf("-");
        }

        if(buf.st_mode & S_IXUSR){
                printf("x");
        }else{
                printf("-");
        }

        /* 获取并打印与文件所有者同组的用户对该文件的操作权限 */
        if(buf.st_mode & S_IRGRP){
                printf("r");
        }else{
                printf("-");
        }

        if(buf.st_mode & S_IWGRP){
                printf("w");
        }else{
                printf("-");
        }

        if(buf.st_mode & S_IXGRP){
                printf("x");
        }else{
                printf("-");
        }
        
        //获取并打印其他用户对文件的操作权限
        if(buf.st_mode & S_IROTH){
                printf("r");
        }else{
                printf("-");
        }
        if(buf.st_mode & S_IWOTH){
                printf("w");
        }else{
                printf("-");
        }
        if(buf.st_mode & S_IXOTH){
                printf("x");
        }else{
                printf("-");
        }

        printf("    ");

        //根据uid和gid获取文件所有者的用户名和组名
        psd=getpwuid(buf.st_uid);
        grp=getgrgid(buf.st_gid);
        printf("%4ld",buf.st_nlink);//打印文件的连接数
        printf("%-8s",psd->pw_name);//psd指向
        printf("%-8s",grp->gr_name);

        printf("%6ld",buf.st_size);
        strcpy(buf_time,ctime(&buf.st_mtime));
        buf_time[strlen(buf_time)-1]='\0';//去掉换行符(哪来的换行符？？？)
        printf(" %s",buf_time);//打印文件的时间信息
        printf("%s",name);
}

void do_ls(int connect_socket)
{
	int 	i,j;
        char 			param[3][100];
        int 			count;
        int 			flag;
        char  			singal[100];
        //char  			attri[60][100];
        struct stat  	buf1;
        //char    		name[30];
		char 			*temp;
		int 			n;
		
        printf("请输入你的ls命令[ls] [选项] [目录]:");
        for(i=0;i<3;i++){
               scanf("%s",param[i]);
        }
        
		//for(i=0;i<3;i++){
          //   printf("\t\t\t%s\n",param[i]);
        //}
        
        for(i=0;i<3;i++){
        		//temp=param[i];
                //send(connect_socket,temp,sizeof(temp),0);//发送客户端录入的命令
                send(connect_socket,param[i],sizeof(param[i]),0);
        }
        
		//for(i=0;i<3;i++){
            // printf("%s\n",param[i]);
        //}
        //printf("\n");
        recv(connect_socket,&flag,sizeof(flag),0);//接收服务器端分析出来的-a  -l的取值
        recv(connect_socket,&count,sizeof(count),0);//目录下文件总数
        printf("\n%d  %d\n\n",flag,count);

        for(i=0 ; i < count ; i++){
                if(flag == PARAM_A){
                		recv(connect_socket,&n,sizeof(n),0);
                        recv(connect_socket,singal,sizeof(singal),0);//-a选项时每个文件的文件名
                        singal[n]='\0';
                        printf("%s\n",singal);
                        //break;
                }else if(flag == PARAM_L){
                		recv(connect_socket,&n,sizeof(n),0);
                        recv(connect_socket,&buf1,sizeof(buf1),0);
                        recv(connect_socket,singal,sizeof(singal),0);
                        singal[n]='\0';
                        display_attri(buf1,singal);                        
                        printf("\n");
                        //break;
                }else if(flag == PARAM_A + PARAM_L){
                		recv(connect_socket,&n,sizeof(n),0);
                        recv(connect_socket,&buf1,sizeof(buf1),0);
                        recv(connect_socket,singal,sizeof(singal),0);
                        singal[n]='\0';
                        display_attri(buf1,singal);
                        printf("\n");
                         //break;
                }else{
                        break;
                }
        bzero(singal,sizeof(singal));
      	bzero(&buf1,sizeof(buf1));
      	//bzero(name,sizeof(name));
        }     
}

void do_cd(int connect_socket)
{
	char move[2][30];
	int i,j;
	char result[30];
	int 	flag=1;
	printf("请输入cd命令 [cd] [path]:");
		for(i=0;i<2;i++)
			scanf("%s",move[i]);
	
			send(connect_socket,move[1],sizeof(move[1]),0);		
			recv(connect_socket,result,sizeof(result),0);	
			recv(connect_socket,&flag,sizeof(flag),0);
			if(flag==1){
				printf("服务器端当前工作目录:%s\n",result);
			}else{
				printf("修改目录有误!\n");
				exit(1);
			}
			
}
