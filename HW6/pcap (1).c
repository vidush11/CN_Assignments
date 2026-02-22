// Source: https://www.devdungeon.com/content/using-libpcap-c

#include <pcap.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <netinet/ip.h>


// options points to the first byte of TCP header options
// len is the length of the TCP header options
// print TCP SACK acknowledgements in this routine

int c=0;
int r1=0;
int r2=0;
int r3=0;
void print_tcp_options(unsigned char *options, size_t len)
{
	int byte=0;
	while (byte<(int)len){
		unsigned char kind=options[byte];
		printf("Kind-%d\n", kind);
		if (kind==0) break;
		else if (kind==1){
			byte++;
		}
		else if (kind==5){
			unsigned char len=options[byte+1];
			int ranges=(len-2)/8;
			if (ranges==1) r1++;
			else if (ranges==2) r2++;
			else if (ranges==3) r3++;
			byte+=2;
			for (int i=0; i<ranges; i++){
				unsigned int * l=(unsigned int*)(options+byte);
				unsigned int * r=(unsigned int*)(options+byte+4);
	 			printf("Range %d %u-%u, 1->%d, 2->%d, 3->%d\n", ++c, ntohl(*l), ntohl(*r), r1, r2, r3);
				byte+=8;
			}
		}
		else{
			unsigned char len=options[byte+1];
			byte+=len;
		}
	
	
	
	}
}

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	struct ethhdr *eth;
	eth = (struct ethhdr *) packet;
	if (ntohs(eth->h_proto) != ETHERTYPE_IP) {
		return;
	}

	struct iphdr *iph = (struct iphdr*)((char*)eth + sizeof(struct ethhdr));

	if (iph->protocol != IPPROTO_TCP) {
		// not a TCP packet
		return;
	}
	struct tcphdr *tcp = (struct tcphdr*)((char*)iph + (iph->ihl * 4));
	int length = tcp->doff * 4;
	if (length > 20) {
		unsigned char *options = (((unsigned char*)tcp) + 20);
		print_tcp_options(options, length - 20);
	}
}

int main(int argc, char *argv[])
{
	pcap_t *handle;			/* Session handle */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
	pcap_if_t *alldevs;
	pcap_if_t *d;

	// Find all devices
	if (pcap_findalldevs(&alldevs, errbuf) == -1) {
		fprintf(stderr, "Error finding devices: %s\n", errbuf);
		return 1;
	}

	if (alldevs == NULL) {
		fprintf(stderr, "no available device\n");
		return 1;
	}

	// Print the list
	printf("Available network interfaces:\n");
	for (d = alldevs; d != NULL; d = d->next) {
		printf("%s", d->name);
		if (d->description) {
			printf(" - %s\n", d->description);
		}
		else {
			printf(" - No description available\n");
		}
	}
	/* Define the device */
	handle = pcap_open_live(alldevs->name, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open default device: %s\n", alldevs->name);
		pcap_freealldevs(alldevs);
		return(2);
	}
	printf("Could open: %s\n", alldevs->name);

	pcap_loop(handle, 0, packet_handler, NULL);

	/* And close the session */
	pcap_close(handle);
	pcap_freealldevs(alldevs);
	return(0);
}
