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

#define MSIZE sizeof(double)*2+20

struct message
{
    double f1;
    double f2;
    char name[10];
};

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

volatile sig_atomic_t flag=0;


/*
    i am not really sure wheter this trickery with double is ok but 
    this is the way i did it
*/

void server_work(mqd_t mq_s, mqd_t mq_d, mqd_t mq_m)
{
    struct message mes;
	if(mq_receive(mq_s, (char*) &mes, sizeof(struct message), NULL)<0) ERR("mq_recive"); 
    printf("server recived %lf,%lf,%s\n",mes.f1, mes.f2, mes.name);
    mqd_t des_c;
    printf("client name in mes  %s\n", mes.name);
    if((des_c=mq_open(mes.name,O_WRONLY,NULL)) == (mqd_t)-1) ERR("mq_open");

    printf("dobuble: %lf\n",mes.f1);
    if(mq_send(des_c,(char*) &(mes.f1),sizeof(double),0)) ERR("mq send"); 
    mq_close(des_c);
    return;
		
}


int main(int argc, char** argv)
{
	char name1[20];char name2[20];char name3[20];
    printf("%d\n",getpid());
	if(snprintf(name1,20,"/%d_s",(int)getpid()) < 0) ERR("snprintf");
	if(snprintf(name2,20,"/%d_d",(int)getpid()) < 0) ERR("snprintf");
	if(snprintf(name3,20,"/%d_m",(int)getpid()) < 0) ERR("snprintf");

	struct mq_attr attr;
	attr.mq_maxmsg=10;
	attr.mq_msgsize=sizeof(struct message); 
	mqd_t des_m;
	mqd_t des_d;
	mqd_t des_s;

	if((des_s=mq_open(name1,O_RDWR | O_CREAT, 0600, &attr))==(mqd_t)-1) ERR("mq_open");
	if((des_d=mq_open(name2,O_RDWR | O_CREAT, 0600, &attr))==(mqd_t)-1) ERR("mq_open");
	if((des_m=mq_open(name3,O_RDWR | O_CREAT, 0600, &attr))==(mqd_t)-1) ERR("mq_open");

	printf("Server %s %s %s queues\n", name1, name2, name3);

    server_work(des_s, des_d, des_m);
	sleep(1);

    // unlink to make sure que is gone after closing
    mq_unlink(name1); 
    mq_unlink(name2); 
    mq_unlink(name3); 
	mq_close(des_d); 
    mq_close(des_s);
    mq_close(des_m); 
	return EXIT_SUCCESS;
}