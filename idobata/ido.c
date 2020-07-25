/*
main.c
*/
#define _GNU_SOURCE      // sighandler_tを使用する為に定義
#include "mynet.h"
#include "ido.h"
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <signal.h>
#include <getopt.h>

#define BUFSIZE 1024
#define MSG_SIZE 488            //メッセージの長さ制限
#define NAMELENGTH 16           //usernameの長さの制限
#define CLIENT_MAX 10           //クライアントの最大接続数
#define TIMEOUT_SEC 1
#define DEFAULT_PORT 50001  /* ポート番号既定値 */
#define NAMELENGTH 16           //usernameの長さの制限

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *argv[]){
    struct sockaddr_in broadcast_adrs;
    struct sockaddr_in from_adrs;
    socklen_t from_len;

    int broadcast_sock;     // ブロードキャスト通信用 socket
    int broadcast_sw=1;     // オプションの設定値　1->bloadcast通信を行う
    in_port_t port_number = DEFAULT_PORT;
    char username[NAMELENGTH];
    
    fd_set mask, readfds;
    struct timeval timeout;

    char conf_packet[BUFSIZE] = "HELO";
    char r_buf[BUFSIZE];    //ブロードキャスト受信用
    int strsize;            //r_buf length
    char mode;              //C -> Client mode，S -> Server mode

    /* username,portが入力されていない場合*/
    if(argc < 2){
        fprintf(stderr,"Usage: %s Server_name User_name is not defined\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* オプション文字列の取得 */
    opterr = 0;
    int c;
    while (1) {
        c = getopt(argc, argv, "n:p:");
        if (c == -1)
            break;
        switch (c) {
            case 'n': /* ユーザ名の指定 */
                snprintf(username, NAMELENGTH, "%s", optarg);
                break;
            case 'p': /* ポート番号の指定 */
                port_number = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optopt);
                return(EXIT_FAILURE);
        }
    }

    // Ctrl + C を無視し，強制終了しない
    sighandler_t sig = 0;
    sig = signal( SIGINT , SIG_IGN );
    if( SIG_ERR == sig ){
        exit(-1);
    }

    /* ブロードキャストアドレスの情報をsockaddr_in構造体に格納する */
    set_sockaddr_in_broadcast(&broadcast_adrs,port_number);

    /* ソケットをDGRAMモードで作成する */
    broadcast_sock = init_udpclient();

    /* ソケットをブロードキャスト可能にする */
    if(setsockopt(broadcast_sock, SOL_SOCKET, SO_BROADCAST,
     (void *)&broadcast_sw, sizeof(broadcast_sw)) == -1){
        exit_errmesg("setsockopt()");
    }

    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(broadcast_sock, &mask);
    
    /* 文字列"HELO"をサーバに送信する (1回目の送信)*/
    Sendto(broadcast_sock, conf_packet, 4, 0, 
        (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));
    
    /* サーバから文字列を受信して表示 */
    int response_count = 1;
    int timeout_check = 1;
    printf("Send Packet:%d\n",response_count);

    for(;;){
        /* 受信データの有無をチェック */
        readfds = mask;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        timeout_check = select( broadcast_sock+1, &readfds, NULL, NULL, &timeout );

        /* タイムアウトの処理 */
        if( timeout_check == 0 ){
            response_count++;
            if(response_count > 3){
                printf("Switch Server mode.\n");
                mode = 'S';
                break;
            }
            /* 文字列"HELO"をサーバに送信する (2,3回目の送信)*/
            Sendto(broadcast_sock, conf_packet, BUFSIZE, 0,  
                (struct sockaddr *)&broadcast_adrs, sizeof(broadcast_adrs));
            printf("Send Packet:%d\n",response_count);
        }

        /* 受信の確認 */
        if(FD_ISSET(broadcast_sock,&readfds)){
            from_len = sizeof(from_adrs);
            strsize = Recvfrom(broadcast_sock, r_buf, BUFSIZE - 1, 0, 
                   (struct sockaddr *)&from_adrs, &from_len);
            r_buf[strsize] = '\0';
            /* サーバのアドレス,r_bufを表示 */
            printf("[%s] %s\n",inet_ntoa(from_adrs.sin_addr), r_buf);
            if(memcmp(r_buf,"HERE", 4) == 0){
                printf("Switch Client mode.\n");
                mode = 'C';
                break;
            }
        }
    }

    switch(mode){
        case 'S':
            ido_server(port_number,username);
            break;
        case 'C':
            ido_client(username, "127.0.0.1", port_number);
            break;
    }

    close(broadcast_sock);
    exit(EXIT_SUCCESS);
}