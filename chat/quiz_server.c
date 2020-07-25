/*
  quiz_server.c
*/
#include "quiz.h"
#include "mynet.h"
#include <stdlib.h>
#include <unistd.h>

void quiz_server(int port_number, int n_client)
{
    int sock_listen;

    /* サーバの初期化 */
    sock_listen = init_tcpserver(port_number, 5);
    /* クライアントの接続 */
    init_client(sock_listen, n_client);
    
    close(sock_listen);

    /* メインループ */
    question_loop();

}