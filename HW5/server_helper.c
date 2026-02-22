

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "server.h"


// server received a string message from client_id
// string message is in msg
// len is the length of the message
// num_clients is maximum number of clients
// valid_ids is an integer array of size num_clients
// all integers between 0 to num_clients-1 are potential clients
// a client X is valid if valid_ids[i] != -1

void recv_message(char *msg, int len, int client_id, int *valid_ids, int num_clients) {
// Hint: use sscanf to parse string (depending on your protocol)
// %n gives the offset of partially parsed string using sscanf
// use send_message API, see server.h
	
	if (len>=6){
		char buff[5];
		memcpy(buff, &msg[0], 4);
		//printf("%s\n",buff);
		if (strcmp(buff,"LIST")==0){
			char to_send[100];
			
			int c=0;
			for (int i=0; i<num_clients; i++){
				if (valid_ids[i]!=-1){
				//I am considering i<=9 otherwise this fails
					to_send[c++]=(i+'0');
					to_send[c++]=' ';
					if (c>=100-1) break;
				}
			}
			to_send[c++]='\0';
			send_message(to_send, c, client_id, SERVER_ID);
		
		}
		else if (strcmp(buff, "DATA")==0){
			int clients;
			sscanf(msg, "%s%d", buff, &clients);
			int ids[100];
			int c=0;
			int skip=0;
			int curr=0;	
			char data[100];
			int d=0;
			for (int i=0; i<len; i++){
				if (d){
					if (msg[i]=='\n') break;
					data[c++]=msg[i];
				}
				else{
				if (msg[i]==':'){
					ids[c++]=curr;
					i++;
					d=1;
					c=0;
					continue;
				}
				
				if (skip!=2){
					if (msg[i]==' ')skip++;
				}
				else{
					if (msg[i]==' '){
						ids[c++]=curr;
						curr=0;
						
					}
					else{
					curr*=10;
					curr+=msg[i]-'0';
					
					}
				}
				}
			}
			data[c++]='\0';
			for (int i=0; i<clients; i++){
				if (ids[i]<num_clients && valid_ids[ids[i]]!=-1){
				send_message(data, c, ids[i], client_id);
				}
			}
			
			//printf("\n|%s|\n",data);
		}
		else{
		printf("client %d: %s\n", client_id, msg);}
	}
	else{
	printf("client %d: %s\n", client_id, msg);}
}
