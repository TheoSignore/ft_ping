#include "ft_ping.h"

void	mmcpy(const void* src, void* dst, size_t size)
{
	for (size_t i = 0 ; i < size ; i++)
		((unsigned char*)dst)[i] = ((const unsigned char*)src)[i];
}

void	zerocalcare(void* ptr, size_t size)
{
	for (size_t i = 0 ; i < size ; i++)
		((char*)ptr)[i] = 0;
}

size_t	ft_strlen(const char* str)
{
	size_t	res = 0;
	while (str[res])
		res++;
	return res;
}

size_t	ipv4_ntoa(uint32_t s_addr, char* dst)
{
	zerocalcare(dst, 16);
	return(sprintf(dst, "%hhu.%hhu.%hhu.%hhu",
			((uint8_t*)&(s_addr))[0],
			((uint8_t*)&(s_addr))[1],
			((uint8_t*)&(s_addr))[2],
			((uint8_t*)&(s_addr))[3]
			));
}

uint16_t	invert_bytes(uint16_t n)
{
	uint8_t	c = ((uint8_t*)&n)[1];
	((uint8_t*)&n)[1] = ((uint8_t*)&n)[0];
	((uint8_t*)&n)[0] = c;
	return n;
}
