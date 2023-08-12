#include "ft_ping.h"


static void	set_iphdr(struct iphdr* ip_hdr, const sain_t* target)
{
	uint16_t	sum;

	ip_hdr->version = 4;
	ip_hdr->ihl = 5;
	ip_hdr->tos = 0;
	ip_hdr->tot_len = sizeof(dgram_t);
	ip_hdr->id = 0;
	ip_hdr->frag_off = 64;
	ip_hdr->ttl = 64;
	ip_hdr->protocol = IPPROTO_ICMP;
	ip_hdr->check = 0;
	ip_hdr->saddr = 0;
	ip_hdr->daddr = target->sin_addr.s_addr;

	sum = 0;
	for(size_t i = 0; i < (sizeof(struct iphdr) / 2) ; i++)
		sum += ((uint16_t*)ip_hdr)[i];
	ip_hdr->check = ~sum;
}

static uint16_t	invert_bytes(uint16_t n)
{
	//__be16	id = getpid();
	//__u8	c = ((__u8*)&id)[1];
	//((__u8*)&id)[1] = ((__u8*)&id)[0];
	//((__u8*)&id)[0] = c;
	uint8_t	c = ((uint8_t*)&n)[1];
	((uint8_t*)&n)[1] = ((uint8_t*)&n)[0];
	((uint8_t*)&n)[0] = c;
	return n;
}

static uint16_t	icmp_checksum(struct icmphdr* icmp_hdr)
{
	uint16_t	sum0;
	uint16_t	sum1;
	uint16_t	sum;

	icmp_hdr->checksum = 0;
	sum0 = 0;
	sum1 = 0;
	for (size_t i = 0; i < (ICMP_ECHO_SIZE / 2); i++)
	{
		sum0 += invert_bytes(((uint16_t*)icmp_hdr)[i]);
		sum1 += ((uint16_t*)icmp_hdr)[i];
	}

	((uint8_t*)&sum)[0] = ((uint8_t*)&sum0)[1];
	((uint8_t*)&sum)[1] = ((uint8_t*)&sum1)[1];
	return (~(sum));
}

static void	set_icmp(struct icmphdr* icmp_hdr, char* data)
{
	static uint16_t	seq = 0;

	if (data)
	{
		icmp_hdr->type = ICMP_ECHO;
		icmp_hdr->code = 0;
		icmp_hdr->checksum = 0;
		icmp_hdr->un.echo.id = invert_bytes(getpid());
		icmp_hdr->un.echo.sequence = 0;
		mmcpy(ICMP_ECHO_DATA, data, ICMP_DATA_SIZE);
		return;
	}
	icmp_hdr->un.echo.sequence = invert_bytes(seq);
	seq++;
	icmp_hdr->checksum = icmp_checksum(icmp_hdr);
}

dgram_t*	create_dgram(sain_t* target)
{
	dgram_t*	res;
	
	res = malloc(sizeof(dgram_t));
	if (res)
	{
		set_iphdr(&(res->ip_hdr), target);
		set_icmp(&(res->icmp_hdr), res->data);
	}
	else
		perror("ping: malloc()");
	return (res);
}

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
	add_ping(pings, data->icmp_hdr.un.echo.sequence, tv.tv_sec, tv.tv_usec);
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

int	receive_icmp_reply(struct msghdr* msg_hdr, ping_t** pings, int bytes)
{
	dgram_t*		dgram;
	tv_t			tv;
	ping_t			*replied_ping;
	sain_t			source;
	char			fqdn[128];
	int				res;

	gettimeofday(&tv, NULL);
	dgram = msg_hdr->msg_iov->iov_base;
	source.sin_family = AF_INET;
	source.sin_port = 0;
	source.sin_addr.s_addr = dgram->ip_hdr.saddr;

	res = getnameinfo((struct sockaddr*)&source, sizeof(sain_t), fqdn, 128, NULL, 0, NI_NAMEREQD);

	printf("%li bytes from %s: ",
			bytes - sizeof(struct iphdr),
			(res ? inet_ntoa(source.sin_addr) : fqdn)
		);

	if (dgram->icmp_hdr.type == ICMP_ECHOREPLY)
	{
		replied_ping = note_reply(*pings, dgram->icmp_hdr.un.echo.sequence, tv.tv_sec, tv.tv_usec);
		printf("icmp_seq=%i ttl=%i time=%.3f ms\n", dgram->icmp_hdr.un.echo.sequence, dgram->ip_hdr.ttl, time_to_fms(&(replied_ping->delay)));

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
	}
	return (0);
}
