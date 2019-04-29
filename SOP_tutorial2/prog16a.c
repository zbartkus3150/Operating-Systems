#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;

void sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) {
	sig_count++;;
}

void child_work(int m) {
	struct timespec t = {0, m*10000};
	sethandler(SIG_DFL,SIGUSR1);
	while(1){
		nanosleep(&t,NULL);
		if(kill(getppid(),SIGUSR1))ERR("kill");
	}
}

void parent_work(int b, int s, char *name) {
	int i,in,out;
	ssize_t count;
	char *buf=malloc(s);
	if(!buf) ERR("malloc");
	if((out=open(name,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,0777))<0)ERR("open");
	if((in=open("/dev/urandom",O_RDONLY))<0)ERR("open");
	for(i=0; i<b;i++){
		if((count=read(in,buf,s))<0) ERR("read");
		if((count=write(out,buf,count))<0) ERR("read");
		if(fprintf(stderr,"Block of %ld bytes transfered. Signals RX:%d\n",count,sig_count)<0)ERR("fprintf");;
	}
	if(close(in))ERR("close");
	if(close(out))ERR("close");
	free(buf);
	if(kill(0,SIGUSR1))ERR("kill");
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s m b s \n",name);
	fprintf(stderr,"m - number of 1/1000 milliseconds between signals [1,999], i.e. one milisecond maximum\n");
	fprintf(stderr,"b - number of blocks [1,999]\n");
	fprintf(stderr,"s - size of of blocks [1,999] in MB\n");
	fprintf(stderr,"name of the output file\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int m,b,s;
	char *name;
	if(argc!=5) usage(argv[0]);
	m = atoi(argv[1]); b = atoi(argv[2]);  s = atoi(argv[3]); name=argv[4];
	if (m<=0||m>999||b<=0||b>999||s<=0||s>999)usage(argv[0]); 
	sethandler(sig_handler,SIGUSR1);
	pid_t pid;
	if((pid=fork())<0) ERR("fork");
	if(0==pid) child_work(m);
	else {
		parent_work(b,s*1024*1024,name);
		while(wait(NULL)>0);
	}
	return EXIT_SUCCESS;
}
