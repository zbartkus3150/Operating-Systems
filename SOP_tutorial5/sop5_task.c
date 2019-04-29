#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))

#define MAX_BUFF 200

void child_work(int fd, int R, char* com) {
	char buf[MAX_BUFF+1];
	//unsigned char s;
	//srand(getpid());
    int n=strlen(com),i;
    char c = 1;
	for(i=0;i<=n;i++){
                write(R,&c,1);
		if(TEMP_FAILURE_RETRY(write(R,&com[i],1)) <0) ERR("write to R");
	}
}

void parent_work(char* com,int *fds,int R) {
    unsigned char c;
	char buf[MAX_BUFF];
	int status;
	srand(getpid());
    for(;;){
        /*if(fds[1]){
            c = 'a'+rand()%('z'-'a');
            status=TEMP_FAILURE_RETRY(write(fds[1],&c,1));
            if(status!=1) {
                if(TEMP_FAILURE_RETRY(close(fds[1]))) ERR("close");
                fds[1]=0;
            }
        }*/
		status=read(R,&c,1);
		if(status<0&&errno==EINTR) continue;
		if(status<0) ERR("read header from R");
		if(0==status) break;
		if(TEMP_FAILURE_RETRY(read(R,buf,c))<c)ERR("read data from R");
		//buf[(int)c]=0;
		printf("%s\n",buf);
	}
        //printf("\n");
}

void create_children_and_pipes(char* com , int* f, int R) {
	int tmpfd[2];
    if(pipe(tmpfd)) ERR("pipe");
    switch (fork()) {
        case 0:
            //if(TEMP_FAILURE_RETRY(close(tmpfd[0]))) ERR("close");
            //printf("CHILD %d close descriptor %i\n",getpid(),tmpfd[0]);
            //if(TEMP_FAILURE_RETRY(close(tmpfd[1]))) ERR("close");
            //printf("CHILD %d close descriptor %i\n",getpid(),tmpfd[1]);           
            child_work(tmpfd[0],R,com);
            exit(EXIT_SUCCESS);

        case -1: ERR("Fork:");
    }
    f = tmpfd;
}

void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"0<n<=10 - number of children\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
        int R[2],fds[2];
	if(2!=argc) usage(argv[0]);
	if(pipe(R)) ERR("pipe");
    //if(TEMP_FAILURE_RETRY(close(R[0]))) ERR("close");
    //printf("PARENT %d close descriptor %i\n",getpid(),R[0]);
    create_children_and_pipes(argv[1],fds,R[1]);
    parent_work(argv[1],fds,R[0]); 
    //if(TEMP_FAILURE_RETRY(close(R[1]))) ERR("close");
    //printf("PARENT %d close descriptor %i\n",getpid(),R[1]);
	
	return EXIT_SUCCESS;
}