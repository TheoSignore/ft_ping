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

#ifndef ICMPREQ_SIZE
# define ICMPREQ_SIZE 64
# define ICMPECHO_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing odio."
#endif

#define MSGHDR_NAMELEN 40
#define MSGHDR_CONTROLLEN 32
#define MSGHDR_IOV_BASELEN 64
#define MSGHDR_TOTAL_SIZE (sizeof(struct msghdr) + sizeof(struct iovec) + MSGHDR_NAMELEN + MSGHDR_CONTROLLEN + MSGHDR_IOV_BASELEN)
#define MSGHDR_IOV_OFFSET (sizeof(struct msghdr))
#define MSGHDR_NAME_OFFSET (MSGHDR_IOV_OFFSET + sizeof(struct iovec))
#define MSGHDR_CONTROL_OFFSET (MSGHDR_NAME_OFFSET + MSGHDR_NAMELEN)
#define MSGHDR_IOVBASE_OFFSET (MSGHDR_CONTROL_OFFSET + MSGHDR_CONTROLLEN)


void	mmcpy(void* src, void* dst, size_t size);
void	zerocalcare(void* ptr, size_t size);
size_t	ft_strlen(const char* str);

typedef struct timeval		tv_t;

void	time_diff(tv_t* a, tv_t* b, tv_t* res);
void	time_div(tv_t* a, size_t nb, tv_t* c);
void	time_sum(tv_t* a, tv_t* b, tv_t* res);
int		time_grt(tv_t* a, tv_t* b);
int		time_lwr(tv_t* a, tv_t* b);
size_t	time_to_ims(tv_t*);
double	time_to_fms(tv_t*);
void	time_cpy(tv_t* a, tv_t* b);

typedef struct s_ping
{
	size_t			seq;
	tv_t			sent;
	tv_t			rcvd;
	tv_t			delay;
	struct s_ping*	next;
}	ping_t;

typedef struct s_summary
{
	tv_t	min;
	tv_t	max;
	tv_t	avg;
	tv_t	mdev;
	size_t	transmitted;
	size_t	received;
	size_t	loss; // (received * 100) / transmitted
	tv_t	time; // max(rcvd) - min(rcvd)
}	summary_t;

void	add_ping(ping_t** first, int seq, time_t seconds, suseconds_t micro);
ping_t*	note_reply(ping_t* first, size_t sequence, time_t seconds, suseconds_t micro);
void	get_summary(ping_t* first, summary_t* summary);
void	free_pings(ping_t* first_ping);

typedef struct sockaddr_in	sain_t;

struct msghdr*	alloc_msghdr(void);
int				get_ttl(struct msghdr* msg_hdr);

void	set_icmp_echo(int socket, sain_t* targetptr, ping_t** pings);
int		send_icmp_echo(void);
int		receive_icmp_reply(struct msghdr* msg_hdr, char* tgt, int target_type, ping_t** pings, int res);
