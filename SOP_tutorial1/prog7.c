#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv) {
    int i=0;
    extern char **environ;
    while (environ[i])
		printf("%s\n", environ[i++]);
    return EXIT_SUCCESS;
}