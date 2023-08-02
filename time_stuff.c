#include "ft_ping.h"
/*
 * getting a bit tired of making my own time utilities all the time
 */

void	time_diff(tv_t* a, tv_t* b, tv_t* c)
{
	c->tv_sec = a->tv_sec - b->tv_sec;
	c->tv_usec = a->tv_usec - b->tv_usec;
}

void	time_div(tv_t* a, size_t nb, tv_t* c)
{
	c->tv_sec = a->tv_sec / nb;
	c->tv_usec = a->tv_usec + ((a->tv_sec % nb) * 1000000);
	c->tv_usec /= nb;
}

void	time_sum(tv_t* a, tv_t* b, tv_t* c)
{
	tv_t	res;
	res.tv_sec = a->tv_sec + b->tv_sec;
	res.tv_usec = a->tv_usec + b->tv_usec;
	if (res.tv_usec >= 1000000)
	{
		res.tv_sec += res.tv_usec / 1000000;
		res.tv_usec = res.tv_usec % 1000000;
	}
	c->tv_sec = res.tv_sec;
	c->tv_usec = res.tv_usec;
}

int		time_grt(tv_t* a, tv_t* b)
{
	if (a->tv_sec == b->tv_sec)
		return (a->tv_usec > b->tv_usec);
	return (a->tv_sec > b->tv_sec);
}

int		time_lwr(tv_t* a, tv_t* b)
{
	if (a->tv_sec == b->tv_sec)
		return (a->tv_usec < b->tv_usec);
	return (a->tv_sec < b->tv_sec);
}

size_t	time_to_ims(tv_t* a)
{
	size_t res = a->tv_sec * 1000 + a->tv_usec / 1000;
	return res;
}

double	time_to_fms(tv_t* a)
{
	double	res = a->tv_sec * 1000 + a->tv_usec / 1000.0;
	return res;
}

void	time_cpy(tv_t* a, tv_t* b)
{
	a->tv_sec = b->tv_sec;
	a->tv_usec = b->tv_usec;
}
