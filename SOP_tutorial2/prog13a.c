#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))

void usage(char *name){
        fprintf(stderr,"USAGE: %s 0<n\n",name);
        exit(EXIT_FAILURE);
}

void child_work(int i) {
        srand(time(NULL)*getpid());     
        int t=5+rand()%(10-5+1);
        sleep(t);
        printf("PROCESS with pid %d terminates\n",getpid());
}

void create_children(int n) {
        pid_t s;
        for (n--;n>=0;n--) {
                if((s=fork())<0) ERR("Fork:");
                if(!s) {
                        child_work(n);
                        exit(EXIT_SUCCESS);
                }
        }
}

int main(int argc, char** argv) {
        int n;
        if(argc<2)  usage(argv[0]);
        n=atoi(argv[1]);
        if(n<=0)  usage(argv[0]);
        create_children(n);
        return EXIT_SUCCESS;
}