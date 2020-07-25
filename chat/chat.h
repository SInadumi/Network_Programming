#ifndef CHAT_H_
#define CHAT_H_

#include "mynet.h"

/* mainから呼び出す関数 */
void chat_server(int port_number, int n_client);

/* クライアントの初期化を行う関数 */
void init_client(int sock_listen, int n_client);

/* メインループ */
void chat_loop();

/* Accept関数(エラー処理つき) */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);

/* 送信関数(エラー処理つき) */
int Send(int s, void *buf, size_t len, int flags);

/* 受信関数(エラー処理つき) */
int Recv(int s, void *buf, size_t len, int flags);

#endif