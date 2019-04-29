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
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>


#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                     exit(EXIT_FAILURE))
#define MSG_LEN 128

void notify_f(union sigval sv)
{
    mqd_t q = *((mqd_t*)sv.sival_ptr);
    char message[MSG_LEN];
    
    static struct sigevent not;
    not.sigev_notify=SIGEV_THREAD;
    not.sigev_notify_function = notify_f;
    not.sigev_notify_attributes = NULL;
    not.sigev_value.sival_ptr = &q;

    if(mq_notify(q,&not)==-1) ERR("notify");

    unsigned msg_prio;
    for(;;)
    {
        
        if(mq_receive(q,message,MSG_LEN,&msg_prio)<1) 
        {
                if(errno==EAGAIN) break;
                else ERR("mq_receive");
        }
        
        printf("Read %s\n",message);
    }
                
}

mqd_t create_queue()
{
    mqd_t queue;

    struct mq_attr attr;
    attr.mq_maxmsg=3;
    attr.mq_msgsize=MSG_LEN;
    char path[20]="";
    snprintf(path,20,"/mq_%d",getpid());
    printf("[%d]\n", getpid());
    queue=TEMP_FAILURE_RETRY(mq_open(path, O_RDWR | O_NONBLOCK | O_CREAT, 0606, &attr));
    if(queue==(mqd_t)-1) ERR("mq open");
    struct sigevent not;
    not.sigev_notify=SIGEV_THREAD;
    not.sigev_notify_function = notify_f;
    not.sigev_notify_attributes = NULL;
    not.sigev_value.sival_ptr = &queue;  
    if(mq_notify(queue,&not)==-1) ERR("notify");
    return queue;
}

int main(int argc, char** argv) 
{
    mqd_t queue = create_queue();
    int pname;
    char message[128];
    int cont = 1;
    char path[20]="";
    while(cont)
    {
        scanf("%s",message);
        if(isdigit(message[0])){
            pname = atoi(message);
            printf("peer PID: %d\n", pname);
            strcpy(path,"/mq_");
            strcat(path,message);
            struct mq_attr attr;
            attr.mq_maxmsg=3;
            attr.mq_msgsize=MSG_LEN;
            if((queue=TEMP_FAILURE_RETRY(mq_open(path, O_RDWR | O_NONBLOCK , 0606, &attr)))==-1){
                if(errno==ENOENT)
                    printf("this peer does not exist!\n");
                else ERR("mq_open");
            }
        }
        else{
            if(queue)
            printf("message: %s\n", message);
            if(mq_send(queue,message,MSG_LEN,0) == -1){
                if(errno==EAGAIN)
                    printf("queue is full!\n");
                else if(errno==EBADF)
                    printf("You are not connected to other peer\n");
                else ERR("mq_send");
            }
        }
    }
    mq_close(queue);
    mq_unlink(path);
    return EXIT_SUCCESS;
}
