#
#makefile
#
#OBJS = init_tcpclient.o init_tcpserver.o other.o
#AR = ar -qc
#libmynet.a : ${OBJS}
#	${RM} $@
#	${AR} $@ ${OBJS}

#${OBJS}: mynet.h

#clean:
#	${RM} :*.o

MYLIBDIR=../mynet/fork_and_thread
MYLIB=-lmynet -lpthread
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}

all: kadai5_fork kadai5_thread test

kadai5_fork: kadai5_fork.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

kadai5_thread: kadai5_thread.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

test: test.o
	${CC} ${CFLAGS} -o $@ $^ ${MYLIB}

clean:
	${RM} *.o kadai5_fork kadai5_thread test *~