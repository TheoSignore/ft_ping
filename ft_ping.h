#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <limits.h>
#include <linux/errqueue.h>

#define ICMP_ECHO_SIZE 64
#define ICMP_DATA_SIZE (ICMP_ECHO_SIZE - sizeof(struct icmphdr))
#define ICMP_ECHO_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing odio."
#define DGRAM_SIZE (sizeof(struct iphdr) + ICMP_ECHO_SIZE)

#define MSGHDR_NAMELEN sizeof(struct sockaddr_in)
#define MSGHDR_CONTROLLEN 64
#define MSGHDR_IOV_BASELEN sizeof(struct iphdr) + ICMP_ECHO_SIZE + 64
#define MSGHDR_TOTAL_SIZE (sizeof(struct msghdr) + sizeof(struct iovec) + MSGHDR_NAMELEN + MSGHDR_CONTROLLEN + MSGHDR_IOV_BASELEN)
#define MSGHDR_IOV_OFFSET (sizeof(struct msghdr))
#define MSGHDR_NAME_OFFSET (MSGHDR_IOV_OFFSET + sizeof(struct iovec))
#define MSGHDR_CONTROL_OFFSET (MSGHDR_NAME_OFFSET + MSGHDR_NAMELEN)
#define MSGHDR_IOVBASE_OFFSET (MSGHDR_CONTROL_OFFSET + MSGHDR_CONTROLLEN)

typedef struct timeval		tv_t;
typedef struct sockaddr_in	sain_t;

typedef struct s_ping
{
	size_t			seq;
	tv_t			sent;
	tv_t			rcvd;
	tv_t			delay;
	struct s_ping*	next;
}	ping_t;

typedef struct s_dgram
{
	struct iphdr	ip_hdr;
	struct icmphdr	icmp_hdr;
	char			data[ICMP_DATA_SIZE];
}	dgram_t;

typedef struct s_summary
{
	tv_t	min;
	tv_t	max;
	tv_t	avg;
	tv_t	mdev;
	size_t	transmitted;
	size_t	received;
	size_t	loss;
}	summary_t;

typedef struct s_target
{
	char*	fqdn;
	char	ip[16];
	sain_t	addr;
}	target_t;

extern int	sig;

//dgram.c
dgram_t*	create_dgram(sain_t* target);
void		dgram_dump(dgram_t* dgram, size_t size);

//icmphdr.c
int		verify_checksum(struct icmphdr* icmp_hdr);
void	set_icmp(struct icmphdr* icmp_hdr, char* data);
void	print_icmp(const struct icmphdr* icmp_hdr, size_t size);

//iphdr.c
void	set_iphdr(struct iphdr* ip_hdr, const sain_t* target);
void	hexdump_iphdr(const struct iphdr* ip_hdr);
void	print_iphdr(struct iphdr* ip_hdr);

//msghdr.c
struct msghdr*	alloc_msghdr(void);

//pings.c
int		add_ping(ping_t** first, int seq, time_t seconds, suseconds_t micro);
ping_t*	note_reply(ping_t* first, size_t sequence, time_t seconds, suseconds_t micro);
void	free_pings(ping_t* first_ping);

//sending_and_receiving
void	set_icmp_echo(int socket, dgram_t* dgram, sain_t* targetptr, ping_t** pings);
int		send_icmp_echo(void);
int		receive_icmp_reply(struct msghdr* msg_hdr, ping_t** pings, int res, target_t* target, char vrb, uint16_t);

//summary.c
void	get_summary(ping_t* first, summary_t* summary);
void	print_summary(summary_t* summary, const char* arg);

//time_stuff.c
void	time_diff(tv_t* a, tv_t* b, tv_t* res);
void	time_div(tv_t* a, size_t nb, tv_t* c);
void	time_sum(tv_t* a, tv_t* b, tv_t* res);
int		time_grt(tv_t* a, tv_t* b);
int		time_lwr(tv_t* a, tv_t* b);
size_t	time_to_ims(tv_t*);
double	time_to_fms(tv_t*);
void	time_cpy(tv_t* a, tv_t* b);

//utils.c
void		mmcpy(const void* src, void* dst, size_t size);
void		zerocalcare(void* ptr, size_t size);
size_t		ft_strlen(const char* str);
size_t		ipv4_ntoa(uint32_t s_addr, char* dst);
uint16_t	invert_bytes(uint16_t n);
