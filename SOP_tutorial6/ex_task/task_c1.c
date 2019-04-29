#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>
#include <signal.h>
#include <pthread.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = f;
	act.sa_flags=SA_SIGINFO;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}


void client_work(mqd_t des_s)
{

}


int main(int argc, char** argv)
{
   char name1[6];
   if(snprintf(name1,6,"/%d", getpid()) !=5) ERR("sprintf");

   struct mq_attr attr;
   attr.mq_maxmsg = 10;
   attr.mq_msgsize = 10;

    mqd_t mdes;

    if((mdes = mq_open(name1,O_RDWR| O_CREAT, 0600, &attr))==(mqd_t)-1) ERR("mq_open");

    printf("Client %s\n", name1);

    sleep(1);

    mq_close(mdes);


    return EXIT_SUCCESS;
}