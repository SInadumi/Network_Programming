#include "mynet.h"

#define BUFSIZE 100

int main(){
    char buf[BUFSIZE];
    char buf_file[BUFSIZE];
    char buf_exit[BUFSIZE];
    int strsize,strsize_file;
    char port_tmp[10];    //標準入力用変数
    int sock_listen,sock_accepted;

    printf("PORTの設定");
    fgets(port_tmp,9,stdin);
    int port = atoi(port_tmp);
    printf("port番号:%d\n",port);

    //サーバーの設定
    sock_listen=init_tcpserver(port,5);

while(1){
    //クライアントからの接続を受け付ける
    sock_accepted = accept(sock_listen, NULL, NULL);
    while(1){
        if (send(sock_accepted, ">", 2, 0) == -1) exit_errmesg("send()");
        //改行コードを受信するまで繰り返す
        do{
            if((strsize = recv(sock_accepted, buf, BUFSIZE, 0)) == -1) exit_errmesg("recv()");
        }while(buf[strsize -1] != '\n');
        
        fflush(stdout);
        strcpy(buf_file,"");

        if(strstr(buf,"list") != NULL){
            FILE *fp;
            char buf_tmp[BUFSIZE];
            
            //ディレクトリ参照
            if((fp=popen("ls ~/work","r")) == -1){
                fprintf(stderr,"fileopen()");
            }
            while (fgets(buf_tmp, strlen(buf_tmp), fp) != NULL)
            { 
                strcat(buf_file, buf_tmp);
            }
            pclose(fp);
        }
        else if(strstr(buf,"type")){
            FILE *fp;
            char *ptr;
            char buf_tmp[BUFSIZE];
            char buff[BUFSIZE] = "cat ~/work/";
            printf("%s",buf);
            strtok(buf," ");    //一回目の分割  
            //二回目の分割
            if((ptr = strtok(NULL,"\r")) ==NULL){  
                fprintf(stderr,"separation");
            }                 
            strcat(buff,ptr);
            if((fp = popen(buff,"r")) == -1){
                fprintf(stderr,"fileopen()");
            }

            while(fgets(buf_tmp,BUFSIZE,fp) != NULL){
                strcat(buf_file,buf_tmp);
            }
            pclose(fp);
        }
        else if(strstr(buf,"exit") != NULL) break;

        //文字列をクライアントに送信
        strsize_file = strlen(buf_file);
        if((send(sock_accepted,buf_file, strsize_file, 0)) == -1) exit_errmesg("send()");
    }
        close(sock_accepted);

        //サーバを閉じる
        printf(">>");
        fgets(buf_exit,BUFSIZE,stdin);
        if(strstr(buf_exit,"bye") != NULL) break;
}
    close(sock_listen);
    exit(EXIT_SUCCESS);
}