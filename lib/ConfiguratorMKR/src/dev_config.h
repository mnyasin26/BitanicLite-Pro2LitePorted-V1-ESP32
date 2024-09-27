#include <Arduino.h>

#define CONFIGURATOR_HOST "configurator.makerindo.co.id"
#define FIRMWARE_VERSION "1.1.2"
#define HARDWARE_VERSION "BT_LT_V1.0_ESP32"
#define DEFAULT_SYNC_INTERVAL "30"
#define DEBUG_PROGRAM false

// MACRO
#define WIFI_SET_AUTOCONNECT 0
#define WIFI_SET_ONDEMAND 1
#define JSON_SIZE 3800
#define DELAY_LCD_UPDATE 50
#define QUEUE_SIZE_ITEMS 10
#define MINUTE 60

// DEVICE DEFAULT CONFIGURATION
// #define DEVICE_SERIAL_NUMBER "BT-LT-180924-V1.0-0001" // not actually used, this only an example

#define DEVICE_ID "BTXX"
// #define MQTT_ID "BT15"
#define MQTT_SERVER "bitanic.id"
#define MQTT_TOPIC_SUBSCRIBE "bitanic/BTXX"
#define MQTT_TOPIC_PUBLISH "bitanic/lite"
#define WIFI_AP_NAME "BL_SETTING"
#define WIFI_AP_PASSWORD "admin1234"

// #define MQTT_SERVER "bitanic.id"
// #define MQTT_TOPIC "bitanic/BT15"
// #define MQTT_TOPIC_PUBLISH "bitanic/lite"

// #define WIFI_AP_NAME "BL_SETTING"
// #define WIFI_AP_PASSWORD "admin1234"

// PIN CONFIGURATION
#define DHTPIN1 33
#define DHTPIN2 32
#define DHTTYPE DHT22
#define MAIN_PUMP_PIN -1
#define WATER_PUMP_PIN 2
#define NUTRIENT_PUMP_PIN 15
#define LAHAN1_PUMP_PIN -1
#define LAHAN2_PUMP_PIN -1
#define LAHAN3_PUMP_PIN -1
#define LAHAN4_PUMP_PIN -1
#define BUZZER_PIN 27
#define LED_PIN_PENYIRAMAN 12
#define LED_PIN_PEMUPUKAN 13

// DEBUG
#define DEBUG_WIFI_MANAGER false
#define DEBUG_MQTT false

#if DEBUG_PROGRAM == true
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x, y) Serial.printf(x, y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(x, y)
#endif