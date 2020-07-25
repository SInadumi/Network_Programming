#include "mynet.h"
#include "ido.h"

#define BUFSIZE 1024
#define MSG_SIZE 488            //メッセージの長さ制限

void ido_client(char *username, char *servername,in_port_t port_number){
    int strsize;
    int tcp_sock;
    char buf[BUFSIZE];
    char mesg[MSG_SIZE];
    fd_set mask, readfds;

    /* サーバに接続する */
    tcp_sock = init_tcpclient(servername, port_number);
    
    /* ビットマスクの準備 */
    FD_ZERO(&mask);
    FD_SET(0,&mask);
    FD_SET(tcp_sock, &mask);

    /* "JOIN username"を送信 */
    sprintf(buf,"%s %s\n","JOIN",username);
    strsize=strlen(buf);
    Send(tcp_sock, buf, strsize, 0);

    for(;;){
        /* 受信データの有無をチェック */
        readfds = mask;
        select(tcp_sock+1, &readfds, NULL, NULL,NULL);
        memset(buf,'\0',BUFSIZE);
        
        /* キーボード入力を監視 */
        if( FD_ISSET(0, &readfds)){
            fgets(buf, BUFSIZE, stdin);
            if(memcmp(buf,"QUIT",4) == 0){
                strncpy(mesg, buf ,BUFSIZE);
            }
            else{
                sprintf(mesg, "%s::::%s::::\n", username, buf);
            }
            strsize = strlen(mesg);
            /* クライアントのメッセージをサーバに送信 */
            Send(tcp_sock, mesg, strsize, 0);
        }
        /* サーバからのを監視 */
        if(FD_ISSET(tcp_sock, &readfds)){
            strsize = Recv(tcp_sock, buf, BUFSIZE-1, 0);
            buf[strsize] = '\0';
            printf("%s",buf);
            /* エラー・終了処理 */
            if(memcmp(buf,"QUIT",4) == 0 || strsize == 0){
                printf("Server is downed!\n");    
                break;
            }
            else if(memcmp(buf,"Error",5) == 0){
                printf("Server denied your message!\n");
                break;
            }else if(memcmp(buf,"Bye",3) == 0){
                break;
            }
            fflush(stdout);
        }
    }
    close(tcp_sock);
}
