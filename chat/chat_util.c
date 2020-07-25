// gcc -c chat_util.c
#include "mynet.h"
#include "chat.h"
#include <stdlib.h>
#include <unistd.h>

#define NAMELENGTH 20 /* ログイン名の長さの制限 */
#define CHATLENGTH 100
#define BUFLEN 500    /* 通信バッファサイズ */

typedef struct{
    int sock;
    char name[NAMELENGTH];
}client_info;

/* プライベート関数 */
static int client_login(int sock_listen);
static char *receive_chat();
static void send_chat(char* comment);
static char *chop_nl(char *s);

/* プライベート変数 */
static int N_client;        /* クライアントの数*/
static client_info *Client; /* クライアントの情報 */
static int Max_sd;          /* ディスクリプタ最大値 */
static char Buf[BUFLEN];    /* 通信用バッファ */
static char username[NAMELENGTH]; /* 発言したクライアント */

/* mainから呼び出す関数 */
void chat_server(int port_number, int n_client){
    int sock_listen;
    // サーバーの初期化
    sock_listen = init_tcpserver(port_number, 5);
    // クライアントの接続
    init_client(sock_listen, n_client);
    close(sock_listen);
    // メインループ
    chat_loop();
}

/* クライアントの初期化を行う関数 */
void init_client(int sock_listen, int n_client){
    N_client = n_client;

    /* クライアント情報の保存用構造体の初期化 */
    if( (Client = (client_info *)malloc(N_client*sizeof(client_info)))==NULL ){
        exit_errmesg("malloc()");
    }

    /* クライアントのログイン処理 */
    Max_sd = client_login(sock_listen);
}

/* メインループ */
void chat_loop(){
    char *chat;
    for(;;){
      int client_id;
      for(client_id=0;client_id<N_client;client_id++){
        Send(Client[client_id].sock, ">", 2, 0);
      }
      // コメントの受信
        chat = receive_chat();
        // 受信したコメントを送信
        send_chat(chat);
         
    }
}

/* クライアントのログイン処理を行う関数 */
static int client_login(int sock_listen){
  int client_id,sock_accepted;
  static char prompt[]="Input your name: ";
  char loginname[NAMELENGTH];
  int strsize;

  for(client_id=0; client_id < N_client; client_id++){
    /* クライアントの接続を受け付ける */
    sock_accepted = Accept(sock_listen, NULL, NULL);
    printf("Client[%d] connected. \n", client_id);

    /* ログインプロンプトの送信 */
    Send(sock_accepted, prompt, strlen(prompt), 0);

    /* ログイン名の受信 */
    strsize = Recv(sock_accepted, loginname, NAMELENGTH-1, 0);
    loginname[strsize] = '\0';
    chop_nl(loginname);

    /* ユーザ情報を保存 */
    Client[client_id].sock = sock_accepted;
    strncpy(Client[client_id].name, loginname, NAMELENGTH);
  }
  /* 最大のソケット番号(最後にログインしたクライアント)を返す */
  return(sock_accepted);
}

/* クライアントからコメントを受け取る関数 */
static char *receive_chat(){
  fd_set mask, readfds;
  int client_id;
  int is_comment = 0;
  int strsize;

  /* ビットマスクの準備 */
  FD_ZERO(&mask);
  for(client_id = 0; client_id < N_client; client_id++){
    FD_SET(Client[client_id].sock, &mask);  
  }
  // コメントが送られてくるまでループ
  while(1){
    /* 受信データの有無をチェック */
    readfds = mask;
    select(Max_sd + 1, &readfds, NULL, NULL, NULL);

    for(client_id = 0; client_id < N_client; client_id++){
      if(FD_ISSET(Client[client_id].sock, &readfds)){
       
        strsize = Recv(Client[client_id].sock, Buf, BUFLEN-1, 0);
        Buf[strsize] = '\0';
        /* 発言したユーザを記憶する */
        strncpy(username,Client[client_id].name,NAMELENGTH);
        is_comment = 1;
        break;       
      }
    }
    if(is_comment == 1) break;
  }    
  return(Buf);
}

/* クライアントに受け取ったコメントを送る関数 */
static void send_chat(char *comment){
  int client_id;
  static char User_temp[NAMELENGTH+12];
  static char User_mesg[CHATLENGTH+12];
  sprintf(User_temp,"%s %s \n", " User: ",username);
  sprintf(User_mesg,"%s %s \n", " Message: ",comment);
  
  for(client_id = 0; client_id < N_client; client_id++){
    /* 各クライアントにUser名とcommentを送信 */
    Send(Client[client_id].sock, User_temp, strlen(User_temp), 0);
    Send(Client[client_id].sock, User_mesg, strlen(User_mesg), 0);
  }
  // 変数の初期化
  fflush(stdout);
  memset(username,'\0',sizeof(username));
  memset(User_temp,'\0',sizeof(User_temp));
  memset(comment,'\0',CHATLENGTH);
  memset(User_mesg,'\0',sizeof(User_mesg));
}

/* 文字列の最後に改行コードがあれば，取り除く関数 */
static char *chop_nl(char *s)
{
  int len;
  len = strlen(s);
  if( s[len-1] == '\n' ){
    s[len-1] = '\0';
  }
  return(s);
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