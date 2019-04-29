#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <string.h>
#define MAXFD 20

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

void usage(char* pname){
	fprintf(stderr,"USAGE:%s -p directory\n",pname);
	exit(EXIT_FAILURE);
}             


int txt=0, c=0, h=0;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
    const char* ext = strrchr(name, '.');
    if(strcmp(ext, ".txt") == 0) txt++;
    else if(strcmp(ext, ".c") == 0) c++;
    else if(strcmp(ext, ".h") == 0) h++;
	return 0;
}

int main(int argc, char** argv) {
	int x, e=2;
        while ((x = getopt (argc, argv, "p:e:")) != -1){
            switch (x)
            {
                case 'p':
                    if(nftw(optarg,walk,MAXFD,FTW_PHYS)==0){
                       printf("Directory: %s\nDirectory analysis:\n",optarg); 
                       e=0;
                    }
                    else usage(argv[0]);
                    break;
                case 'e':
                        if(strcmp(optarg, "txt") == 0){
                            printf("c: %d\nh: %d\n",c, h);
                            e=1;
                        }
                        else if(strcmp(optarg, "h") == 0){
                            printf("txt: %d\nc: %d\n",txt, c);
                            e=1;
                        }
                        else if(strcmp(optarg, "c") == 0){
                            printf("txt: %d\nh: %d\n",txt, h);
                            e=1;
                        }
                        else{
                            printf("txt: %d\nc: %d\nh: %d\n",txt, c, h);
                            e=1;
                        }
                        break;
                /*case 'n':
                    name=optarg;
                    break;*/
                case '?':
                default:
                    usage(argv[0]);
            }
        }
        if(e==0)
            printf("txt: %d\nc: %d\nh: %d\n",txt, c, h);
		txt=c=h=0;
	return EXIT_SUCCESS;
}
