SRCS	:=	ft_ping.c \
			sending_and_receiving.c \
			dgram.c \
			icmphdr.c \
			iphdr.c \
			msghdr.c \
			summary.c \
			pings.c \
			time_stuff.c \
			utils.c

OBJS	=	${SRCS:%.c=.%.o}

DEP		=	${SRCS:%.c=.%.d}

FLAGS	:=  -Wall -Wextra -Werror -MMD
#FLAGS	:=  -Wall -Wextra -Werror -g -MMD
#FLAGS	:=  -Wall -Wextra -Werror -g -MMD -fsanitize=address

COMPILO	:= gcc

NAME	:=	ft_ping

.SUFFIXES: .c .o .d

.%.o: %.c
		${COMPILO} ${FLAGS} -c $< -o $@

all:	${NAME}

clean:
		rm -f ${OBJS}
		rm -f ${DEP}

fclean:		clean
		rm -f ${NAME}

${NAME}: ${OBJS}
		${COMPILO} ${FLAGS} -o ${NAME} ${OBJS}

-include ${DEP}

re:		fclean all

.PHONY:		all clean fclean re dependencies
