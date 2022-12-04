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
#include "pico/stdlib.h"
#include "time.h"

//File with custom config data
#include "config.h"

//#define POWERSAVE_SLEEP
#define WAIT_WIFI_FAILURE_S 3
#define SLEEP_TIME_S 300

int current_id = 0;
char values [NUM_SENSORS][32];
char update_times[NUM_SENSORS][150];
bool processing = false;
json_t mem[32];
char * string_copy;

void callback(const char * string);
void callback_error(const char * error);
void callback_closed(const char * error);

#ifdef POWERSAVE_SLEEP
#include <stdlib.h>
#include "hardware/clocks.h"
#include "hardware/structs/scb.h"
#include "hardware/watchdog.h"
#include "pico/sleep.h"
#include "hardware/rtc.h"
#include "hardware/rosc.h"
static bool awake;
void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();
    irq_init_priorities();
    return;
}

static void sleep_callback(void) {
    printf("RTC woke us up\n");
    uart_default_tx_wait_blocking();
    awake = true;
}

static void rtc_sleep(void) {
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };
    datetime_t t_alarm = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 30
    };
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);
    uart_default_tx_wait_blocking();
    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
}
#endif

int main(void)

{
    stdio_init_all();
    #ifdef POWERSAVE_SLEEP
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    } else {
        printf("Clean boot\n");
    }
    #endif
    int reqnum = 0;
    uint16_t counter = 0;
    processing = false;
    current_id = 0;
    char path[256];
    char counter_str[7];
    bool wifi_init = false;
  
    init_display();
    prepareBuffer();
    my_http_client_init(SERVER_IP, SERVER_PORT, &callback, &callback_error, &callback_closed);

    while (1)
    {
        // Initialise Wifi if needed
        if (wifi_init == false)
        {
            printf("Reinitialise Wifi");
            if (cyw43_arch_init_with_country(CYW43_COUNTRY_GERMANY))
            {
                printf("WiFi init failed");
                sleep_ms(WAIT_WIFI_FAILURE_S * 1000);
                continue;
            }
            cyw43_arch_enable_sta_mode();
            if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000))
            {
                printf("failed to connect\n");
                cyw43_arch_deinit();
                sleep_ms(WAIT_WIFI_FAILURE_S * 1000);
                continue;
            }
            wifi_init = true;
        }
        connect();
        for (int i = 0; i < NUM_SENSORS; i++)
        {
            current_id = i;
            sprintf(&path[0], "/api/states/%s", requests[current_id]);
            processing = true;
            request_website(path, token);
            printf("Request Number %i\t", reqnum);
            reqnum++;
            while (processing)
            {
                sleep_ms(100);
                cyw43_arch_poll();
            }
        }
        close_connection();
        // TODO Implement deactivation of the wifi during the sleep times
        cyw43_arch_poll();
        puts("WiFi deinit");
        cyw43_arch_deinit();
        wifi_init = false;

        counter++;
        sprintf(&counter_str[0], "%06d", counter);
        // Now we paint the data that we downloaded
        clear_buffer();
        add_counter(counter_str);
        for (int i = 0; i < NUM_SENSORS; i++)
        {
            add_text(names[i], i, 1, false);
            add_value(values[i], i, 0);
            add_footer(update_times[i], i, 1);
        }
        add_grid();
        display_buffer();
#ifdef POWERSAVE_SLEEP
        uint scb_orig = scb_hw->scr;
        uint clock0_orig = clocks_hw->sleep_en0;
        uint clock1_orig = clocks_hw->sleep_en1;
        printf("Switching to XOSC\n");
        uart_default_tx_wait_blocking();
        sleep_run_from_xosc();
        printf("Switched to XOSC\n");
        awake = false;
        rtc_sleep();
        // Make sure we don't wake
        while (!awake)
        {
            printf("Should be sleeping\n");
        }

        recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
        puts("Enabling Watchdot to force reboot");
        //Forcing reboot via watchdog
        // watchdog_enable(100, 1);
        // while(1);
#else
        sleep_ms(SLEEP_TIME_S*1000);
        printf("Sleep finished\n");
#endif
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