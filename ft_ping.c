#include "ft_ping.h"

int	sig = 0;

static int	get_target(char* arg, host_t* target)
{
	int	ret = inet_pton(AF_INET, arg, (void*)&(target->addr.sin_addr));
	target->addr.sin_family = AF_INET;
	if (ret != 1)
	{
		struct addrinfo*	addr_nfo = NULL;
		ret = getaddrinfo(arg, NULL, NULL, &addr_nfo);
		if (ret != 0)
		{
			dprintf(STDERR_FILENO, "ft_ping: %s: %s\n", arg, gai_strerror(ret));
			return (1);
		}
		target->fqdn = arg;
		target->addr.sin_addr.s_addr = ((struct sockaddr_in*)addr_nfo->ai_addr)->sin_addr.s_addr;
		ipv4_ntoa(target->addr.sin_addr.s_addr, target->ip);
		freeaddrinfo(addr_nfo);
	}
	else
	{
		mmcpy(arg, target->ip, ft_strlen(arg));
		target->fqdn = NULL;
	}
	return (0);
}

static int	icmp_socket(void)
{
	int	res;
	int	one = 1;
	int	suckit = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (suckit == -1)
	{
		perror("ft_ping: socket()");
		return (-1);
	}
	res = setsockopt(suckit, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
	if (!res)
	{
		res = setsockopt(suckit, IPPROTO_IP, IP_RECVERR, &one, sizeof(one));
		if (res)
			perror("ft_ping: setsockopt(,,IP_RECVERR,,)");
	}
	else
		perror("ft_ping: setsockopt(,,IP_HDRINCL,,)");
	if (res == 0)
		return (suckit);
	close(suckit);
	return (-1);
}

void	signal_handler(int signum)
{
	if (signum == SIGINT)
		sig = 1;
	else
	{
		send_icmp_echo();
		if (!sig)
			alarm(1);
	}
}

int	main(int ac, char** av)
{
	if (ac < 2 || ac > 3)
	{
		dprintf(STDERR_FILENO, "ft_ping: invalid usage\nTry 'ft_ping -?' for more information\n");
		return (1);
	}

	// I don't have in mind to add more options
	int vrb = 0;
	if (ac == 3)
	{
		if (av[1][0] == '-' && av[1][1] == 'v')
		{
			vrb = 1;
			av[1] = av[2];
		}
		else
		{
			dprintf(STDERR_FILENO, "ft_ping: invalid usage\nTry 'ft_ping -?' for more information\n");
			return (1);
		}
	}
	else if (ac == 2 && av[1][0] == '-' && av[1][1] == '?')
	{
		printf("-- ft_ping --\nSend ICMP ECHO_REQUEST packets to a network host.\nUsage:\n  ft_ping HOST    | standard usage\n  ft_ping -v HOST | verbose mode\n  ft_ping -?      | display this help\n");
		return (0);
	}

	//get info on targeted host
	host_t	target;
	zerocalcare(&target, sizeof(target));
	int ret = get_target(av[1], &target);
	if (ret)
		return (1);

	//opening and configuring socket
	int	suckit = icmp_socket();
	if (suckit == -1)
		return (1);

	//allocating and building datagram: IP and ICMP headers, and data
	dgram_t*	dgram = create_dgram(&(target.addr));
	if (!dgram)
	{
		close(suckit);
		return (1);
	}

	//pointer to the first ping of the ping list
	ping_t*	first_ping = NULL;

	//passing the values to initialise the statics in the sender function
	set_icmp_echo(suckit, dgram, &(target.addr), &first_ping);

	//setting signal handlers (sigaction would be better but it's unauthorized)
	signal(SIGINT, &signal_handler);
	signal(SIGALRM, &signal_handler);

	printf("PING %s (%s): %zu data bytes", av[1], target.ip, ICMP_ECHO_SIZE - sizeof(struct icmphdr));
	if (vrb)
		printf(", id 0x%x = %i\n", getpid(), getpid());
	else
		printf(".\n");

	//allocating and setting message header for recvmsg()
	struct msghdr*	msg_hdr = alloc_msghdr();
	if (!msg_hdr)
	{
		free(dgram);
		close(suckit);
		return (1);
	}

	//starting the sender loop
	alarm(1);

	//starting the listening loop
	int	res = 0;
	while(!sig)
	{
		res = recvmsg(suckit, msg_hdr, MSG_DONTWAIT);
		if(res > 0)
		{
			receive_icmp_reply(msg_hdr, &first_ping, res, &target, vrb, dgram->icmp_hdr.un.echo.id);
		}
	}
	close(suckit);
	free(dgram);
	free(msg_hdr);

	//computing summary with the ping list
	summary_t	summary;
	get_summary(first_ping, &summary);

	free_pings(first_ping);

	print_summary(&summary, av[1]);
	return (0);
}
