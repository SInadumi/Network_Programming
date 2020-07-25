/*
  echo_server3th.c (Thread版)
*/
#include "mynet.h"
#include <pthread.h>

#define BUFSIZE 50   /* バッファサイズ */

void *echo_thread(void *arg);

int main(int argc, char *argv[])
{
  int port_number;
  int sock_listen, sock_accepted;
  int *tharg;
  pthread_t tid;
  int PRCS_LIMIT; /* プロセス数制限 */
  char error_msg[BUFSIZE] = {"block now\n"};

  /* 引数のチェックと使用法の表示 */
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s Port_number\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port_number = atoi(argv[1]); /* 引数の取得 */
  PRCS_LIMIT = atoi(argv[2]);

  /* サーバの初期化 */
  sock_listen = init_tcpserver(port_number, 5);
  for (int i = 0; i < PRCS_LIMIT; i++)
  {

    /* スレッド関数の引数を用意する */
    if ((tharg = (int *)malloc(sizeof(int))) == NULL)
    {
      exit_errmesg("malloc()");
    }

    *tharg = sock_listen;

    /* スレッドを生成する */
    if (pthread_create(&tid, NULL, echo_thread, (void *)tharg) != 0)
      exit_errmesg("pthread_create()");
  }
  for (;;)
  {
    /*parent's process 待機*/
    sock_accepted = accept(sock_listen, NULL, NULL);
    if (send(sock_accepted, error_msg, BUFSIZE, 0) == -1)
    {
      exit_errmesg("send()");
    }
    close(sock_accepted);
  }

  /* never reached */
}

/* スレッドの本体 */
void *echo_thread(void *arg)
{
  int sock;
  char buf[BUFSIZE];
  int strsize;
  int sock_accepted;

  sock = *((int *)arg);
  free(arg); /* 引数用のメモリを開放 */
  for (;;)
  {
    /* クライアントの接続を受け付ける */
    sock_accepted = accept(sock, NULL, NULL);
    pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

    do
    {

      {
        /* 文字列をクライアントから受信する */
        if ((strsize = recv(sock_accepted, buf, BUFSIZE, 0)) == -1)
        {
          exit_errmesg("recv()");
        }

        /* 文字列をクライアントに送信する */
        if (send(sock_accepted, buf, strsize, 0) == -1)
        {
          exit_errmesg("send()");
        }
      }
    } while (buf[strsize - 1] != '\n'); /* 改行コードを受信するまで繰り返す */

    close(sock_accepted); /* ソケットを閉じる */
  }

  return (NULL);
}