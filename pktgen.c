#include "pktgen.h"

int main(int argc, char *argv[]) {
	FILE *log_fp;
	int count = 1;
	int s;
	char buffer[BUFLEN];
	//char source[15];
	//char dest[15];
	char *ep;
	u_long port;
	pktgen_s * pkt_s;
	struct sockaddr_in si_server;

	if (argc != 3)
		usage();

	port = strtoul(argv[1], &ep, 10);

	if (argv[1] == '\0' || *ep != '\0') {
		fprintf(stderr, "error: [%s] is not a vaild port!\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	log_fp = fopen(argv[2],"w+");

	if ( log_fp == NULL) {
		fprintf(stderr, "error: cannot open [%s] file!\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr,"Client: cannot open socket\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&si_server, 0, sizeof(si_server));
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(port);
	si_server.sin_addr.s_addr = htonl(IP);

	pktgen_log_s *log_s = malloc(sizeof(pktgen_log_s));
	bzero(log_s,sizeof(pktgen_log_s));
	(void) signal(SIGINT, sig_handler);

	while (stop_flag) {
		pkt_s = genterat_new_pktgen(count);
		sprintf(buffer, "%d,%s,%s,%d,\"%s\"",
			pkt_s->pkt_id, pkt_s->source_ip, pkt_s->dest_ip,
			pkt_s->ttl, pkt_s->payload);
		sendto(s, buffer, strlen(buffer) + 1, 0,
			(struct sockaddr *)&si_server, sizeof(si_server));
		if(count % 2 == 0)
			sleep(2);
		if(s_index<=1) {
			if(d_index>1 && d_index<=4) {
				log_s->atob++;
			} else if(d_index>4 && d_index<=8) {
				log_s->atoc++;
			} else{
				log_s->inva++;
			}	
		}
		if(s_index>1 && s_index<=4) {
			if(d_index<=1) {
				log_s->btoa++;
			} else if(d_index>4 && d_index<=8) {
				log_s->btoc++;
			} else {
				log_s->inva++;
			}
		}
		if(s_index>4 && s_index<=8) {
			if(d_index<=1) {
				log_s->ctoa++;
			}else if(d_index>1 && d_index<=4) {
				log_s->ctob++;
			} else {
				log_s->inva++;
			}
		}
		if (count % 20 == 0) {
			update_pkt_logfile(log_s, log_fp);
		}
		count++;

	}
	update_pkt_logfile(log_s, log_fp);
	fclose(log_fp);
	close(s);
	return 0;
}

pktgen_s * genterat_new_pktgen(uint32_t pkg_id)
{
	s_index = rand() % 9;
	d_index = rand() % 10;

	while(s_index == d_index 
		|| (s_index<=1 && d_index<=1)
		|| (s_index>1 && s_index<=4 && d_index>1 && d_index<=4)
		|| (s_index>4 && s_index<=8 && d_index>4 && d_index<=8))
		d_index = rand() % 10;
	
	pktgen_s * pkt_s = malloc(sizeof(pktgen_s));
	bzero(pkt_s,sizeof(pktgen_s));
	pkt_s->payload = malloc(100 * sizeof(char));
	pkt_s->source_ip = malloc(100 * sizeof(char));
	pkt_s->dest_ip = malloc(100 * sizeof(char));

	pkt_s->pkt_id = pkg_id;
	strcpy(pkt_s->source_ip, host[s_index]);
	strcpy(pkt_s->dest_ip, host[d_index]);
	pkt_s->ttl = 1 + rand() % 4;
	strcpy(pkt_s->payload, "Hello World!");
		
	return pkt_s;
}

void update_pkt_logfile(pktgen_log_s *log_s, FILE *fp)
{
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "NetA to NetB: %d\n", log_s->atob);
	fprintf(fp, "NetA to NetC: %d\n", log_s->atoc);
	fprintf(fp, "NetB to NetA: %d\n", log_s->btoa);
	fprintf(fp, "NetB to NetC: %d\n", log_s->btoc);
	fprintf(fp, "NetC to NetA: %d\n", log_s->ctoa);
	fprintf(fp, "NetC to NetB: %d\n", log_s->ctob);
	fprintf(fp, "Invalid Destination: %d\n", log_s->inva);
	fflush(fp);
}
