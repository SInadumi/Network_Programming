#include <stdio.h>
#include <stdlib.h>
#include "mynet.h"

void exit_errmesg(char *errmesg)
{
  perror(errmesg);
  exit(EXIT_FAILURE);
}

/* 関数acceptのWrapper関数 */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int r;
  if((r=accept(s,addr,addrlen))== -1){
    exit_errmesg("accept()");
  }
  return(r);
}
/* 関数sendのWrapper関数 */
int Send(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=send(s,buf,len,flags))== -1){
    exit_errmesg("send()");
  }
  return(r);
}
/* 関数recvのWrapper関数 */
int Recv(int s, void *buf, size_t len, int flags)
{
  int r;
  if((r=recv(s,buf,len,flags))== -1){
    exit_errmesg("recv()");
  }
  return(r);
}

int Sendto( int sock, const void *s_buf, size_t strsize, int flags, const struct sockaddr *to, socklen_t tolen)
{
  int r;
  if( (r=sendto(sock, s_buf, strsize, 0, to, tolen))== -1){
    exit_errmesg("sendto()");
  }

  return(r);
}

int Recvfrom(int sock, void *r_buf, size_t len, int flags,
       struct sockaddr *from, socklen_t *fromlen)
{
  int r;
  if((r=recvfrom(sock, r_buf, len, 0, from, fromlen))== -1){
    exit_errmesg("recvfrom()");
  }

  return(r);
}