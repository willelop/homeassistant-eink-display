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

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "eink_display_266.h"
#include "http_client.h"
#include "tiny-json.h"
#include "lwip/apps/http_client.h"

//File with custom config data
#include "config.h"

#define WAIT_WIFI_FAILURE_S 3
#define SLEEP_TIME_S 120

int current_id = 0;
char values [NUM_SENSORS][32];
char update_times[NUM_SENSORS][150];
bool processing = false;
json_t mem[32];
char * string_copy;

void callback(const char * string);
void callback_error(const char * error);
void callback_closed(const char * error);

int main(void)

{
    int reqnum = 0;
    uint16_t counter = 0;
    processing = false;
    current_id = 0;
    char path[256];
    char counter_str[7];
    bool wifi_init = false;
    stdio_init_all();
    init_display();
    prepareBuffer();
    my_http_client_init(SERVER_IP, SERVER_PORT, &callback, &callback_error, &callback_closed);

    while(1)
    {
        //Initialise Wifi if needed
        if (wifi_init==false)
        {                
            printf("Reinitialise Wifi");
            if (cyw43_arch_init_with_country(CYW43_COUNTRY_GERMANY))
            {
                printf("WiFi init failed");
                sleep_ms(WAIT_WIFI_FAILURE_S*1000);
                continue;
            }
            cyw43_arch_enable_sta_mode();
            if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000))
            {
                printf("failed to connect\n");
                cyw43_arch_deinit();
                sleep_ms(WAIT_WIFI_FAILURE_S*1000);
                continue;
            }
            wifi_init = true;
        }
        connect();
        for(int i =0; i < NUM_SENSORS; i++)
        {
            current_id = i;
            sprintf(&path[0], "/api/states/%s", requests[current_id]);
            processing = true;
            request_website(path,token);
            printf("Request Number %i\t",reqnum);
            reqnum++;
            while(processing)
            {
                sleep_ms(100);
                cyw43_arch_poll();
            }
        }
        close_connection();
        //TODO Implement deactivation of the wifi during the sleep times
        cyw43_arch_poll();
        puts("WiFi deinit");
        cyw43_arch_deinit();
        wifi_init = false;

        counter++;
        sprintf(&counter_str[0],"%06d",counter);
        //Now we paint the data that we downloaded
        clear_buffer();
        add_counter(counter_str);
        for(int i =0; i < NUM_SENSORS; i++)
        {
            add_text(names[i],i, 1,false);
            add_value(values[i],i, 0);
            add_footer(update_times[i],i,1);
        }
        add_grid();
        display_buffer();

        //Sleeping time
        sleep_ms(SLEEP_TIME_S*1000);
        puts("Waking up");
    }

    printf("End of program reached\n");
    return 0;
}

void callback(const char * string)
{
    string_copy = strdup(string);
    json_t const* json = json_create( string_copy, mem, sizeof mem / sizeof *mem );
    free(string_copy);
    if ( !json ) {
        puts("Error json create.");
        return;
    }

    json_t const* stateProp = json_getProperty( json, "state" );
    if ( !stateProp || JSON_TEXT != json_getType( stateProp ) ) {
        puts("Error, the state property is not found.");
        return;
    }
    char const* stateVal = json_getValue( stateProp );

    json_t const* lastProp = json_getProperty( json, "last_updated" );
    if ( !lastProp || JSON_TEXT != json_getType( lastProp ) ) {
        puts("Error, the last_updated property is not found.");
        return;
    }
    char const* lastVal = json_getValue( lastProp );

    printf( "%s: %s.\n", names[current_id] ,stateVal);
    sprintf(values[current_id], "%.4s", stateVal);
    sprintf(update_times[current_id], "%.*s\n", 19,lastVal);
    update_times[current_id][10] = 0x20;
    update_times[current_id][19] = 0x0;
    processing = false;
}

void callback_error(const char * string)
{
    puts("Error received, unblocking");
    processing = false;
}

void callback_closed(const char * string)
{
    puts("Closed received, unblocking");
    processing = false;
}