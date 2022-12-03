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

#include "lwipopts.h"
#include "lwip/dns.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/timeouts.h"
#include "lwip/tcpip.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"

void get_dns_reponse(const char *name, const ip_addr_t *ipaddr, void *arg);

void tcpErrorHandler(void *arg, err_t err);
err_t tcpRecvCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t  tcpSendCallback(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t connectCallback(void *arg, struct tcp_pcb *tpcb, err_t err);
uint32_t tcp_send_packet(void);
void close_connection(void);

void my_http_client_init(const char *domain, int port,void (*callback)(const char *),void (*callback_err)(const char *),void (*callback_closed)(const char *));
void request_website(const char *path,  const char *headers);
void connect();