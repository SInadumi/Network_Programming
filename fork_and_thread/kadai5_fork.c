#include "mynet.h"
#include <sys/wait.h>

#define BUFSIZE 50   /* バッファサイズ */

int main(int argc, char *argv[])
{
  int port_number;
  int sock_listen, sock_accepted;
  int n_process = 0;
  pid_t child;
  char buf[BUFSIZE];
  int PRCS_LIMIT; /* プロセス数制限 */
  char error_msg[BUFSIZE] = {"block now\n"};
  int strsize;

  /* 引数のチェックと使用法の表示 */
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s Port_number\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port_number = atoi(argv[1]);
  PRCS_LIMIT = atoi(argv[2]);

  /* サーバの初期化 */
  sock_listen = init_tcpserver(port_number, 5);
  //子プロセスの生成
  for (int i = 0; i < PRCS_LIMIT; i++)
  {
    if ((child = fork()) == 0)
      break;
    else if (child > 0)
    {
      /*parent's process*/
      n_process++;
      printf("child:%d\n", child);
    }
    else if (child < 0)
    {
      /* fork()に失敗 */
      close(sock_listen);
      exit_errmesg("fork()");
    }
  }
  //子プロセスをすべて生成するまで，親プロセスは待機．
  if (child > 0)
    sleep(5);

  for (;;)
  {
    sock_accepted = accept(sock_listen, NULL, NULL);
    if (child == 0)
    {
      /* Child process */
      do
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
      } while (buf[strsize - 1] != '\n'); /* 改行コードを受信するまで繰り返す */
    }
    else if (child > 0)
    {
      if (send(sock_accepted, error_msg, BUFSIZE, 0) == -1)
      {
        exit_errmesg("send()");
      }
    }
    close(sock_accepted);
  }

  //子プロセスの回収
  if (child == 0)
    exit(EXIT_SUCCESS);
  while ((child = waitpid(-1, NULL, WNOHANG)) > 0)
  {
    n_process--;
  }
  close(sock_listen);
}

/* never reached */