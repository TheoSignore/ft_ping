#include "ft_ping.h"

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
		perror("ft_ping: malloc()");
	return (res);
}

void	dgram_dump(dgram_t* dgram, size_t size)
{
	hexdump_iphdr(&(dgram->ip_hdr));
	print_iphdr(&(dgram->ip_hdr));
	print_icmp(&(dgram->icmp_hdr), size);
}
