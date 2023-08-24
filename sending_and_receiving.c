#include "ft_ping.h"

static int	icmp_echo(int socket, dgram_t* dgram, sain_t* targetptr, ping_t** first)
{
	static int		suckit;
	static dgram_t*	data;
	static sain_t*	target;

	static ping_t**	pings;
	struct timeval	tv;
	int				ret;

	if (dgram)
	{
		suckit = socket;
		data = dgram;
		target = targetptr;
		pings = first;
		return (0);
	}

	if (!pings)
		return (-2);

	set_icmp(&(data->icmp_hdr), NULL);
	ret = sendto(suckit, data, sizeof(dgram_t), 0, (struct sockaddr*)target, sizeof(sain_t));
	gettimeofday(&tv, NULL);
	if(add_ping(pings, data->icmp_hdr.un.echo.sequence, tv.tv_sec, tv.tv_usec))
		sig = 1;
	return (ret);
}

void	set_icmp_echo(int socket, dgram_t* dgram, sain_t* targetptr, ping_t** pings)
{
	icmp_echo(socket, dgram, targetptr, pings);
}

int		send_icmp_echo(void)
{
	return icmp_echo(-1, NULL, NULL, NULL);
}

static int	get_fqdn(host_t* src)
{
	int		res = EAI_OVERFLOW;
	size_t	size = 32;

	src->fqdn = NULL;
	while (res == EAI_OVERFLOW)
	{
		free(src->fqdn);
		size *= 2;
		src->fqdn = malloc(size);
		if (src->fqdn)
		{
			res = getnameinfo((struct sockaddr*)&(src->addr), sizeof(sain_t), src->fqdn, size, NULL, 0, NI_NAMEREQD);
		}
		else
		{
			perror("ft_ping: malloc()");
			return (EAI_FAIL);
		}
	}
	if (res)
	{
		free(src->fqdn);
		src->fqdn = NULL;
	}
	return (res);
}

int	receive_icmp_reply(struct msghdr* msg_hdr, ping_t** pings, int bytes, host_t* target, char vrb, uint16_t id)
{
	dgram_t*	dgram;
	tv_t		tv;
	ping_t		*replied_ping;
	host_t	source;

	gettimeofday(&tv, NULL);
	dgram = msg_hdr->msg_iov->iov_base;
	source.addr.sin_family = AF_INET;
	source.addr.sin_port = 0;
	source.addr.sin_addr.s_addr = dgram->ip_hdr.saddr;

	if (dgram->icmp_hdr.type == ICMP_ECHO && id != dgram->icmp_hdr.un.echo.id)
		return (0);
	if (dgram->ip_hdr.saddr != target->addr.sin_addr.s_addr)
	{
		get_fqdn(&source);
		ipv4_ntoa(source.addr.sin_addr.s_addr, source.ip);
		printf("%li bytes from %s (%s): ",
			bytes - sizeof(struct iphdr),
			(source.fqdn ? source.fqdn : source.ip),
			source.ip
		);
		free(source.fqdn);
	}
	else
	{
		printf("%li bytes from %s: ",
			bytes - sizeof(struct iphdr),
			target->ip
		);
	}

	if (dgram->icmp_hdr.type == ICMP_ECHOREPLY)
	{
		if (verify_checksum(&(dgram->icmp_hdr)))
		{
			replied_ping = note_reply(*pings, dgram->icmp_hdr.un.echo.sequence, tv.tv_sec, tv.tv_usec);
			if (replied_ping)
			{
				printf("icmp_seq=%i ttl=%i time=%.3f ms\n",
						invert_bytes(dgram->icmp_hdr.un.echo.sequence),
						dgram->ip_hdr.ttl,
						time_to_fms(&(replied_ping->delay))
					);
			}
			else
				printf("Unknown ICMP Sequence %i\n", invert_bytes(dgram->icmp_hdr.un.echo.sequence));
		}

	}
	else if (dgram->icmp_hdr.type == ICMP_DEST_UNREACH)
	{
		switch(dgram->icmp_hdr.code)
		{
			case ICMP_NET_UNREACH:
				printf("Destination Network Unreachable\n");
				break;
			case ICMP_HOST_UNREACH:
				printf("Destination Host Unreachable\n");
				break;
			case ICMP_PROT_UNREACH:
				printf("Destination Protocol Unreachable\n");
				break;
			case ICMP_PORT_UNREACH:
				printf("Destination Port Unreachable\n");
				break;
			case ICMP_FRAG_NEEDED:
				printf("Fragmentation Needed/DF set\n");
				break;
			default:
				printf("Something went wrong\n");
		}
		if (vrb)
		{
			dgram_t*	ptr = (void*)dgram->data;
			dgram_dump(ptr, bytes - (sizeof(struct iphdr) * 2) - sizeof(struct icmphdr));
		}
	}
	else
		printf("Unhandled ICMP type\n");
	return (0);
}
