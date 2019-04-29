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

struct message
{
    double f1;
    double f2;
    char name[10];
};

#define MSIZE sizeof(double)*2+20

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


void client_work(mqd_t des_s, mqd_t des_c, char name[10])
{
    struct message mes;
    printf("gimme numbers boi: ");
    scanf("%1lf%1lf",&(mes.f1),&(mes.f2));
    snprintf(mes.name,10,"%s",name);
    if(mq_send(des_s, (char*)&mes, sizeof(struct message),1) < 0 ) ERR("mq_send");
    double dub;
    if(mq_receive(des_c,(char*) &dub,sizeof(struct message),NULL) < 0 ) ERR("mq_recive");
    printf("client recived %lf\n",dub);
    return;
}


int main(int argc, char** argv)
{
   char name1[10];
   if(snprintf(name1,10,"/%d", getpid()) < 0) ERR("sprintf");

   struct mq_attr attr;
   attr.mq_maxmsg = 10;
   attr.mq_msgsize = sizeof(struct message);

    mqd_t des_c;
    mqd_t des_s;
    if((des_c = mq_open(name1,O_RDWR| O_CREAT, 0600, &attr))==(mqd_t)-1) ERR("mq_open");
    if((des_s = mq_open(argv[1],O_WRONLY, NULL))==(mqd_t)-1) ERR("mq_open");
    printf("Client %s\n", name1);
    printf("server %s\n", argv[1]);

    client_work(des_s,des_c,name1);
    sleep(1);

    mq_unlink(name1);
    mq_close(des_c);
    mq_close(des_s);


    return EXIT_SUCCESS;
}