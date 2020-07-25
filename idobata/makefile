#
# Makefile for libmynet
#
#OBJS = init_tcpserver.o init_tcpclient.o other.o init_udpserver.o init_udpclient.o
#AR = ar -qc

#libmynet.a : ${OBJS}
#	${RM} $@
#	${AR} $@ ${OBJS}

#${OBJS}: mynet.h

#clean:
#	${RM} *.o
MYLIBDIR=../mynet/lib/
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}
OBJS=ido.o ido_client.o ido_server.o 
all: ido
ido: ${OBJS}
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}
clean:
	${RM} *.o ido *~