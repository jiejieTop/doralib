/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-27 18:34:01
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-15 11:56:58
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_log.h"
#include "dora_socket.h"
#include "dora_epoller.h"

#define EPOLLER_DATA "this is a epoller test ..."

using doralib::epoller;

void udp_read(sock *s)
{
    char recv_buf[1024];
    int len = 0;
    memset(recv_buf, 0, sizeof(recv_buf));

    DORA_LOG_INFO("udp read callback ...");
    
    len = s->sock_read(recv_buf, sizeof(recv_buf), NULL, NULL, 0);

    DORA_LOG_INFO("len is {} : {}", len, recv_buf);
}


int main(void)
{   
    DORA_LOG_INFO("========================= epoller =========================");
    
    auto epoller = new doralib::epoller;

    auto udp1 = new doralib::sock("127.0.0.4", "8001");
    auto udp2 = new doralib::sock("127.0.0.4", "8002");
    udp1->sock_bind("8002");
    udp2->sock_bind("8001");

    epoller->epoller_add(udp2, doralib::DORA_EPOLLER_READ, udp_read);

    udp1->sock_write(EPOLLER_DATA, strlen(EPOLLER_DATA), "127.0.0.4", "8001");
    
    sleep(1);
    delete epoller;

    DORA_LOG_INFO("========================= success =========================");

    return 0;
}

