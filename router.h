#include "config.h"
#define BUFLEN 512
#define IP 2130706433  /* 127.0.0.1 */
typedef struct {
    in_addr_t	net_addr;
    int		prefix_len;
    char	nexthop[100];

} router_table_s;


typedef struct {
    uint32_t     expired;
    uint32_t     unroutable;
    uint32_t     delivered;
    uint32_t     router_B;
    uint32_t     router_C;
} router_log_s;

router_table_s routing_table[3];
router_log_s *log_s;
int stop_flag = 1;

void sig_handler(int sig);
static void usage();
void update_stat_logfile(FILE *fp);
in_addr_t to_mask(int n);
void read_table(FILE *rout_fp);
void process_packet(char buffer[BUFLEN]);
