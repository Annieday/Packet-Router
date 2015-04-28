#include "config.h"
#define BUFLEN 512
#define IP 2130706433  /* 127.0.0.1 */

typedef struct {
    uint32_t	pkt_id;
    char	*source_ip;
    char	*dest_ip;
    uint8_t	ttl;
    char	*payload;

} pktgen_s;


typedef struct {
    uint32_t     atob;
    uint32_t     atoc;
    uint32_t     btoa;
    uint32_t     btoc;
    uint32_t     ctoa;
    uint32_t     ctob;
    uint32_t     inva;
} pktgen_log_s;

static void usage()
{
	extern char * __progname;
	fprintf(stderr,"usage: %s <port number>  <packets file path> \n",
		__progname);
	exit(EXIT_FAILURE);

}

int stop_flag = 1;

static void sig_handler(int sig)
{
	stop_flag = 0;
	
}

const char *host[10] = {"192.168.128.7","192.168.128.1","192.168.192.10",
				"192.168.192.6","192.168.192.4","192.224.0.5",
				"192.224.0.7","192.224.0.5","192.224.15.6","168.130.192.01"};
int s_index;
int d_index;
pktgen_s * genterat_new_pktgen(uint32_t pkg_id);
void update_pkt_logfile(pktgen_log_s *log_s, FILE *fp);


