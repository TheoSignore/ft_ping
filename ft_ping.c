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

	struct sockaddr_in	lipton;
	zerocalcare(&lipton, sizeof(lipton));
	int ret = get_target(av[1], &lipton);
	if (ret)
		return (1);

	int	suck = socket(AF_INET, SOCK_DGRAM, 1);
	if (suck == -1)
	{
		perror("socket");
		return (1);
	}

	struct icmphdr icmp_hdr;
	zerocalcare(&icmp_hdr, sizeof(icmp_hdr));
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = 42;

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

	char	msgname[1024];
	zerocalcare(msgname, 1024);
	msg_hdr.msg_name = msgname;
	msg_hdr.msg_namelen = 1024;

	size_t	rcvd = recvmsg(suck, &msg_hdr, 0);
	(void)rcvd;
	printf("%s - %s\n", inet_ntoa(lipton.sin_addr), inet_ntoa(((struct sockaddr_in*)msg_hdr.msg_name)->sin_addr));

	close(suck);
}
