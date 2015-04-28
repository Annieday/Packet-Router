#include "router.h"

int main(int argc, char *argv[]) {
	FILE *rout_fp;
	FILE *stat_fp;
	char *ep;
	u_long port;
	struct sockaddr_in si_router, si_server;
	int s, count = 1, slen=sizeof(si_server);
	char buffer[BUFLEN];

	if (argc != 4)
		usage();

	port = strtoul(argv[1], &ep, 10);

	if (argv[1] == '\0' || *ep != '\0'){
		fprintf(stderr, "error: [%s] is not a vaild port!\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	rout_fp = fopen(argv[2],"r");

	if ( rout_fp == NULL) {
		fprintf(stderr, "error: cannot open [%s] file!\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	stat_fp = fopen(argv[3],"w");

	if ( stat_fp == NULL) {
		fprintf(stderr, "error: cannot open [%s] file!\n", argv[3]);
		exit(EXIT_FAILURE);
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr,"Client: cannot open socket\n");
		exit(EXIT_FAILURE);
	}

	memset((char *) &si_router, 0, sizeof(si_router));
	si_router.sin_family = AF_INET;
	si_router.sin_port = htons(port);
	si_router.sin_addr.s_addr = htonl(IP);

	if (bind(s, (struct sockaddr *)&si_router, sizeof(si_router)) == -1) {
		fprintf(stderr,"Error in binding the socket\n");
		exit(EXIT_FAILURE);
	}
	log_s = malloc(sizeof(router_log_s));
	bzero(log_s,sizeof(router_log_s));
	read_table(rout_fp);
	
	(void) signal(SIGINT, sig_handler);

	while (stop_flag) {
		if (recvfrom(s, buffer, BUFLEN, 0,
			(struct sockaddr *)&si_server, (socklen_t*)&slen)==-1){
			fprintf(stderr,"Error in receive.\n");
			continue;
		} else{
			process_packet(buffer);
			if(count%20 == 0)
				update_stat_logfile(stat_fp);
			count++;
		}
	}

	free(log_s);
	update_stat_logfile(stat_fp);
	fclose(rout_fp);
	fclose(stat_fp);
	close(s);
	return 0;
}

void sig_handler(int sig)
{
	stop_flag = 0;
	exit(0);
	
}

static void usage() {
	extern char * __progname;
	fprintf(stderr,"usage: %s <port number> <routing table file path> <statistics file path> \n",
		__progname);
	exit(EXIT_FAILURE);
}

void update_stat_logfile(FILE *fp) {
	fseek(fp, 0, SEEK_SET);
	fprintf(fp, "expired packets: %d\n", log_s->expired);
	fprintf(fp, "unroutable packets: %d\n", log_s->unroutable);
	fprintf(fp, "delivered direct: %d\n", log_s->delivered);
	fprintf(fp, "router B: %d\n", log_s->router_B);
	fprintf(fp, "router C: %d\n", log_s->router_C);
	fflush(fp);
}
in_addr_t to_mask(int n){
	int r = 0;
	for(int i = 0; i< n; i++){
		r = r | (1 << i);
	} 
	r = r << (32-n);
	return (in_addr_t) r;
}

void read_table(FILE *rout_fp) {
	char line[BUFLEN];
	char net_addr[100];
	int i = 0;
	bzero(routing_table,sizeof(router_table_s));

	while(fgets(line,sizeof(line), rout_fp)) {
		if(line[0]=='\n' || line[0]=='\r' || line[0]=='\0')
			continue;
		sscanf(line,"%s%d%s", net_addr,
				&routing_table[i].prefix_len,
				routing_table[i].nexthop);
		routing_table[i].net_addr = inet_network(net_addr);
		
		i++;
	}
}

void process_packet(char buffer[BUFLEN]) {
	int		i,j,pkt_id,ttl,unroutable = 1;
	char	dest_ip[100];
	in_addr_t	dest;
	in_addr_t	mask;
	char *token = strtok(buffer,",");
	for(i = 0; i < 4; i++){
		if(i == 0)
			pkt_id = atoi(token);
		if(i == 2)
			strcpy(dest_ip,token);
		if(i == 3)
			ttl = atoi(token);
		token = strtok(NULL,",");
	}
	dest = inet_network(dest_ip); 
	if(ttl <= 1) {
		log_s->expired++;
	} else{
		for(j = 0; j < 3; j++){
			mask = to_mask(routing_table[j].prefix_len);
			int s = (int) routing_table[j].net_addr ^ (int) dest;
			int m = s & (int) mask;
			if(m == 0){
				if(strncmp(routing_table[j].nexthop,"0",1)==0) {
					log_s->delivered++;
					fprintf(stdout,
						"Delivering direct: packet ID=%d, dest=%s\n",
						pkt_id, dest_ip);
				}
				else if(strncmp(routing_table[j].nexthop,
							"RouterB",7) == 0) {
					log_s->router_B++;
				}
				else if(strncmp(routing_table[j].nexthop,
							"RouterC",7) == 0) {
					log_s->router_C++;
				}
				unroutable = 0;
				break;
			}
		}
		if(unroutable)
			log_s->unroutable++;
	}
		
}
