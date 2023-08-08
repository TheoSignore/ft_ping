#include "ft_ping.h"

void	mmcpy(void* src, void* dst, size_t size)
{
	for (size_t i = 0 ; i < size ; i++)
		((unsigned char*)dst)[i] = ((unsigned char*)src)[i];
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
