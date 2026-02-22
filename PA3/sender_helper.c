
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sender.h"
#include "config.h"
#include<unistd.h>
#define ull unsigned long long
unsigned long long seq_no=0;
unsigned long long send_base=0;

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

Queue* qu=NULL;

void insert(const void* pt, size_t len){
	Node* new_pkt=(Node*)malloc(sizeof(Node));
	void* pt_=malloc(len);
	memcpy(pt_,pt, len);
	new_pkt->pkt=pt_;
	new_pkt->len=len;
	new_pkt->next=NULL;
	//printf("Inside insert has seq_no. %llu.\n", get_seqno(pt_));
	if (qu->head==NULL){
		qu->head=new_pkt;
		qu->tail=new_pkt;
	}
	else{	
		qu->tail->next=new_pkt;
		qu->tail=qu->tail->next;
		//printf("Head seq no, Tail seq no %llu, %llu.\n", get_seqno(qu->head->pkt), get_seqno(qu->tail->pkt));
	}
		
}
	
void del(Queue *que,unsigned long long ackno){
	//printf("Received ack for no %llu.\n", ackno);
        if (ackno>send_base){
        	stop_timer();
		while (que->head!=NULL && get_seqno(que->head->pkt)+que->head->len-PACKET_HEADER_LEN<=ackno){
			stop_timer();
			Node* temp=que->head;			
			que->head=que->head->next;
			
			free(temp->pkt);
			free(temp);
			temp=NULL;		
		}
		send_base=ackno;
		
		if (que->head==NULL) que->tail=NULL;
		else start_timer();
	}
}

// send buffer, buf, of length len

ull ok=0;
void rdt_send(const void *buf, size_t len)
{	
	ok++;
	
	if (qu==NULL){
		qu=(Queue*) malloc(sizeof(Queue));
		qu->head=NULL;
		qu->tail=NULL;
		
	}
	
	make_pkt(buf, seq_no);
	udt_send(buf,len);
	insert(buf, len);
	
	if (seq_no==send_base){
		start_timer();
	}
	seq_no+=(len-PACKET_HEADER_LEN);
	if (ok==1) sleep(1);
}

// received an acknowledgment number, ackno
void rdt_recv_ack(unsigned long long ackno)
{
	del(qu,ackno);
}

// timeout event handler
void timeout()
{
	if(qu->head!=NULL){
		udt_send(qu->head->pkt, qu->head->len);
		start_timer();
		
	}
}
