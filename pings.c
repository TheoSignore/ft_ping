#include "ft_ping.h"

int	add_ping(ping_t** first, int seq, time_t seconds, suseconds_t micro)
{
	ping_t*	ptr = malloc(sizeof(ping_t));
	if (ptr)
	{
		ptr->seq = seq;
		ptr->sent.tv_sec = seconds;
		ptr->sent.tv_usec = micro;
		ptr->rcvd.tv_sec = 0;
		ptr->rcvd.tv_usec = 0;
		ptr->next = (*first);
		*first = ptr;
		return (0);
	}
	perror("ft_ping: malloc()");
	return (1);
}

ping_t*	note_reply(ping_t* first, size_t sequence, time_t seconds, suseconds_t micro)
{
	while (first && first->seq != sequence)
		first = first->next;
	if (first)
	{
		first->seq = -1;
		first->rcvd.tv_sec = seconds;
		first->rcvd.tv_usec = micro;
		time_diff(&(first->rcvd), &(first->sent), &(first->delay));
		return (first);
	}
	return (NULL);
}

void	free_pings(ping_t* ptr)
{
	ping_t*	tmp;

	while (ptr)
	{
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
	}
}
