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

int	receive_icmp_reply(struct msghdr* msg_hdr, ping_t** pings, int res)
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
		printf("%i bytes from %s: icmp_seq=%i ttl=%i time=%.3f ms\n", res,inet_ntoa(((struct sockaddr_in*)msg_hdr->msg_name)->sin_addr) , icmprpl.un.echo.sequence, get_ttl(msg_hdr), time_to_fms(&(replied_ping->delay)));
	}
	else if (icmprpl.type == ICMP_ECHO)
	{
		struct cmsghdr*	cmsg;
		struct sock_extended_err seerr;
		for (cmsg = CMSG_FIRSTHDR(msg_hdr) ; cmsg != NULL ; cmsg = CMSG_NXTHDR(msg_hdr, cmsg))
		{
			if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVERR)
				mmcpy(CMSG_DATA(cmsg), &seerr, sizeof(seerr));
		}
		printf("num: %i\t%s\nogn: %i %i\ntyp: %i\ncod: %i\nnfo: %i\ndat: %i\n",
				seerr.ee_errno,
				strerror(seerr.ee_errno),
				seerr.ee_origin,
				SO_EE_ORIGIN_ICMP,
				seerr.ee_type,
				seerr.ee_code,
				seerr.ee_info,
				seerr.ee_data);
		printf("%i %i %s\n", ((struct sockaddr_in*)SO_EE_OFFENDER(&seerr))->sin_family, AF_UNSPEC, inet_ntoa(((struct sockaddr_in*)SO_EE_OFFENDER(&seerr))->sin_addr));
		printf("[%i] %i bytes from %s: icmp_seq=%i :unrechabl\n", icmprpl.type, res,inet_ntoa(((struct sockaddr_in*)msg_hdr->msg_name)->sin_addr) , icmprpl.un.echo.sequence);
	}
	return (0);
}
