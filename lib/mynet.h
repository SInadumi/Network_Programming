/*
  mynet.h
*/
#ifndef MYNET_H_
#define MYNET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

int init_tcpserver(in_port_t myport, int backlog);
int init_tcpclient(char *servername, in_port_t serverport);
void exit_errmesg(char *errmesg);

/* 以下の5つの関数を追加 */
int init_udpserver(in_port_t myport);
int init_udpclient();
void set_sockaddr_in(struct sockaddr_in *server_adrs, 
		     char *servername, in_port_t port_number );
void set_sockaddr_in_broadcast(struct sockaddr_in *server_adrs,
			 in_port_t port_number );
int Sendto( int sock, const void *s_buf, size_t strsize, int flags, 
	    const struct sockaddr *to, socklen_t tolen);
int Recvfrom(int sock, void *r_buf, size_t len, int flags,
	     struct sockaddr *from, socklen_t *fromlen);
/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);
#endif  /* MYNET_H_ */