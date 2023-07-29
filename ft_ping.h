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

#ifndef ICMPREQ_SIZE
# define ICMPREQ_SIZE 64
# define ICMPECHO_DATA "Lorem ipsum dolor sit amet, consectetur adipiscing odio."
#endif

#define MSGHDR_NAMELEN 40
#define MSGHDR_CONTROLLEN 32
#define MSGHDR_IOV_BASELEN 64
void	mmcpy(void* src, void* dst, size_t size);
void	zerocalcare(void* ptr, size_t size);
size_t	ft_strlen(const char* str);

typedef struct sockaddr_in	sain_t;

typedef struct s_ping
{
	size_t				seq;
	struct timeval_t	sent;
	struct timeval_t	rcvd;
	struct s_ping*		next;
}	ping_t;

typedef struct s_summary
{
	struct timeval_t*	min;
	struct timeval_t*	max;
	struct timeval_t	avg;
	struct timeval_t	mdev;
	size_t				transmitted;
	size_t				received;
	int					loss; // (received * 100) / transmitted
	struct timeval_t	time; // max(rcvd) - min(rcvd)
}	summary_t;

void	add_ping(ping_t* first, int seq, time_t seconds, suseconds_t micro);
void	note_reply(ping_t* first, size_t sequence, time_t seconds, suseconds_t micro);
void	get_summary(ping_t* first, struct timeval_t** min, struct timeval_t** max, struct timeval_t* avg, struct timeval_t* mdev, 

void	set_icmp_echo(int socket, sain_t* targetptr, ping_t* pings);
int		send_icmp_echo(void);
