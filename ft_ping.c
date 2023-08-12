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

int	icmp_socket(void)
{
	int	res;
	int	one = 1;
	int	suckit = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (suckit == -1)
	{
		perror("ping: socket()");
		return (-1);
	}
	res = setsockopt(suckit, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
	if (!res)
	{
		res = setsockopt(suckit, IPPROTO_IP, IP_RECVERR, &one, sizeof(one));
		if (res)
			perror("ping: setsockopt(,,IP_RECVERR,,)");
	}
	else
		perror("ping: setsockopt(,,IP_HDRINCL,,)");
	if (res == 0)
		return (suckit);
	close(suckit);
	return (-1);
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
	if (!sig)
		alarm(1);
}

int	main(int ac, char** av)
{
	if (ac < 2 || ac > 3)
	{
		dprintf(STDERR_FILENO, "Usage: ft_ping [-v] <IPv4 | FQDN>\n");
		return (1);
	}

	char	vrb = 1;
	struct sockaddr_in	target;
	char				target_type = 0;
	zerocalcare(&target, sizeof(target));
	int ret = get_target(av[1], &target, &target_type);
	if (ret)
		return (1);
	
	int	suckit = icmp_socket();
	if (suckit == -1)
		return (1);

	ping_t*	first_ping = NULL;

	signal(SIGINT, &handolo);

	dgram_t*	dgram = create_dgram(&target);
	if (!dgram)
	{
		close(suckit);
		return (1);
	}

	set_icmp_echo(suckit, dgram, &target, &first_ping);
	signal(SIGALRM, &sendsig);

	if (vrb)
		printf("PING %s (%s): %zu bytes of data, id 0x%x = %i\n", av[1], (target_type ? inet_ntoa(target.sin_addr) : av[1]), ICMP_ECHO_SIZE - sizeof(struct icmphdr), getpid(), getpid());
	else
		printf("PING %s (%s): %zu bytes of data.\n", av[1], (target_type ? inet_ntoa(target.sin_addr) : av[1]), ICMP_ECHO_SIZE - sizeof(struct icmphdr));
	alarm(1);

	struct msghdr*	msg_hdr = alloc_msghdr();
	int	res = 0;
	while(!sig)
	{
		res = recvmsg(suckit, msg_hdr, MSG_DONTWAIT);
		if(res > 0)
		{
			receive_icmp_reply(msg_hdr, &first_ping, res);
		}
	}
	close(suckit);
	free(msg_hdr);
	summary_t	summary;
	get_summary(first_ping, &summary);
	free_pings(first_ping);
	printf("--- %s ping statistics ---\n%zu packets transmitted, %zu received, %zu%% packet loss\n round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			av[1],
			summary.transmitted,
			summary.received,
			summary.loss,
			time_to_fms(&(summary.min)),
			time_to_fms(&(summary.avg)),
			time_to_fms(&(summary.max)),
			time_to_fms(&(summary.mdev)));
	return (0);
}
