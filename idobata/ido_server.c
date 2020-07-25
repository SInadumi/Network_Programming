#include "mynet.h"
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <pthread.h>

#define BUFSIZE 1024
#define MSG_SIZE 488            //メッセージの長さ制限
#define NAMELENGTH 16           //usernameの長さの制限
#define CLIENT_MAX 10           //クライアントの最大接続数
#define TIMEOUT_SEC 1           //タイムアウト時間
#define DEFAULT_PORT 50001      // ポート番号既定値 
#define NAMELENGTH 16           //usernameの長さ

char r_buf[BUFSIZE];
char s_buf[BUFSIZE];

int tcp_sock_listen;            //TCP socket
int udp_sock_listen;            //UDP socket

/* プライベート変数 */
typedef struct{
    int sock;                   //accept socketを格納
    char name[NAMELENGTH];      //Usernameを格納
    int is_exist;               //sockが存在している -> 1
}client_info;

client_info *Client;    /* クライアント情報 */
int N_client = 0;       /* クライアントの接続数 */
int Max_sd = 0;         /* ディスクリプタ最大値 */
char server_name[NAMELENGTH];

/* server main loop */
static void ido_server_main_loop();
/* UDPポートを監視する関数 */
static void check_udp_port();
/* TCPポートを監視する関数 */
static void check_tcp_port();
/* クライアントスレッドの生成 */
static void *client_thread(void *arg);
/* クライアント情報の初期化 */
static void init_client();
/* クライアントの抹消を行う関数 */
static void del_client(int accept_sock);
/* クライアントのログイン処理を行う関数 */
static int registration_client(int tcp_sock_accepted, char *username);
/* サーバのメッセージを全クライアントに送信する関数 */
static void server_send_mesg(char *username, char *mesg);
/* "\n"を取り出す関数 */
static char *chop_nl(char *s);
/* 文字列の切り出しを行う関数 */
static char *chop_string(char *s, char *chop,int num);

/* mainから呼び出す関数 */
void ido_server(in_port_t port_number, char *username){
    
    /* UDPサーバの初期化 */
    udp_sock_listen = init_udpserver(port_number);
    /* TCPサーバの初期化 */
    tcp_sock_listen = init_tcpserver(port_number, 5);

    init_client();
    strncpy(server_name,username,NAMELENGTH);
    
    //メインループ
    ido_server_main_loop();

    close(udp_sock_listen);
    close(tcp_sock_listen);
}

/* サーバーメインループ */
static void ido_server_main_loop(){
    fd_set mask, readfds;
    int strsize;
    int client_id;
    FD_ZERO(&mask);
    /* (server)キーボードsocketの準備 */
    FD_SET(0,&mask);
    /* udp socketの準備 */
    FD_SET(udp_sock_listen,&mask);
    /* tcp socketの準備 */
    FD_SET(tcp_sock_listen,&mask);

    // 最大値を更新
    if(udp_sock_listen > Max_sd) Max_sd = udp_sock_listen;
    if(tcp_sock_listen > Max_sd) Max_sd = tcp_sock_listen;
    
    for(;;){
        readfds = mask;
        select( Max_sd+1, &readfds, NULL, NULL, NULL );

        /* UDPポートの監視 */
        if(FD_ISSET(udp_sock_listen, &readfds)){
            check_udp_port();
        }
        /* TCPポートの監視 */
        if(FD_ISSET(tcp_sock_listen, &readfds)){
            check_tcp_port();
        }
        /* 標準入力の監視 */
        if(FD_ISSET(0,&readfds)){
            fgets(s_buf, BUFSIZE, stdin);
            /* server のメッセージを全クライアントに送信 */
            server_send_mesg(server_name, s_buf);

            if(memcmp(s_buf,"QUIT",4) == 0) break;    //serverのログアウト
        }
        /* 初期化 */
        memset(s_buf,'\0',strlen(s_buf));
        memset(r_buf,'\0',strlen(r_buf));
    }
}

/* UDPポートを監視する関数 */
static void check_udp_port(){
    /* 文字列をクライアントから受信する */
    struct sockaddr_in from_adrs;
    int from_len = sizeof(from_adrs);
    int strsize;
    char response_packet[BUFSIZE] = "HERE";

    /* メッセージの受信 */
    strsize = Recvfrom(udp_sock_listen, r_buf, BUFSIZE-1, 0, 
                (struct sockaddr *)&from_adrs, &from_len);
    r_buf[strsize] = '\0';

    /* シェイクメッセージの送信 */
    if(memcmp(r_buf,"HELO",4) == 0){
        strsize = strlen(response_packet);
        Sendto(udp_sock_listen, response_packet, strsize, 0,
         (struct sockaddr *)&from_adrs, sizeof(from_adrs) );
    }
}

/* TCPポートを監視する関数 */
static void check_tcp_port(){
    int *tharg;
    pthread_t tid;

    /* 子プロセスの生成 */
    /* スレッド関数の引数を用意する */
    if ((tharg = (int *)malloc(sizeof(int))) == NULL){
        exit_errmesg("malloc()");
    }
    *tharg = tcp_sock_listen;
    /* スレッドの生成 */
    if (pthread_create(&tid, NULL, client_thread, (void *)tharg) != 0){
        exit_errmesg("pthread_create()");
    }
}

/* クライアントの初期化 */
static void init_client(){
    int client_id;

    /* クライアント情報の保存用構造体の初期化 */
    if((Client = (client_info *)malloc(CLIENT_MAX*sizeof(client_info))) == NULL){
        exit_errmesg("malloc()");
    }
    /* すべてのClientに対して，is_exist = 0とする */
    for(client_id = 0; client_id < CLIENT_MAX;client_id++){
        Client[client_id].is_exist = 0;
    }
}

/* クライアントの抹消を行う関数 */
static void del_client(int accept_sock){
    int client_id;
    int del_client;

    /* 終了メッセージの送信 */
    Send(accept_sock, "Bye", 4, 0);

    /* 抹消する要素を特定 */
    for(client_id = 0; client_id < CLIENT_MAX; client_id++){
        if(Client[client_id].sock == accept_sock){
            del_client = client_id;
            break;
        }
    }

    /* クライアントの抹消 */
    printf("Client[%d] is downed. ID is [%d] Name is [%s]\n",del_client, Client[del_client].sock,Client[del_client].name);
    Client[del_client].sock = 0;
    memset(Client[del_client].name, '\0', strlen(Client[del_client].name));
    Client[del_client].is_exist = 0;
    N_client--;
}

/* クライアントスレッド */
static void *client_thread(void *arg){
    int strsize;
    int sock_accepted;  
    char *username;

    free(arg); /* 引数用のメモリを開放 */
    
    /* クライアントの接続を受け付ける */
    sock_accepted = Accept(tcp_sock_listen, NULL, NULL);
    pthread_detach(pthread_self()); /* スレッドの分離(終了を待たない) */

    strsize = Recv(sock_accepted, r_buf, BUFSIZE, 0); 
    r_buf[strsize] = '\0';
    
    /* クライアントの登録処理 */

    /* クライアントが最大数接続されている場合 */
    if(N_client >= CLIENT_MAX){
        // エラーメッセージを送信
        Send(sock_accepted, "Error: TCP server can't have CLIENT.\n", 39, 0);
        close(sock_accepted);
        return(NULL);
    }

    if(memcmp(r_buf, "JOIN", 4) == 0){
        username = chop_string(r_buf, " ", 2);
    }else{
        /* エラーメッセージを送信 */
        Send(sock_accepted, "Error: Login\n", 15, 0);
        close(sock_accepted);
        return(NULL);
    }

    if(registration_client(sock_accepted, chop_nl(username)) == -1){
        close(sock_accepted);
        return(NULL);
    }

    /* ログイン済クライアントに対する処理 */
    /* socketの準備 */
    fd_set mask, readfds;
    char *Send_Client;      /* 発言者 */
    char *mesg;             /* メッセージ */

    FD_ZERO(&mask);
    FD_SET(sock_accepted, &mask);

    while(1){
        readfds = mask;
        select( Max_sd+1, &readfds, NULL, NULL, NULL );
        if(FD_ISSET(sock_accepted,&readfds)){
            /* メッセージをクライアントから受信する */
            strsize = Recv(sock_accepted, r_buf, BUFSIZE+NAMELENGTH, 0);

            /* 終了受付 */
            if(memcmp(r_buf, "QUIT", 4) == 0){
                /* プロセス回収 */
                del_client(sock_accepted);
                break;
            }

            /* ユーザ名とメッセージを取り出す */
            mesg = chop_string(r_buf, "::::", 2); 
            Send_Client = chop_string(r_buf, "::::", 1);

            /*　メッセージエラー処理　*/
            if(strsize > MSG_SIZE){
                Send(sock_accepted, "Message Error: Message is too long\n", 37, 0);
            }
            /* POSTされたメッセージを全クライアントに送信 */
            else{
                server_send_mesg(Send_Client, mesg);
            }
        }
        /* メッセージ初期化 */
        memset(mesg,'\0',strlen(mesg));
        memset(Send_Client,'\0',strlen(Send_Client));
        memset(r_buf,'\0',strlen(r_buf));
    }

    close(sock_accepted); /* ソケットを閉じる */
    return (NULL);
}

/* サーバのメッセージを全クライアントに送信する関数 */
static void server_send_mesg(char *username, char *send_mesg){
    int client_id;
    static char User_temp[NAMELENGTH+12];
    static char MSG_temp[MSG_SIZE+12];

    /* メッセージ加工 */
    sprintf(User_temp,"\n[%s] %s", "username", username);
    /* サーバからの"QUIT"メッセージである場合 */
    if(memcmp(send_mesg, "QUIT", 4) == 0 && memcmp(username, server_name, 6) == 0){
        sprintf(MSG_temp, "%s", send_mesg);
    }
    /* MESG発言メッセージである場合 */
    else {
        sprintf(MSG_temp, "\n[%s] %s", "message", send_mesg);
        /* クライアントのメッセージを表示 */
        printf("%s",User_temp);
        printf("%s",MSG_temp);
        fflush(stdout);
    }

    /* 各クライアントにメッセージを送信 */
    for(client_id=0; client_id < CLIENT_MAX; client_id++){

        /* is_exist = 0 のクライアントには送信しない */
        if(Client[client_id].is_exist == 0) continue;

        /* サーバからの"QUIT"メッセージを送る場合，Usernameは送信しない */
        if(memcmp(send_mesg,"QUIT",4) == 0 && memcmp(username,server_name,6) == 0){
            Send(Client[client_id].sock, MSG_temp, strlen(MSG_temp), 0);
        }
        else{
            Send(Client[client_id].sock, User_temp, strlen(User_temp), 0);
            Send(Client[client_id].sock, MSG_temp, strlen(MSG_temp), 0);
        }
    }
}

/* クライアントのログイン処理を行う関数 */
static int registration_client(int tcp_sock_accepted, char *username){
    int client_id;
    int client_is_exist = 0;

    /* メッセージの解析 */
    for(client_id=0; client_id<CLIENT_MAX; client_id++){
        /* 既に存在している場合 */
        if(memcmp(Client[client_id].name, username, strlen(username)) == 0){
            client_is_exist=1;
            break;
        }
    }

    if(memcmp(server_name, username, strlen(username)) == 0) client_is_exist = 1;

    if(client_is_exist == 1){
        /* エラーメッセージを送信 */
        Send(tcp_sock_accepted, "Error: Username is already existed", 35, 0);
        return(-1);
    }

    /* ログインプロンプトの送信 */
    Send(tcp_sock_accepted, "Connected\n", 12, 0);

    /* 最大のソケット番号を更新 */
    if(Max_sd < tcp_sock_accepted) Max_sd = tcp_sock_accepted;
        
    /* ユーザデータを更新 */
    for(client_id = 0; client_id < CLIENT_MAX; client_id++){
        /* is_exist = 0　であるClientを探索 */
        if(Client[client_id].is_exist == 0){
            Client[client_id].sock = tcp_sock_accepted;
            strncpy(Client[client_id].name, username, NAMELENGTH);
            Client[client_id].is_exist = 1;
            break;
        }
    }

    printf("Client[%d] connected. ID is [%d] Name is [%s]\n",client_id,Client[client_id].sock,Client[client_id].name);
    N_client++;
    return(0);
}

/* 文字列の最後に改行コードがあれば，取り除く */
static char *chop_nl(char *s)
{
  int len;
  len = strlen(s);
  if( s[len-1] == '\n' ){
    s[len-1] = '\0';
  }
  return(s);
}

/* 文字列の切り出しを行う関数*/
static char *chop_string(char *s, char *chop,int num){
    /* s:切り出す前の文字列
     * chop:分割したい文字
     * num:所望する文字列，分割数で指定    
     */
    char *temp;      //文字の切り出し時に使用
    
    /* 1回目の切り出し */
    temp = strtok(r_buf, chop);
    int tok_temp=1;
    if(tok_temp == num){
        return (temp);
    }
    /* 2回目以降の切り出し */
    while(temp != NULL){
        if(tok_temp != num){
            tok_temp++;
            continue;
        }
        temp = strtok(NULL, chop);
        break;
    }
    return (temp);
}