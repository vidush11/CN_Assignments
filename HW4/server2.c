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
#include <time.h>
#define SERVER_PORT  5432
#define MAX_PENDING  5
#define MAX_LINE     256

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

int max(int a, int b){
	if (a>=b) return a;
 	else return b;	
 }

#define NMAX 1000
int main()
{
	struct sockaddr_in sin;
	socklen_t addr_len;
	int s, new_s;
	fd_set fds;

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
	int client_fds[NMAX];
	int ct=0;
	int max_fd=STDIN_FILENO+1;
	max_fd=max(max_fd,s+1);
	struct timeval tv;
	tv.tv_sec=30;
	tv.tv_usec=0;
	while(1){
		
		FD_ZERO(&fds);
		FD_SET(s,&fds);
		FD_SET(STDIN_FILENO, &fds);
		for (int i=0; i<ct; i++){
			FD_SET(client_fds[i],&fds);	
		}
		
		//printf("Some code running.\n");
		
		char buf[MAX_LINE];
		int activity=select(max_fd,&fds, NULL, NULL, &tv);
		
        if (activity<=0){
            continue;
        }
		
		if (FD_ISSET(s,&fds)){
			if ((new_s = accept(s, (struct sockaddr *)&sin, &addr_len)) < 0) {
				perror("accept failed");
				exit(1);
			
			}
			//printf("Client connected\n");
			client_fds[ct++]=new_s;
			max_fd=max(max_fd,new_s+1);
				
		}
		for (int i=0; i<ct; i++){
			if (FD_ISSET(client_fds[i],&fds)){
				int bytes = recv(client_fds[i], buf, sizeof(buf), 0);
				buf[MAX_LINE-1] = '\0';
				if (bytes <= 0) {
					printf("client disconnected.\n");
					close(new_s);
					
				}
				printf("Client says: %s\n", buf);
			}
		}
		
		if  (FD_ISSET(STDIN_FILENO, &fds) && ct>0) {
			// data is avaliable at stdin
			if (fgets(buf, sizeof(buf), stdin) != NULL) {
				buf[MAX_LINE-1] = '\0';

				// broadcast message to all clients
				new_s=client_fds[rand()%ct];
				if (send(new_s, buf, strlen(buf)+1, 0) < 0) {
					perror("send error");
					close(new_s);
				}
			}
		}
		
			
	}
	

	close(s);
	return 0;
}
