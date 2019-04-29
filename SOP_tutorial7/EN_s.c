//serial
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#define ERR(source) (perror(source),\
		fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		exit(EXIT_FAILURE))

#define BACKLOG 3
volatile sig_atomic_t do_work = 1;
int max = 0;
void sigint_handler(int sig) {
	do_work = 0;
}
int sethandler(void(*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1 == sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}
int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain, type, 0);
	if (sock < 0) ERR("socket");
	return sock;
}
int bind_local_socket(char *name) {
	struct sockaddr_un addr;
	int socketfd;
	if (unlink(name) < 0 && errno != ENOENT) ERR("unlink");
	socketfd = make_socket(PF_UNIX, SOCK_STREAM);
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);
	if (bind(socketfd, (struct sockaddr*) &addr, SUN_LEN(&addr)) < 0)  ERR("bind");
	if (listen(socketfd, BACKLOG) < 0) ERR("listen");
	return socketfd;
}
int bind_tcp_socket(uint16_t port) {
	struct sockaddr_in addr;
	int socketfd, t = 1;
	socketfd = make_socket(PF_INET, SOCK_STREAM);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t))) ERR("setsockopt");
	if (bind(socketfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)  ERR("bind");
	if (listen(socketfd, BACKLOG) < 0) ERR("listen");
	return socketfd;
}
int add_new_client(int sfd) {
	int nfd;
	if ((nfd = TEMP_FAILURE_RETRY(accept(sfd, NULL, NULL))) < 0) {
		if (EAGAIN == errno || EWOULDBLOCK == errno) return -1;
		ERR("accept");
	}
	return nfd;
}
void usage(char * name) {
	fprintf(stderr, "USAGE: %s socket port\n", name);
}
ssize_t bulk_read(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(read(fd, buf, count));
		if (c < 0) return c;
		if (0 == c) return len;
		buf += c;
		len += c;
		count -= c;
	} while (count > 0);
	return len;
}
ssize_t bulk_write(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(write(fd, buf, count));
		if (c < 0) return c;
		buf += c;
		len += c;
		count -= c;
	} while (count > 0);
	return len;
}
void calculate(char dataRec[10], int16_t dataToSend[1]) {
	int16_t sum = 0;
	int i = 0;
	while (dataRec[i] != '\0') {
		sum += (dataRec[i] - '0');
		i++;
	}
	printf("sum: %d\n", sum);
	if (sum > max)
		max = sum;
	dataToSend[0] = htons(sum);

}
void communicate(int cfd) {
	ssize_t size;
	char data[10];
	int16_t dataToSend[1];
	int hits = 0;
	if ((size = bulk_read(cfd, (char *)data, sizeof(char[10]))) < 0) ERR("read:");
	if (size == (int)sizeof(char[10])) {
		printf("dataRec: %s\n", data);
		calculate(data, dataToSend);
		printf("sum: %d\n", ntohs(dataToSend[0]));

		if (bulk_write(cfd, (char *)dataToSend, sizeof(int16_t[1])) < 0) ERR("write:");
	}

	if (TEMP_FAILURE_RETRY(close(cfd)) < 0)ERR("close");
}
void doServer(int fdT) {
	int cfd, fdmax;
	fd_set base_rfds, rfds;
	sigset_t mask, oldmask;
	FD_ZERO(&base_rfds);
	FD_SET(fdT, &base_rfds);
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	while (do_work) {
		rfds = base_rfds;
		if (pselect(fdT + 1, &rfds, NULL, NULL, NULL, &oldmask) > 0) {
			cfd = add_new_client(fdT);
			if (cfd >= 0)communicate(cfd);
		}
		else {
			if (EINTR == errno) continue;
			ERR("pselect");
		}
	}
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
int main(int argc, char** argv) {
	int fdT;
	int new_flags;
	if (argc != 2) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	if (sethandler(SIG_IGN, SIGPIPE)) ERR("Seting SIGPIPE:");
	if (sethandler(sigint_handler, SIGINT)) ERR("Seting SIGINT:");
	fdT = bind_tcp_socket(atoi(argv[1]));
	new_flags = fcntl(fdT, F_GETFL) | O_NONBLOCK;
	fcntl(fdT, F_SETFL, new_flags);
	doServer(fdT);
	if (TEMP_FAILURE_RETRY(close(fdT)) < 0)ERR("close");
	fprintf(stderr, "Biggest sum: %d\n\nServer has terminated.\n", max);
	return EXIT_SUCCESS;

}