#include "ft_ping.h"

static uint16_t	invert_bytes(uint16_t n)
{
	uint8_t	c = ((uint8_t*)&n)[1];
	((uint8_t*)&n)[1] = ((uint8_t*)&n)[0];
	((uint8_t*)&n)[0] = c;
	return n;
}

static void	set_iphdr(struct iphdr* ip_hdr, const sain_t* target)
{
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
}

/*
 * Let's take a moment to appreciate this piece of code
 *
 * It is the product of hours of empirical testing
 * I do not understand how it matches the RFC
 * Yet, it works
 */

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

static void	dump_iphdr(const struct iphdr* ip_hdr)
{
	printf("IP Hdr Dump:\n %04hx %04hx %04hx %04hx %04hx %04hx %04hx %04hx %04hx %04hx\n",
			invert_bytes(((const uint16_t*)ip_hdr)[0]),
			invert_bytes(((const uint16_t*)ip_hdr)[1]),
			((const uint16_t*)ip_hdr)[2],
			invert_bytes(((const uint16_t*)ip_hdr)[3]),
			invert_bytes(((const uint16_t*)ip_hdr)[4]),
			((const uint16_t*)ip_hdr)[5],
			invert_bytes(((const uint16_t*)ip_hdr)[6]),
			invert_bytes(((const uint16_t*)ip_hdr)[7]),
			invert_bytes(((const uint16_t*)ip_hdr)[8]),
			invert_bytes(((const uint16_t*)ip_hdr)[9])
		);
}

static void	print_icmp(const struct icmphdr* icmp_hdr, size_t size)
{
	printf("ICMP: type %hhu, code %hhu, size %zu, id 0x%04hx, seq 0x%04hx\n",
			icmp_hdr->type,
			icmp_hdr->code,
			size,
			icmp_hdr->un.echo.id,
			icmp_hdr->un.echo.sequence
		);
}

/*
 * IP Hdr Dump:
 *  4500 0054 0000 4000 3f01 eff4 0a00 020f c0a8 7a02
 * ┌──┬──┬───┬────┬────┬───┬────┬───┬───┬────┬─────────┬─────────────┐
 * │Vr│HL│TOS│Len │ID  │Flg│off │TTL│Pro│cks │src      │dst          │
 * ├──┼──┼───┼────┼────┼───┼────┼───┼───┼────┼─────────┼─────────────┤
 * │ 4│ 5│ 00│0054│0000│  2│0000│ 3f│ 01│eff4│10.0.2.15│192.168.122.2│
 * └──┴──┴───┴────┴────┴───┴────┴───┴───┴────┴─────────┴─────────────┘
 * ICMP: type 8, code 0, size 64, id 0xfb28, seq 0x0200
 */

void	dgram_dump(dgram_t* dgram, size_t size)
{
	unsigned int	src_ip_len;
	unsigned int	dst_ip_len;
	char			dst_ip[16];
	char			src_ip[16];
	char*			line = "───────────────";
	size_t			linelen = ft_strlen(line);
	(void)size;

	dst_ip_len = ipv4_ntoa(dgram->ip_hdr.daddr, dst_ip);
	src_ip_len = ipv4_ntoa(dgram->ip_hdr.saddr, src_ip);

	dump_iphdr(&(dgram->ip_hdr));
	printf("┌──┬──┬───┬────┬────┬───┬────┬───┬───┬────┬%s┬%s┐\n",
			&(line[linelen - (src_ip_len * 3)]),
			&(line[linelen - (dst_ip_len * 3)])
		);
	printf("│Vr│HL│TOS│Len │ID  │Flg│off │TTL│Pro│cks │%-*s│%-*s│\n",
			src_ip_len,
			"src",
			dst_ip_len,
			"dst"
		);
	printf("├──┼──┼───┼────┼────┼───┼────┼───┼───┼────┼%s┼%s┤\n",
			&(line[linelen - (src_ip_len * 3)]),
			&(line[linelen - (dst_ip_len * 3)])
		);
	printf("│%2hhx│%2hhx│ %02hhx│%04hx│%04hx│%3hx│%04hx│%3hhx│ %02hhx│%04hx│%s│%s│\n",
			dgram->ip_hdr.version,
			dgram->ip_hdr.ihl,
			dgram->ip_hdr.tos,
			invert_bytes(dgram->ip_hdr.tot_len),
			dgram->ip_hdr.id,
			invert_bytes(dgram->ip_hdr.frag_off) >> 13,
			invert_bytes(dgram->ip_hdr.frag_off) & 8191,
			dgram->ip_hdr.ttl,
			dgram->ip_hdr.protocol,
			dgram->ip_hdr.check,
			src_ip,
			dst_ip
		);
	printf("└──┴──┴───┴────┴────┴───┴────┴───┴───┴────┴%s┴%s┘\nDATA:\n",
			&(line[linelen - (src_ip_len * 3)]),
			&(line[linelen - (dst_ip_len * 3)])
		);
	print_icmp(&(dgram->icmp_hdr), size);
}
