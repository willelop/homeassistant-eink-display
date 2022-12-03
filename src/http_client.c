/*
    Homeassistant-Eink-Display: Uses a Raspberry Pi Pico W (rp2040) and 
    a Waveshare Pico-ePaper-2.66 to display Home-Assisant sensors 
    (e.g.: temperature)
    Copyright (C) 2022  Guillermo López

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "http_client.h"

bool waiting_async = false;
ip_addr_t target_ip;
struct tcp_pcb * my_pcb;
bool error;
const char *domain_internal;
const char *token_internal; 
const char *path_internal; 
int port_internal;
void (*callback_fun)(const char *);
void (*callback_error_fun)(const char *);
void (*callback_closed_fun)(const char *);

void my_http_client_init(const char *domain, int port,void (*callback)(const char *),void (*callback_err)(const char *),void (*callback_closed)(const char *))
{
    domain_internal = domain;
    port_internal = port;
    callback_fun = callback;
    callback_error_fun = callback_err;
    callback_closed_fun = callback_closed;
}

void request_website(const char *path, const char *token)
{
    cyw43_arch_lwip_begin();
    if (my_pcb == NULL)
    {
        puts("TestPCB NULL, connecting");
        connect();
    }
    if (my_pcb->state == CLOSED)
    {
        puts("TestPCB closed, connecting");
        connect();
    }
    
    token_internal = token;
    path_internal = path;
    tcp_send_packet();
    cyw43_arch_lwip_end();
}

void connect()
{
    cyw43_arch_lwip_begin();
    ip_addr_t resolved;
    err_t response = dns_gethostbyname(domain_internal,&resolved,get_dns_reponse, NULL);
    waiting_async = true;
    if(response == ERR_INPROGRESS)
    {
    while (waiting_async)
        {
            cyw43_arch_poll();
            sleep_ms(50);
        }
    }
    else{
        target_ip = resolved;
    }

    /* now connect */
    my_pcb = tcp_new(); 
    /* register callbacks with the pcb */
    tcp_err(my_pcb, tcpErrorHandler);
    tcp_recv(my_pcb, tcpRecvCallback);
    tcp_sent(my_pcb, tcpSendCallback);
    err_t error = tcp_connect(my_pcb, &target_ip, port_internal, connectCallback);
    printf("Error %i\n", error);
    while(my_pcb->state == CLOSED)
    {
        sleep_ms(50);
        cyw43_arch_poll();
    }
    puts("Finishing connect()");
    cyw43_arch_lwip_end();
}
void get_dns_reponse(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    target_ip = *ipaddr;
    waiting_async = false;
}


err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    printf("Connection Established: %i.\n", my_pcb->state);
    return ERR_OK;
}
void tcpErrorHandler(void *arg, err_t err)
{
    printf("Error Established.: %i\n", err);
    callback_error_fun("Error");
}

err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    //printf("Data recieved.\n");
    cyw43_arch_lwip_begin();
    if (p == NULL) {
        close_connection();
        tcp_recved(my_pcb,sizeof(p->payload) / sizeof(char*));
        pbuf_free(p);
        return ERR_CLSD;
    } else {
        callback_fun(p->payload);
        tcp_recved(my_pcb,sizeof(p->payload) / sizeof(char*));
        pbuf_free(p);
    }
    cyw43_arch_lwip_end();
    return 0;
}

err_t  tcpSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    //printf("Sent Confirmed!.\n");
    return ERR_OK;
}

uint32_t tcp_send_packet(void)
{
    cyw43_arch_lwip_begin();
    static char string[350];
    sprintf(string,"GET %s HTTP/1.1\r\nHost: %s\r\nAuthorization: Bearer %s\r\nConnection: keep-alive\r\n\r\n", path_internal, domain_internal, token_internal);
    uint32_t len = strlen(string);

    error = tcp_write(my_pcb, string, strlen(string), TCP_WRITE_FLAG_COPY);

    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_write)\n", error);
        return 1;
    }

    error = tcp_output(my_pcb);
    if (error) {
        printf("ERROR: Code: %d (tcp_send_packet :: tcp_output)\n", error);
        return 1;
    }
    cyw43_arch_lwip_end();
    return 0;
}

void close_connection(void)
{
    cyw43_arch_lwip_begin();
    puts("Closing Connection\n");
    err_t error = tcp_close(my_pcb);
    printf("Closing Error: %i\n", error);
    if (error != ERR_OK) {
      tcp_abort(my_pcb);
    }
    callback_closed_fun("Closed");
    cyw43_arch_lwip_end();
}