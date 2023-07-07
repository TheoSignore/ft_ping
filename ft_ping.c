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

void	mmcpy(void* src, void* dst, size_t size)
{
	for (size_t i = 0 ; i < size ; i++)
		((unsigned char*)dst)[i] = ((unsigned char*)src)[i];
}

void	zerocalcare(void* ptr, size_t size)
{
	for (size_t i = 0 ; i < size ; i++)
		((char*)ptr)[i] = 0;
}

size_t	ft_strlen(const char* str)
{
	size_t	res = 0;
	while (str[res])
		res++;
	return res;
}

typedef struct sockaddr_in	sain_t;

int	get_target(const char* tgt, sain_t* sain)
{
	int	ret = inet_pton(AF_INET, tgt, (void*)&(sain->sin_addr));
	sain->sin_family = AF_INET;
	if (ret != 1)
	{
		struct addrinfo*	addr_nfo = NULL;
		ret = getaddrinfo(tgt, NULL, NULL, &addr_nfo);
		if (ret != 0)
		{
			dprintf(STDERR_FILENO, "ping: %s: %s\n", tgt, gai_strerror(ret));
			return (1);
		}
		sain->sin_addr.s_addr = ((struct sockaddr_in*)addr_nfo->ai_addr)->sin_addr.s_addr;
		freeaddrinfo(addr_nfo);
	}
	return (0);
}

//printf("ip: %s\n", inet_ntoa(addr->sin_addr));
int	main(int ac, char** av)
{
	if (ac == 0)
	{
		printf("I need something to ping\n");
		return (1);
	}
	printf ("PID: %i\n", getpid());

	struct sockaddr_in	lipton;
	zerocalcare(&lipton, sizeof(lipton));
	int ret = get_target(av[1], &lipton);
	if (ret)
		return (1);

	int	suck = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	if (suck == -1)
	{
		perror("socket");
		return (1);
	}
	ret = 1;
	ret = setsockopt(suck, IPPROTO_IP, IP_RECVTTL, &ret, sizeof(ret));
	if (ret != 0)
	{
		perror("setsockoption");
		return (1);
	}

	struct icmphdr icmp_hdr;
	zerocalcare(&icmp_hdr, sizeof(icmp_hdr));
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = getpid();
	icmp_hdr.un.echo.sequence = 1;

	char*	text = "Lorem ipsum dolor sit amet, consectetur adipiscing odio.";
	size_t	textsize = ft_strlen(text);
	size_t	datasize = sizeof(icmp_hdr) + textsize;
	unsigned char	data[datasize];

	mmcpy(&icmp_hdr, data, sizeof(icmp_hdr));
	mmcpy(text, data + sizeof(icmp_hdr), textsize);


	ret = sendto(suck, data, datasize, 0, (struct sockaddr*)&lipton, sizeof(lipton));
	if (ret == -1)
	{
		perror("sendto");
		return (1);
	}

	struct msghdr	msg_hdr;
	zerocalcare(&msg_hdr, sizeof(msg_hdr));

	char	msg_name[64];
	zerocalcare(msg_name, 64);
	msg_hdr.msg_name = msg_name;
	msg_hdr.msg_namelen = 64;

	msg_hdr.msg_iov = NULL;

	char	msgname[40];
	zerocalcare(msgname, 40);
	msg_hdr.msg_name = msgname;
	msg_hdr.msg_namelen = 40;

	char	msgctrl[1024];
	zerocalcare(msgctrl, 1024);
	msg_hdr.msg_control = msgctrl;
	msg_hdr.msg_controllen = 1024;

	struct iovec	shit[1];
	shit[0].iov_len = 64;
	shit[0].iov_base = malloc(64);

	msg_hdr.msg_iov = shit;
	msg_hdr.msg_iovlen = 1;


	int	rcvd = recvmsg(suck, &msg_hdr, 0);
	if (rcvd == -1)
		return (1);
	(void)rcvd;
	printf("%s - %s\n", inet_ntoa(lipton.sin_addr), inet_ntoa(((struct sockaddr_in*)msg_hdr.msg_name)->sin_addr));

	struct icmphdr	icmprpl;
	char			buffer[57];
	zerocalcare(buffer, 57);
	mmcpy(shit[0].iov_base, &icmprpl, sizeof(struct icmphdr));
	mmcpy(shit[0].iov_base + sizeof(struct icmphdr), &buffer, 56);

	if (icmprpl.type == ICMP_ECHOREPLY)
		printf("%i: %s\n",icmprpl.un.echo.sequence, buffer);
	else
		printf("FUCK\n");

	close(suck);
}

typedef struct s_icmpecho
{
	size_t			len;
	unsigned char	type;
	char*			fqdn;
	char			ip[33];
	unsigned int	icmp_seq;
	char*			data;
	size_t			datalen;
}	icmpecho_t;

int	get_ttl(struct msghdr* msg_hdr)
{
	int	ttl = -1;
	struct cmsghdr*	cmsg;
	for (cmsg = CMSG_FIRSTHDR(msg_hdr) ; cmsg != NULL ; cmsg = CMSG_NXTHDR(msg_hdr, cmsg))
	{
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL)
			mmcpy(CMSG_DATA(cmsg), &ttl, sizeof(ttl));
	}
	return (ttl);
}


