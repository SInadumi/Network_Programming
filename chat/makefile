MYLIBDIR=../mynet/chat
MYLIB=-lmynet
CFLAGS=-I${MYLIBDIR} -L${MYLIBDIR}
OBJS=chat.o chat_util.o
all: chat
chat: ${OBJS}
   ${CC} ${CFLAGS} -o $@ $^ ${MYLIB}
clean:
   ${RM} *.o chat.o chat_util.o *~