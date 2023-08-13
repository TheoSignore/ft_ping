#include "ft_ping.h"

void	add_ping(ping_t** first, int seq, time_t seconds, suseconds_t micro)
{
	ping_t*	ptr = malloc(sizeof(ping_t));
	ptr->seq = seq;
	ptr->sent.tv_sec = seconds;
	ptr->sent.tv_usec = micro;
	ptr->rcvd.tv_sec = 0;
	ptr->rcvd.tv_usec = 0;
	ptr->next = (*first);
	*first = ptr;
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

void	get_mdev(ping_t* first, tv_t* avg, tv_t* mdev, size_t nbr)
{
	tv_t	tmp;
	mdev->tv_sec = 0;
	mdev->tv_usec = 0;
	while (first)
	{
		if (first->seq == (size_t)-1)
		{
			if (time_lwr(avg, &(first->delay)))
				time_diff(&(first->delay), avg, &tmp);
			else
				time_diff(avg, &(first->delay), &tmp);
			time_sum(mdev, &tmp, mdev);
		}
		first = first->next;
	}
	time_div(mdev, nbr, mdev);
}

void	get_summary(ping_t* first, summary_t* summary)
{
	ping_t	*tmp = first;

	//min
	summary->min.tv_sec = LONG_MAX;
	summary->min.tv_usec = LONG_MAX;
	//max
	summary->max.tv_sec = 0;
	summary->max.tv_usec = 0;
	//avg
	tv_t	delay_sum;
	delay_sum.tv_sec = 0;
	delay_sum.tv_usec = 0;

	//transmitted
	summary->transmitted = 0;
	//received
	summary->received = 0;
	while(first)
	{
		summary->transmitted++;
		if (first->seq == (size_t)-1)
		{
			summary->received++;
			if (time_lwr(&(first->delay), &(summary->min)))
				time_cpy(&(summary->min), &(first->delay));
			else if (time_grt(&(first->delay), &(summary->max)))
				time_cpy(&(summary->max), &(first->delay));

			time_sum(&delay_sum, &(first->delay), &delay_sum);
		}
		first = first->next;
	}
	first = tmp;
	if (summary->received)
	{
		time_div(&delay_sum, summary->received, &(summary->avg));
		get_mdev(first, &(summary->avg), &(summary->mdev), summary->received);
		summary->loss = ((summary->received * 100) / summary->transmitted) - 100;
	}
	else
		summary->loss = 100;
	
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
