#include "ft_ping.h"

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

int	verify_checksum(struct icmphdr* icmp_hdr)
{
	uint16_t	rcvd_cksum = icmp_hdr->checksum;
	uint16_t	real_cksum = icmp_checksum(icmp_hdr);
	return (rcvd_cksum == real_cksum);
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

void	print_icmp(const struct icmphdr* icmp_hdr, size_t size)
{
	printf("ICMP: type %hhu, code %hhu, size %zu, id 0x%04hx, seq 0x%04hx\n",
			icmp_hdr->type,
			icmp_hdr->code,
			size,
			icmp_hdr->un.echo.id,
			icmp_hdr->un.echo.sequence
		);
}
