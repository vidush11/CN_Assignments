
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "receiver.h"
#include "config.h"


#define ull unsigned long long
// received a packet, pkt
// len, length of the paket
unsigned long long recv_base=0;
unsigned long long recv_read=0;
unsigned char data[300000000];
int received[300000000];
typedef struct Node Node;
typedef struct Queue Queue;

struct Node{
	void* pkt;
	size_t len;
	Node* next;
};

struct Queue{
	Node* head;
	Node* tail;
};

ull min(ull a, ull b){
	if (a<=b) return a;
	else return b;
}

typedef struct pkt_header pkt_header;


void rdt_recv(const unsigned char *pkt, size_t len)
{	
	if (recv_base==0) {for (ull i=0; i<300000000; i++) {received[i]=0;}}
	const unsigned char* pkt_data=get_data((void*)pkt);

	len-=PACKET_HEADER_LEN;
	if (get_seqno((void*)pkt)>=recv_base){
		for (ull i=0; i<len; i++){
			data[i+get_seqno((void*) pkt)]=pkt_data[i];
			received[i+get_seqno((void*) pkt)]=len-i;	
		}
		if (get_seqno((void*)pkt)==recv_base) {recv_base+=len;}
		while (received[recv_base]) {recv_base+=received[recv_base];}
		if (received[recv_read]) notify_app();
	}
	while (received[recv_base]) {recv_base+=received[recv_base];}
	
	//printf("Recv_base, %llu.\n", recv_base);
	send_ack(recv_base);
}

// app requested a data of length len
// buf is len bytes long
// returns the number of bytes copied to buf
size_t app_recv(unsigned char *buf, size_t len)
{	
	size_t sent=0;
	ull curr=recv_read;
	//printf("%llu, %llu.\n", curr, recv_base);
	//while (received[recv_base]==1) {recv_base++;}
	while (sent<len && curr<recv_base){
		buf[sent++]=data[curr++];
	}
	recv_read=curr;
	return sent;
}

