#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#define MAX_PATH 101

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

int sum =0;

void scan_dir (char* path, char* bytes){
	DIR *dirp;
	struct dirent *dp;
	struct stat filestat;
	if (NULL == (dirp = opendir(path))) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			if (lstat(dp->d_name, &filestat)) ERR("lstat");
            if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
                sum += filestat.st_size;
                //printf("%s %ld\n", dp->d_name, filestat.st_size);
            }
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
    }

int main(int argc, char** argv) {
	int i;
    FILE *f;
    f = fopen("out.txt", "w+");
	char path[MAX_PATH];
	if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
	for(i=1;i<argc;i+=2){
		if(chdir(argv[i]))ERR("chdir");
		scan_dir(argv[i], argv[i+1]);
        if(sum > atoi(argv[i+1]))
            fprintf(f, "%s\n", argv[i]);
		if(chdir(path))ERR("chdir");
	}
	return EXIT_SUCCESS;
}
