#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <ftw.h>
#include <time.h>
#include <string.h>

#define MAX 256
#define ERR(source)(perror(source),\
fprintf(stderr,"%s:%d\n",__FILE__, __LINE__),\
exit(EXIT_FAILURE)

volatile sig_atomic_t last_signal=0;

void sethandler(void (*f)(int), int sigNo)
{	
	struct sigaction a;
	memset(&a, 0, sizeof(struct sigaction));
	a.sa_handler =f;
	sigaction(sigNo, &a, NULL);
}

void sig_handler(int sig)
{
	last_signal = sig;
}

void sigchld_handler(int sig)
{
	pid_t p;
	while(1)
	{
		p=waitpid(0,NULL,WNOHANG);
		if(p<0) return;
	}
}

void child_work(int i)
{
	int random;
	srand(time(NULL)*getpid());
	
	random=rand()%281+20;
	
	struct timespec t={0,random*1000000};
	
	
	printf("[%d] bit: %d; random: %d\n", getpid(),i,random);
	while(1)
	{
		//printf("sth\n");
	nanosleep(&t,NULL);
	if(i==0) kill(getppid(), SIGUSR2);
	if(i==1) kill(getppid(), SIGUSR1);
}

	exit(EXIT_SUCCESS);
}

void parent_work(sigset_t oldmask,int len,char* tmp)
{
	int buf[5]={0,0,0,0,0};
	int curr,j,ch,lch=0;
	while(1)
	{
		last_signal=0;
		while(last_signal != SIGUSR2 && last_signal != SIGUSR1) sigsuspend(&oldmask);
		
		//printf("got it\n");
		
		
		
		if(last_signal==SIGUSR1) curr=1;
		if(last_signal==SIGUSR2) curr=0;
		
		buf[4]=buf[3];
		buf[3]=buf[2];
		buf[2]=buf[1];
		buf[1]=buf[0];
		buf[0]=curr;
		
		printf("[%d,%d,%d,%d,%d]\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
		
		for(j=0;j<len-5;j++)
		{
			ch=0;
			if(buf[0]==tmp[j]-48)ch++;
			if(buf[1]==tmp[j+1]-48) ch++;
			else ch=0;
				
			if(buf[2]==tmp[j+2]-48)ch++;
			else ch=0;
					
			if(buf[3]==tmp[j+3]-48) ch++;
			else ch=0;
						
			if(buf[4]==tmp[j+4]-48) ch++;
			else ch=0;
							
			if(lch<ch)
			{
				printf("longer substring: %d\n",ch);
				lch=ch;
			}
			
			
		}
		
		if (ch==5) 
		{
			break;
			exit(EXIT_SUCCESS);
		}
	}
}


int main (int agrc, char** argv)
{
	int in,i;
	ssize_t len;
	char *tmp=malloc(MAX);
	pid_t pid;
	
	sethandler(sigchld_handler,SIGCHLD);
	sethandler(sig_handler,SIGUSR1);
	sethandler(sig_handler,SIGUSR2);
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	
	in=open(argv[1], O_RDONLY);
	len=read(in,tmp,MAX);
	
	
	printf("%ld, %s\n\n", len, tmp);
	
	for(i=0; i<len; i++)
	{
		pid=fork();
		
		if(pid==0)
		{
			child_work(tmp[i]-48);
			exit(EXIT_SUCCESS);
		}
		
	}
	parent_work(oldmask,len,tmp);
	
	
	
	
	exit(EXIT_SUCCESS);
}
