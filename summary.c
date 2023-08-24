#include "ft_ping.h"

static void	get_mdev(ping_t* first, tv_t* avg, tv_t* mdev, size_t nbr)
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
		summary->loss = 100 - ((summary->received * 100) / summary->transmitted);
	}
	else
		summary->loss = 100;
}

void	print_summary(summary_t* summary, const char* arg)
{
	printf("--- %s ping statistics ---\n%zu packets transmitted, %zu received, %zu%% packet loss\n",
			arg,
			summary->transmitted,
			summary->received,
			summary->loss
		);
	if (summary->received)
	{
		printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
			time_to_fms(&(summary->min)),
			time_to_fms(&(summary->avg)),
			time_to_fms(&(summary->max)),
			time_to_fms(&(summary->mdev))
			);
	}
}
