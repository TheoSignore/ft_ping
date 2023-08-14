#include "ft_ping.h"

void	set_iphdr(struct iphdr* ip_hdr, const sain_t* target)
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

void	hexdump_iphdr(const struct iphdr* ip_hdr)
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

/*
 * IP Hdr Dump:
 *  4500 0054 0000 4000 3f01 eff4 0a00 020f c0a8 7a02
 * ┌──┬──┬───┬────┬────┬───┬────┬───┬───┬────┬─────────┬─────────────┐
 * │Vr│HL│TOS│Len │ID  │Flg│off │TTL│Pro│cks │src      │dst          │
 * ├──┼──┼───┼────┼────┼───┼────┼───┼───┼────┼─────────┼─────────────┤
 * │ 4│ 5│ 00│0054│0000│  2│0000│ 3f│ 01│eff4│10.0.2.15│192.168.122.2│
 * └──┴──┴───┴────┴────┴───┴────┴───┴───┴────┴─────────┴─────────────┘
 */
void	print_iphdr(struct iphdr* ip_hdr)
{
	unsigned int	src_ip_len;
	unsigned int	dst_ip_len;
	char			dst_ip[16];
	char			src_ip[16];
	char*			line = "───────────────";
	size_t			linelen = ft_strlen(line);

	dst_ip_len = ipv4_ntoa(ip_hdr->daddr, dst_ip);
	src_ip_len = ipv4_ntoa(ip_hdr->saddr, src_ip);
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
			ip_hdr->version,
			ip_hdr->ihl,
			ip_hdr->tos,
			invert_bytes(ip_hdr->tot_len),
			ip_hdr->id,
			invert_bytes(ip_hdr->frag_off) >> 13,
			invert_bytes(ip_hdr->frag_off) & 8191,
			ip_hdr->ttl,
			ip_hdr->protocol,
			ip_hdr->check,
			src_ip,
			dst_ip
		);
	printf("└──┴──┴───┴────┴────┴───┴────┴───┴───┴────┴%s┴%s┘\nDATA:\n",
			&(line[linelen - (src_ip_len * 3)]),
			&(line[linelen - (dst_ip_len * 3)])
		);
	
}
