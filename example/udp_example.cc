/*
 * @Author: jiejie
 * @GitHub: https://github.com/jiejieTop
 * @Date: 2020-10-27 18:34:01
 * @LastEditors: jiejie
 * @LastEditTime: 2020-11-15 11:46:20
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include "dora_socket.h"
#include "dora_log.h"

#define DGRAM_DATA "this is a udp test ..."

int main(void)
{
    
    char recv_buf[1024];
    int len = 0;
    memset(recv_buf, 0, sizeof(recv_buf));

    DORA_LOG_INFO("========================= udp =========================");

    auto udp1 = new doralib::sock("127.0.0.4", "8001");
    auto udp2 = new doralib::sock("127.0.0.4", "8002");
    udp1->sock_bind("8002");
    udp2->sock_bind("8001");

    udp1->sock_write(DGRAM_DATA, strlen(DGRAM_DATA), "127.0.0.4", "8002");

    len = udp1->sock_read(recv_buf, sizeof(recv_buf), NULL, NULL, 1000);

    DORA_LOG_INFO("len is {} : {}", len, recv_buf);

    udp1->sock_close();
    udp2->sock_close();

    DORA_LOG_INFO("========================= success =========================");

    return 0;
}

