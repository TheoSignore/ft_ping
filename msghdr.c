#include "ft_ping.c"

struct msghdr*	alloc_msghdr(void)
{
	void*	ptr = malloc(MSGHDR_TOTAL_SIZE);
	if (!ptr)
		return (NULL);

	zerocalcare(ptr, MSGHDR_TOTAL_SIZE);

	((struct msghdr*)ptr)->msg_name = ptr + MSGHDR_NAME_OFFSET;
	((struct msghdr*)ptr)->msg_namelen = MSGHDR_NAMELEN;

	((struct msghdr*)ptr)->msg_control = ptr + MSGHDR_CONTROL_OFFSET;
	((struct msghdr*)ptr)->msg_controllen = MSGHDR_CONTROLLEN;

	((struct msghdr*)ptr)->msg_iov = ptr + MSGHDR_IOV_OFFSET;
	((struct msghdr*)ptr)->msg_iovlen = 1;

	((struct msghdr*)ptr)->msg_iov->iov_base = ptr + MSGHDR_IOVBASE_OFFSET;
	((struct msghdr*)ptr)->msg_iov->iov_len = MSGHDR_IOV_BASELEN;
}
