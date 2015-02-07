#define _DO_FUN_C

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h>
#include<dirent.h>
#include"do_fun.h"

#define         BUFFER_SIZE     	1024  //1KB
#define         FILE_NAME_MAXSIZE 	200
//#define         PARAM_NONE              0
#define         PARAM_NONE              0
#define         PARAM_A                 1       //-a
#define         PARAM_L                 2       //-l
#define         NAMEMAX                 50
#define 		PATH_MAXNUM            260

int          maxlen=0;

void my_err(const char *err_string,int line)//错误处理函数
{
        fprintf(stderr,"line:%d",line);
        perror(err_string);
        exit(1);
}

void do_put(int connect_socket)
{
		int alive1=1;//即将上传的文件客户端存在
		int alive2=1;//即将上传的文件服务器端不存在
        struct flock    lock;//锁的结构体
	               //bzero(choice,sizeof(choice))
	char buffer[BUFFER_SIZE];
        bzero(buffer,BUFFER_SIZE);  //缓冲区清零  
        if(recv(connect_socket,buffer,BUFFER_SIZE,0)<0){//接收到文件名，并放入到缓冲区buffer中
                   my_err("recv",__LINE__);
                   exit(1);
        }    
                		
                		//printf("%s",buffer);
        			//strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE ? BUFFER_SIZE : strlen(file_name));
        	        		
        char file_name[FILE_NAME_MAXSIZE];
        bzero(file_name,FILE_NAME_MAXSIZE);//清零操作
        strncpy(file_name,buffer,strlen(buffer)>FILE_NAME_MAXSIZE ? FILE_NAME_MAXSIZE : strlen(buffer));
                		//将buffer中的文件名拷贝到file_name中             		
        				//打开文件，准备写入服务器端文件夹中
        int fd;
        int fd1;
        if((fd1=open(file_name,O_RDWR))>0)
        {
             printf("服务器端该文件已存在，无法上传!\n");
             alive2=0;
            // exit(1);
             close(fd1);
        }
        recv(connect_socket,&alive1,sizeof(alive1),0);//即将上传的文件客户端没有 
        send(connect_socket,&alive2,sizeof(alive2),0);//即将上传的文件服务器端已经有
        
        if(alive1 && alive2){
        //umask(0);//不屏蔽任何权限			
        	if((fd=creat(file_name,S_IRWXU))==-1){//以用户可读可写的方式创建
                	my_err("creat",__LINE__);
                	exit(1);
       		}else{
       				printf("\n上传%s文件\n",file_name);
       	  			printf("文件%s创建成功！\n",file_name);
       		}

        	/* 初始化锁结构 */
        	memset(&lock,0,sizeof(struct flock));
        	lock.l_start=SEEK_SET;//锁住整个文件
        	lock.l_whence=0;
        	lock.l_len=0;
        
        	int flag=0;
        	/* 设置写锁 */
        	lock.l_type=F_WRLCK;
        	if((fcntl(fd,F_GETLK,&lock))==0){//执行成功
                	if(lock.l_type==F_UNLCK){
                        	flag=1;
                	}else{//有不兼容的锁存在，打印出设置锁的进程ID
                        	if(lock.l_type==F_RDLCK){
                                	printf("不能设置锁，存在读锁:%d\n",lock.l_pid);
                        	}else if(lock.l_type==F_WRLCK){
                                	printf("不能设置锁,存在写锁:%d\n",lock.l_pid);
                        	}
                	}
        	}else{
                	printf("获取文件锁属性出错!\n");
       		}

        	if(flag==1){
                	lock.l_type=F_WRLCK;
                	if(fcntl(fd,F_SETLKW,&lock)==0){//有别的锁存在的话就等待直到那个锁被释放
                        	printf("给文件%s设置了写锁,pid:%d\n",file_name,getpid());
                	}
        	}
        				//从客户端接受数据到buffer中，每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
        	bzero(buffer,BUFFER_SIZE);
        	int length=0;
       		while((length=recv(connect_socket,buffer,BUFFER_SIZE,MSG_WAITALL))>0){
        			if((write(fd,buffer,length))<length){//写文件失败
        			   	//printf("\n%s\n",buffer);
                       	printf("File:\t%s write failed\n",file_name);
                    		break;
                	}
                	bzero(buffer,BUFFER_SIZE);
        	}
        
        	printf("Receeived file:%s from client successful!\n",file_name);
        
        	lock.l_type=F_UNLCK;
        	if((fcntl(fd,F_SETLK,&lock))==0){
                	printf("文件解锁!\n");
        	}
			close(fd); 
		}else{
			 return;
		}			
}

void do_get(int connect_socket){
//bzero(choice,sizeof(choice));
		int alive1=1;//判断即将下载的文件客户端不存在
		int alive2=1;//判断即将下载的文件服务器端存在
		struct flock    lock;//锁的结构体
        char buffer[BUFFER_SIZE];//recv接收到的数据保存在缓冲区buffer中,文件名
        bzero(buffer,BUFFER_SIZE);
        if(recv(connect_socket,buffer,BUFFER_SIZE,0)<0){
                  my_err("recv",__LINE__);
                  exit(1);
        }
                		
        char file_name[FILE_NAME_MAXSIZE];//将buffer接收到的文件名转存到file_name中
        bzero(file_name,FILE_NAME_MAXSIZE);
        strncpy(file_name,buffer,strlen(buffer) > FILE_NAME_MAXSIZE ? FILE_NAME_MAXSIZE : strlen(buffer));
                		
        printf("\n下载%s文件\n",file_name);//输出客户端从服务器端下载的文件名
                
                		/* 打开文件并且读取数据 */
        int fd;
        if((fd=open(file_name,O_RDWR))==-1){//打开文件失败
                 //my_err("open",__LINE__);
                 printf("该文件不存在!\n");
                 alive2=0;
        }
        
        recv(connect_socket,&alive1,sizeof(alive1),0);//即将下载的文件客户端已经有了
        //printf("\t\t%d\n",alive1);
        send(connect_socket,&alive2,sizeof(alive2),0);//即将下载的文件服务器端没有
        //printf("\t\t%d\n",alive2);
        if(alive1 && alive2){ 								//打开文件成功
        		        /* 初始化锁结构 */
        		memset(&lock,0,sizeof(struct flock));
        		lock.l_start=SEEK_SET;//锁住整个文件
        		lock.l_whence=0;
        		lock.l_len=0;
        		        int flag=0;
        		/* 设置读锁 */
        		lock.l_type=F_RDLCK;
        		if((fcntl(fd,F_GETLK,&lock))==0){//执行成功
                		if(lock.l_type==F_UNLCK){
                        		flag=1;
                		}else{//有不兼容的锁存在，打印出设置锁的进程ID
                        		if(lock.l_type==F_RDLCK){
                                	printf("不能设置锁，存在读锁:%d\n",lock.l_pid);
                        		}else if(lock.l_type==F_WRLCK){
                                	printf("不能设置锁,存在写锁:%d\n",lock.l_pid);
                        		}
                		}
        		}else{
                		printf("获取文件锁属性出错!\n");
        		}
        		
        		if(flag==1){
                		lock.l_type=F_RDLCK;
                		if(fcntl(fd,F_SETLKW,&lock)==0){//有别的锁存在的话就等待直到那个锁被释放
                        		printf("给文件%s设置读锁,pid:%d\n",file_name,getpid());
                		}
        		}
        		
                 bzero(buffer,BUFFER_SIZE);  //缓冲区清零
                 int length=0;
                 while((length=read(fd,buffer,BUFFER_SIZE))>0){//读取数据
                           if(send(connect_socket,buffer,length,0)<0){//发送数据
                                      my_err("send",__LINE__);
                                       break;
                           }
                       bzero(buffer,BUFFER_SIZE);//每传输一次需要将buffer清零
                 }
                 
                 printf("Send file:%s successful!\n",file_name);
                 
                 lock.l_type=F_UNLCK;
        		 if((fcntl(fd,F_SETLK,&lock))==0){
                		printf("读锁释放!\n");
        		 }
        		 close(fd);//文件传输完毕，关闭文件
        }else{              //alive=0
        	return;
        }              		       		
}


void do_ls(int connect_socket)
{
        DIR             *dir;
        struct dirent   *ptr;
        struct stat 	buf;
        char            filename[256][PATH_MAXNUM+1],temp[PATH_MAXNUM+1];
        char            param[3][100];
        int             i,j;
        char            *pathname;
        int             count=0;
        char           choice[32];
        int             flag_choice=PARAM_NONE;//参数种类，是否含有-a -l选项
		char 			name[100];
		char 			*path;
		char 			*t;
//		 int             m,n;

        for(i=0;i<3;i++){
             //recv(connect_socket,t,sizeof(t),0);
             //printf("%s\n",t);
             //t=param[i];
             recv(connect_socket,param[i],sizeof(param[i]),0);
        }
        
		for(j=0;j<3;j++){
			printf("%s\n",param[j]);
		}
		
        for(i=1 ; i < strlen(param[1]) ; i++){          //param二位数组的第二行保存的ls的选项
                if(param[1][i] == 'a'){
                        flag_choice |= PARAM_A;
                        continue;
                }else if(param[1][i] == 'l'){
                        flag_choice |= PARAM_L;
                        continue;
                }else{
                        printf("不正确的选项!\n");
                        exit(1);
                }
        }
        param[1][i]='\0';

        //param[2]就是路径
        path=param[2];
        //获取该目录下所有的文件名
        dir=opendir(path);
        if(dir==NULL){
                my_err("opendir",__LINE__);
        }
        while((ptr=readdir(dir))!=NULL){
                if(maxlen < strlen(ptr->d_name)){
                        //t=strlen(ptr->d_name);
                        maxlen = strlen(ptr->d_name);//获取最长文件名
                }
                count++;
        }
        closedir(dir);

        if(count>256){
                my_err("too many files under this dir\n",__LINE__);
        }

        int 	m,n;
        int 	len=strlen(path);
        //获取该目录下所有的文件名
        dir=opendir(path);
        for(m=0 ; m < count ; m++){
                ptr=readdir(dir);
                if(ptr == NULL){
                        my_err("readdir",__LINE__);
                }
                strncpy(filename[m],path,len);//连目录带文件名
                //strcat(filename[m],'/');
                filename[m][len]='/';
                filename[m][len+1]='\0';
                strcat(filename[m],ptr->d_name);
                filename[m][len+strlen(ptr->d_name)+1]='\0';
        }

        for(m=0 ; m < count ; m++){
                for(n=0 ; n < count-1-i ; n++){
                        if(strcmp(filename[n] , filename[j+1]) > 0){
                                strcpy(temp,filename[n+1]);
                                temp[strlen(filename[n+1])]='\0';
                                strcpy(filename[n+1],filename[n]);
                                filename[n+1][strlen(filename[n])]='\0';
                                strcpy(filename[n],temp);
                                filename[n][strlen(temp)]='\0';
                        }
                }
        }

        send(connect_socket,&flag_choice,sizeof(flag_choice),0);
        send(connect_socket,&count,sizeof(count),0);//文件总数传过去
		printf("\n%d  %d\n\n",flag_choice,count);
		
        i=0;
        while(i < count){ 		//count个文件
        		pathname=filename[i];
        		//printf("%s\n",filename[i]);
        		//从路径中解析出文件名
      			for(m=0 , n=0 ; m < strlen(pathname) ; m++){
      				if(pathname[m]=='/'){
      					n=0;
      					continue;
      				}
      				name[n++]=pathname[m];
      			}
      			name[n]='\0';
      			//printf("%s\n",name);
      			
      			if(lstat(pathname,&buf)==-1){//获取文件属性
      				my_err("stat",__LINE__);
      			}
      			
      			switch(flag_choice){
      				case PARAM_A:
      					//display_single(connect_socket,name,count);
      					send(connect_socket,&n,sizeof(n),0);
      					send(connect_socket,name,sizeof(name),0);//发送一个文件名
                                        break;
      				case PARAM_L:
      					//display_attri(connect_socket,buf,name);
      					send(connect_socket,&n,sizeof(n),0);
                        send(connect_socket,&buf,sizeof(buf),0);
                        send(connect_socket,name,sizeof(name),0);
      					break;
      				case PARAM_A + PARAM_L:
      					//display_attri(connect_socket,buf,name);
      					send(connect_socket,&n,sizeof(n),0);
                        send(connect_socket,&buf,sizeof(buf),0);
                        send(connect_socket,name,sizeof(name),0);
                                       // sleep(5);
      					break;
      				default:
      					break;	
      			}
      			i++;
      			bzero(pathname,sizeof(pathname));
      			bzero(&buf,sizeof(buf));
      			bzero(name,sizeof(name));
        }        	        
}

void do_cd(int connect_socket)
{
	char 	curr[30];
	char 	temp[30];
	int 	i,j;
	int 	flag=1;
	recv(connect_socket,temp,sizeof(temp),0);
	
	if(getcwd(curr,sizeof(curr))<0){
		my_err("getcwd",__LINE__);
		flag=0;
	}
	
	if(chdir(temp)<0){
		my_err("chdir",__LINE__);
		flag=0;
	}
	
	if(getcwd(curr,sizeof(curr))<0){
		my_err("getcwd",__LINE__);
		flag=0;
	}
	send(connect_socket,curr,sizeof(curr),0);
	send(connect_socket,&flag,sizeof(flag),0);
	if(flag==0){
		printf("修改目录有误！\n");
		exit(1);
	}
}

