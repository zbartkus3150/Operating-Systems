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


void server_work(mqd_t mq_s, mqd_t mq_d, mqd_t mq_m)
{
}


int main(int argc, char** argv)
{
	char name1[20];char name2[20];char name3[20];
	if(snprintf(name1,20,"/%d_s",getpid()) <0) ERR("snprintf");
	if(snprintf(name2,20,"/%d_d",getpid()) <0) ERR("snprintf");
	if(snprintf(name3,20,"/%d_m",getpid()) <0) ERR("snprintf");

	struct mq_attr attr;
	attr.mq_maxmsg=10;
	attr.mq_msgsize=20; 
	mqd_t des_m;
	mqd_t des_d;
	mqd_t des_s;
	if((des_m=mq_open(name1,O_RDWR | O_CREAT, 0600, &attr)==(mqd_t)-1)) ERR("mq_open");
	if((des_s=mq_open(name1,O_RDWR | O_CREAT, 0600, &attr)==(mqd_t)-1)) ERR("mq_open");
	if((des_d=mq_open(name1,O_RDWR | O_CREAT, 0600, &attr)==(mqd_t)-1)) ERR("mq_open");

	printf("Server %s %s %s queues\n", name1, name2, name3);

	sleep(1);

	mq_close(des_d); mq_close(des_s); mq_close(des_m); 
	return EXIT_SUCCESS;
}