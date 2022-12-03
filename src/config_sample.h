//Change the values in the file and rename to config.h
#define NUM_SENSORS 4
#define SERVER_IP "0.0.0.0"
#define SERVER_PORT 8123

char ssid[] = "WIFI_SSID";
char pass[] = "WIFI_PASSWORD";

char * requests[NUM_SENSORS] = {"sensor.id_1", "sensor.id_2", "sensor.id_3","sensor.id_4"};
char * names[NUM_SENSORS] = {"Sensor 1","Sensor 2","Sensor 3","Sensor 4"};