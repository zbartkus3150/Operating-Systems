#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#define MAX 256

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

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

void usage(char *name){
	fprintf(stderr,"USAGE: %s name \n",name);
	fprintf(stderr,"name of the file\n");
	exit(EXIT_FAILURE);
}

void child_work(int buf) {
    int random;
	srand(time(NULL)*getpid());
	random = rand()%191+10;
    struct timespec t={0,random*1000000};
    printf("[%d] Bit: %d Interval: %dms\n",getpid(), buf, random); 
    while(1)
	{
        nanosleep(&t,NULL);
        if(buf==0) kill(getppid(), SIGUSR2);
        if(buf==1) kill(getppid(), SIGUSR1);
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

int main(int argc, char** argv) {
	if(argc!=2) usage(argv[0]);
	sethandler(sigchld_handler,SIGCHLD);
	sethandler(sig_handler,SIGUSR1);
	sethandler(sig_handler,SIGUSR2);
	pid_t pid;
    int in, i;
    ssize_t len;
    sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGUSR1);
	sigaddset(&mask,SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	char *buf=malloc(MAX);
	if(!buf) ERR("malloc");
	if((in=open(argv[1],O_RDONLY))<0)ERR("open");
	if((len=read(in,buf,MAX))<0) ERR("read");
    printf("%ld, %s\n", len, buf);
    for(i=0; i<len; i++){
        if((pid=fork())<0) ERR("fork");
        if(0==pid) {
            child_work(buf[i]-48);
            exit(EXIT_SUCCESS);
        }
    }
    parent_work(oldmask,len,buf);
	if(close(in))ERR("close");
	free(buf);
    while(wait(NULL)>0){}
	exit(EXIT_SUCCESS);
}
