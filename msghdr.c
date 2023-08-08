#include "ft_ping.h"

/*
 * ┌─ ptr
 * │
 * │ struct msghdr
 * │
 * ├─ ptr + sizeof(struct msghdr)
 * │
 * │ msghdr.msg_iov
 * │
 * ├─ ptr + sizeof(struct msghdr) + sizeof(struct iovec)
 * │
 * │ msghdr.msg_name
 * │
 * ├─ ptr + sizeof(struct msghdr) + sizeof(struct iovec) + NAMELEN
 * │
 * │ msghdr.msg_control
 * │
 * ├─ ptr + sizeof(struct msghdr) + sizeof(struct iovec) + NAMELEN + CONTROLLEN
 * │
 * │ msghdr.msg_iov.base
 * │
 * └─
 */

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
	return (ptr);
}

int	get_ttl(struct msghdr* msg_hdr)
{
	int	ttl = -1;
	struct cmsghdr*	cmsg;
	for (cmsg = CMSG_FIRSTHDR(msg_hdr) ; cmsg != NULL ; cmsg = CMSG_NXTHDR(msg_hdr, cmsg))
	{
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL)
			mmcpy(CMSG_DATA(cmsg), &ttl, sizeof(ttl));
		else
			printf("lvl: (%i)%i\ttype: %i\n", IP_RECVERR, cmsg->cmsg_level, cmsg->cmsg_type);
	}
	return (ttl);
}
