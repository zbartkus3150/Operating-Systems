#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    void usage(char* pname){
        fprintf(stderr, "USAGE:%s name times>0\n", pname);
        exit(EXIT_FAILURE);
    }
    if(argc!=3) usage(argv[0]);
	int i,j=atoi(argv[2]);
	if(0==j) usage(argv[0]);
	for(i=0;i<j;i++)
		printf("Hello %s\n",argv[1]);
    return EXIT_SUCCESS;
}