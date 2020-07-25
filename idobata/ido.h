#ifndef IDO_H_
#define IDO_H_

#include "mynet.h"

/* mainから呼び出すサーバ用関数 */
void ido_server(in_port_t port_number,char *username);

/* mainから呼び出すクライアント用関数 */
void ido_client(char *username, char *servername, in_port_t port_number);

#endif 