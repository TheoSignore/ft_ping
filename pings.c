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

void	note_reply(ping_t* first, size_t sequence, time_t seconds, suseconds_t micro)
{
	while (first && first->seq != sequence)
		first = first->next;
	if (first)
	{
		first->rcvd.tv_sec = seconds;
		first->rcvd.tv_usec = micro;
	}
}

static void	rcvd_min_max(ping_t* first, tv_t** min, tv_t** max)
{
	*min = &(first->rcvd);
	*max = &(first->rcvd);
	first = first->next;
	while(first)
	{
		if (time_lwr(&(first->rcvd), *min))
			*min = &(first->rcvd);
		else if (time_grt(&(first->rcvd), *max))
			*min = &(first->rcvd);
		first = first->next;
	}
}

static size_t	calculate_diffs(ping_t* first)
{
	size_t	diffs = 0;
	while (first)
	{
		if (time_lwr(&(first->sent), &(first->rcvd)))
		{
			time_diff(&(first->rcvd), &(first->sent), &(first->sent));
			diffs++;
			first->seq = -1;
		}
		first = first->next;
	}
	return (diffs);
}

static void	diffs_to_avg(ping_t* first, tv_t* avg)
{
	while (first)
	{
		if (first->seq == -1)
		{
			if (time_lwr(&(first->sent), avg))
				time_diff(avg, &(first->sent), &(first->sent));
			else
				time_diff(&(first->sent), avg, &(first->sent));
		}
		first = first->next;
	}
}

static void	get_mdev(ping_t* first, tv_t* mdev, size_t nbr)
{
	mdev->tv_sec = 0;
	mdev->tv_usec = 0;
	while (first)
	{
		if (first->seq == -1)
			time_sum(mdev, &(first->sent), mdev);
		first = first->next;
	}
	time_div(mdev, nbr, mdev);
}

void	get_summary(ping_t* first, summary_t* summary)
{
	tv_t*	rcvd_min;
	tv_t*	rcvd_max;
	rcvd_min_max(first, &rcvd_min, &rcvd_max);
	time_diff(rcvd_max, rcvd_min, &(summary->time));

	summary->received = calculate_diffs(first);

	tv_t	tvmax;
	tv_t	tvmin;

	tvmin.tv_sec = 0;
	tvmin.tv_usec = 0;
	tvmax.tv_sec = -1;
	tvmax.tv_usec = -1;

	summary->min = &tvmax;
	summary->max = &tvmin;
	summary->transmitted = 0;

	tv_t	diffsum;
	diffsum.tv_sec = 0;
	diffsum.tv_usec = 0;
	while (first)
	{
		summary->transmitted++;
		if (first->seq == -1)
		{
			time_sum(&diffsum, &(first->sent), &diffsum);
			if(time_lwr(&(first->sent), summary->min))
				summary->min = &(first->sent);
			else if (time_grt(&(first->sent), summary->max))
				summary->max = &(first->sent);
		}
		first = first->next;
	}
	summary->loss = (summary->received * 100) / summary->transmitted;
	time_div(&diffsum, summary->received, &(summary->avg));
	diffs_to_avg(first, &(summary->avg));
	get_mdev(first, &(summary->mdev), summary->received);
}
