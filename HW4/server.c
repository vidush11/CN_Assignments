// Source: https://book.systemsapproach.org/foundation/software.html
// sudo lsof -i :5432

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define SERVER_PORT  5432
#define MAX_PENDING  5
#define MAX_LINE     256

typedef struct {
	int s;
} th_args;

void event_loop(int new_s)
{
	fd_set readfds;
	struct timeval tv;
	int max_fd = STDIN_FILENO;
	int activity;
	char buf[MAX_LINE];

	if (new_s > max_fd) {
		max_fd = new_s;
	}

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(new_s, &readfds);

		tv.tv_sec = 30;
		tv.tv_usec = 0;

		activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

		if (activity < 0) {
			perror("select");
			break;
		} else if (activity == 0) {
			printf("No activity for 30 secs.\n");
			continue;
		}

		if (FD_ISSET(new_s, &readfds)) {
			// data is available at client[i] connection
			int bytes = recv(new_s, buf, sizeof(buf), 0);
			buf[MAX_LINE-1] = '\0';
			if (bytes <= 0) {
				printf("client disconnected.\n");
				close(new_s);
				return;
			}
			printf("Client says: %s\n", buf);
		}

		if  (FD_ISSET(STDIN_FILENO, &readfds)) {
			// data is avaliable at stdin
			if (fgets(buf, sizeof(buf), stdin) != NULL) {
				buf[MAX_LINE-1] = '\0';

				// broadcast message to all clients
				if (send(new_s, buf, strlen(buf)+1, 0) < 0) {
					perror("send error");
					close(new_s);
					return;
				}
			}
		}
	}
	close(new_s);
}

void* ok_fun(void* ptr){
	th_args* t=(th_args*) ptr;
	event_loop(t->s);
	return NULL;
}
#define NTHREADS 1000
int main()
{
	struct sockaddr_in sin;
	socklen_t addr_len;
	int s, new_s;

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	/* setup passive open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(1);
	}
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
		perror("bind failed");
		exit(1);
	}
	if (listen(s, MAX_PENDING) < 0) {
		perror("listen failed");
		exit(1);
	}
	
	pthread_t tid[NTHREADS];
	th_args args[NTHREADS];
	
	int ct=0;
	while (1){
		if ((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
			perror("accept failed");
			exit(1);
		}
		args[ct].s=new_s;
		pthread_create(&tid[ct],NULL,ok_fun, (void*) &args[ct]);
		ct++;
	}
	close(s);
	return 0;
}
