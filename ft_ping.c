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

int	main(int ac, char** av)
{
	if (ac == 0)
	{
		printf("I need something to ping\n");
		return (1);
	}
//	struct icmphdr icmp_hdr;
	//struct addrinfo*	res = NULL;
	//struct addrinfo		hint;
	//zerocalcare(&hint, sizeof(hint));
	//hint.ai_family = AF_INET;
	//hint.ai_socktype = SOCK_DGRAM;
	//hint.ai_protocol = IPPROTO_ICMP;
	//int	ret = getaddrinfo(av[1], NULL, &hint, &res);
	//if (ret != 0)
	//{
	//	printf("%s\n", gai_strerror(ret));
	//	return (1);
	//}

	struct sockaddr_in	*addr;
	struct sockaddr_in	lipton;
	zerocalcare(&lipton, sizeof(lipton));
	int	ret = inet_pton(AF_INET, av[1], (void*)&(lipton.sin_addr));
	lipton.sin_family = AF_INET;
	if (ret != 1)
	{
		struct addrinfo*	addr_nfo;
		zerocalcare(&addr_nfo, sizeof(addr_nfo));
		ret = getaddrinfo(av[1], NULL, NULL, &addr_nfo);
		if (ret != 0)
		{
			printf("getaddrinfo: %s\n", gai_strerror(ret));
			return (1);
		}
		addr = (struct sockaddr_in*)addr_nfo->ai_addr;
		addr->sin_family = AF_INET;
	}
	else
		addr = &lipton;

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


	ret = sendto(suck, data, datasize, 0, (struct sockaddr*)addr, sizeof(addr));
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

	char	msg_ctrl[1024];
	zerocalcare(msg_ctrl, 1024);
	msg_hdr.msg_control = msg_ctrl;
	msg_hdr.msg_controllen = 1024;

	size_t	rcvd = recvmsg(suck, &msg_hdr, 0);
	printf("recmsg: %zu\n", rcvd);

	close(suck);
}
