#include "ft_ping.h"

static int	icmp_echo(int socket, struct sockaddr_in* targetptr, ping_t** first)
{
	static int					suckit;
	static struct sockaddr_in	target;
	static struct icmphdr		*icmp_hdr;
	static ping_t**				pings;
	static unsigned char		data[ICMPREQ_SIZE];
	static char					text[ICMPREQ_SIZE - sizeof(icmp_hdr)] = ICMPECHO_DATA;
	static size_t	seq = 1;
	int				ret;
	struct timeval	tv;

	if (targetptr)
	{
		pings = first;
		suckit = socket;
		mmcpy(targetptr, &target, sizeof(target));
		zerocalcare(&icmp_hdr, sizeof(icmp_hdr));
		icmp_hdr = (void*)data;
		icmp_hdr->type = ICMP_ECHO;
		icmp_hdr->un.echo.id = getpid();
		mmcpy(text, data + sizeof(icmp_hdr), ICMPREQ_SIZE - sizeof(icmp_hdr));
		return (0);
	}

	if (!pings)
		return (-2);
	icmp_hdr->un.echo.sequence = seq;

	ret = sendto(suckit, data, ICMPREQ_SIZE, 0, (struct sockaddr*)&target, sizeof(target));
	gettimeofday(&tv, NULL);
	add_ping(pings, seq, tv.tv_sec, tv.tv_usec);
	seq++;
	return (ret);
}

void	set_icmp_echo(int socket, sain_t* targetptr, ping_t** pings)
{
	icmp_echo(socket, targetptr, pings);
}

int		send_icmp_echo(void)
{
	return icmp_echo(-1, NULL, NULL);
}

int	receive_icmp_reply(struct msghdr* msg_hdr, char* tgt, int target_type, ping_t** pings, int res)
{
	struct icmphdr	icmprpl;
	char			buffer[57];
	tv_t			tv;
	ping_t			*replied_ping;

	gettimeofday(&tv, NULL);
	zerocalcare(buffer, 57);
	mmcpy(msg_hdr->msg_iov->iov_base, &icmprpl, sizeof(struct icmphdr));
	mmcpy(msg_hdr->msg_iov->iov_base + sizeof(struct icmphdr), &buffer, 56);

	if (icmprpl.type == ICMP_ECHOREPLY)
	{
		replied_ping = note_reply(*pings, icmprpl.un.echo.sequence, tv.tv_sec, tv.tv_usec);
		if (target_type)
			printf("%i bytes from %s (%s): icmp_seq=%i ttl=%i time=%.2f ms\n", res, tgt, inet_ntoa(((struct sockaddr_in*)msg_hdr->msg_name)->sin_addr) , icmprpl.un.echo.sequence, get_ttl(msg_hdr), time_to_fms(&(replied_ping->delay)));
		else
			printf("%i bytes from %s: icmp_seq=%i ttl=%i\n time=%.2f ms\n", res,inet_ntoa(((struct sockaddr_in*)msg_hdr->msg_name)->sin_addr) , icmprpl.un.echo.sequence, get_ttl(msg_hdr), time_to_fms(&(replied_ping->delay)));
	}
	else
		printf("FUCK\n");
	return (0);
}
