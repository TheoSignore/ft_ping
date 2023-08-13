#include "ft_ping.h"

static void	set_iphdr(struct iphdr* ip_hdr, const sain_t* target)
{
	//uint16_t	sum;

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

	//sum = 0;
	//for(size_t i = 0; i < (sizeof(struct iphdr) / 2) ; i++)
	//	sum += ((uint16_t*)ip_hdr)[i];
	//ip_hdr->check = ~sum;
}

static uint16_t	invert_bytes(uint16_t n)
{
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

void	set_icmp(struct icmphdr* icmp_hdr, char* data)
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
