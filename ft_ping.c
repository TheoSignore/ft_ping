#include "ft_ping.h"

char	sig = 0;

int	get_target(const char* tgt, sain_t* sain, char* target_type)
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
		*target_type = 1;
	}
	return (0);
}

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

int	icmp_socket(void)
{
	int	suckit = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	if (suckit == -1)
	{
		perror("ping: socket()");
		return (-1);
	}
	int	ret = 1;
	ret = setsockopt(suckit, IPPROTO_IP, IP_RECVTTL, &ret, sizeof(ret));
	if (ret == 0)
		return (suckit);
	close(suckit);
	perror("ping: setsockopt()");
	return (-1);
}


void	build_msghdr(struct msghdr* msg_hdr, char* name, char* control, struct iovec* yovek)
{
	zerocalcare(msg_hdr, sizeof(msg_hdr));
	msg_hdr->msg_name = name;
	zerocalcare(msg_hdr->msg_name, MSGHDR_NAMELEN);
	msg_hdr->msg_namelen = MSGHDR_NAMELEN;

	msg_hdr->msg_control = control;
	zerocalcare(msg_hdr->msg_control, MSGHDR_CONTROLLEN);
	msg_hdr->msg_controllen = MSGHDR_CONTROLLEN;

	msg_hdr->msg_iov = yovek;
	msg_hdr->msg_iovlen = 1;
}

void	handolo(int fuck)
{
	(void)fuck;
	sig = 1;
}

void	sendsig(int fuck)
{
	(void)fuck;
	send_icmp_echo();
	alarm(1);
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

	struct sockaddr_in	target;
	char				target_type = 0;
	zerocalcare(&target, sizeof(target));
	int ret = get_target(av[1], &target, &target_type);
	if (ret)
		return (1);
	
	int	suckit = icmp_socket();
	if (suckit == -1)
		return (1);

	struct msghdr	msg_hdr;
	char			msghdr_name[MSGHDR_NAMELEN];
	char			msghdr_ctrl[MSGHDR_CONTROLLEN];
	char			msghdr_iovbase[MSGHDR_IOV_BASELEN];
	struct iovec	yovek;
	yovek.iov_base = msghdr_iovbase;
	yovek.iov_len = MSGHDR_IOV_BASELEN;
	build_msghdr(&msg_hdr, msghdr_name, msghdr_ctrl, &yovek);

	signal(SIGINT, &handolo);
	signal(SIGALRM, &sendsig);
	alarm(1);

	printf("PING %s (%s) %zu(%zu) bytes of data.\n", av[1], (target_type ? inet_ntoa(target.sin_addr) : av[1]), ICMPREQ_SIZE - sizeof(struct icmphdr), (size_t)ICMPREQ_SIZE);

	size_t		nbr_reply_rcvd = 0;

	struct timeval	min;
	struct timeval	max;
	struct timeval	avg;
	struct timeval	mdev;

	int	rcvd;
	while(!sig)
	{
		rcvd = recvmsg(suckit, &msg_hdr, 0);
		if (rcvd != -1)
		{
			struct icmphdr	icmprpl;
			char			buffer[57];
			zerocalcare(buffer, 57);
			mmcpy(msg_hdr.msg_iov->iov_base, &icmprpl, sizeof(struct icmphdr));
			mmcpy(msg_hdr.msg_iov->iov_base + sizeof(struct icmphdr), &buffer, 56);

			if (icmprpl.type == ICMP_ECHOREPLY)
			{
				if (target_type)
					printf("%i bytes from %s (%s): icmp_seq=%i ttl=%i\n", rcvd, av[1], inet_ntoa(((struct sockaddr_in*)msg_hdr.msg_name)->sin_addr) , icmprpl.un.echo.sequence, get_ttl(&msg_hdr));
				else
					printf("%i bytes from %s: icmp_seq=%i ttl=%i\n", rcvd,inet_ntoa(((struct sockaddr_in*)msg_hdr.msg_name)->sin_addr) , icmprpl.un.echo.sequence, get_ttl(&msg_hdr));
			}
			else
				printf("FUCK\n");
		}
	}
	summary();
	close(suckit);
	free_shit();
	return (0);
}
