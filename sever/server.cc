#include <sys/types.h>  
#include <sys/wait.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <iostream>  
#include <signal.h>  
#include <memory.h>  
#include <errno.h>  
#include <stdlib.h>  
#include <time.h>
#include <stdio.h>
#include <string.h>

const int BUF_SIZE = 1024;  
  
using namespace std;  
  
void sig_chld(int sig)  
{  
    pid_t pid;  //进程号的类型定义
    int stat;  
    while((pid = waitpid(-1,&stat,WNOHANG))>0)  
        cout << "child " << pid << " termination" << endl;  
    return;  
}
  
char* getPicName()
{
    time_t rawtime;
    struct tm * timeinfo;
    char *buffer=(char *)malloc(30*sizeof(char));
    if (buffer == 0)
    {
        fprintf(stdout,"can't alloc mem\n");
        return 0;
    }
    else
    {
        memset(buffer,0x00,sizeof(char)*26);
    }

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buffer,30,"%F-%I-%M-%S%p.jpg",timeinfo);//save as jpg files

    //puts (buffer);

    return buffer;
}  

int main()  
{  
    int serv_sockfd;  
    int client_sockfd;  
  
    char buf[BUF_SIZE];  
    FILE *fp;

    memset(buf,0,sizeof(buf));  
  
    sockaddr_in serv_addr;  
    sockaddr_in client_addr;  
  
    memset(&serv_addr,0,sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(8887);  
  
    if((serv_sockfd = socket(PF_INET,SOCK_STREAM,0))<0)  
    {  
        cout << "socket error" << endl;  
        return -1;  
    }  
  
    if((bind(serv_sockfd,(sockaddr *)&serv_addr,sizeof(serv_addr)))<0)  
    {  
        cout << "bind error" << endl;  
        return -2;  
    }  
  
    if((listen(serv_sockfd,5))<0)  
    {  
        cout << "listen error" << endl;  
        return -3;  
    }  
    cout << "listening..." << endl;  
  
    signal(SIGCHLD,sig_chld);  
  
    socklen_t sin_size;  
    int read_len=0;  
    int write_len=0;  
    int child_pid;  
    while(1)  
    {  
        sin_size = sizeof(client_addr);  
        memset(&client_addr,0,sizeof(client_addr));  
        if((client_sockfd = accept(serv_sockfd,(sockaddr*)&client_addr,&sin_size))<0)  
        {  
            if(errno == EINTR || errno == ECONNABORTED)  
                continue;  
            else  
            {  
                cout << "accept error" << endl;  
                return -4;  
            }  
        }  
  
        if((child_pid = fork())==0)  
        {  
            cout << "client " <<inet_ntoa(client_addr.sin_addr) << " 进程号："<< getpid() << endl;  
            write(client_sockfd,"Welcome to my server",21);  
            close(serv_sockfd);  
            char *date = getPicName();
	    //puts(date);
            if((fp = fopen(date,"w"))== NULL){
		perror("Open file failed\n");
		exit(0);
            }
            while(1)  
            {  
                memset(buf,0,sizeof(buf));  
                read_len = read(client_sockfd,buf,BUF_SIZE);
                if(read_len < 0)  
                {  
                    close(client_sockfd);  
                    //cout << "read error" << endl;  
                    exit(-5);  
                }else if(read_len == 0){
		    break;
		}  
 		write_len = fwrite(buf,sizeof(char),read_len,fp);
		if(write_len < read_len){
		    printf("Write file failed\n");
		    break;
		}
		bzero(buf,BUF_SIZE);	
            }
	    
            free(date);
            close(client_sockfd);  
            exit(-6);  
        }  
        else if(child_pid>0)  
            close(client_sockfd);  
    }  
    return 0;  
}
