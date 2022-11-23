#include <esp8266.h>
#include <config.h>
#include <sys/time.h>
#include "mqtt_client.h"
#include "mqtt_cmd.h"

#define REPEAT_INTERVAL 2000
#define REPEAT_OFF_COUNT 5

char* topic = "dusty";

unsigned long lastMessage = 0;
unsigned long closeCount = 0;

static ETSTimer mytimer;

inline unsigned long millis() {
  struct timeval te;
  gettimeofday(&te, NULL);  // get current time
  long long milliseconds =
      te.tv_sec * 1000LL + te.tv_usec / 1000;  // calculate milliseconds
  return milliseconds;
}

void sendMessage(bool on) {
  char data[40] = "{\"ID\":\"0\", \"EVENT\":";
  if (on) {
    strcat(data, "\"TOOL_ON\"}");
  } else {
    strcat(data, "\"TOOL_OFF\"}");
  }
  MQTT_Publish(&mqttClient, topic, data, strlen(data), 1, 0);
}

static void ICACHE_FLASH_ATTR sense() {
    bool opened = false;
    if (GPIO_INPUT_GET(5) == 0) {
        opened = true;
    }
    if (opened) {
        if (lastMessage + REPEAT_INTERVAL <= millis()) {
            sendMessage(true);
            lastMessage = millis();
        }
        closeCount = REPEAT_OFF_COUNT;
    } else {
        if (closeCount > 0) {
            sendMessage(false);
            closeCount--;
        }
    }    
} 

// initialize the custom stuff that goes beyond esp-link
void app_init() {
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(5)); 
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
    ETS_GPIO_INTR_DISABLE();
    
    os_timer_disarm(&mytimer);
    os_timer_setfn(&mytimer, sense, NULL);
    os_timer_arm(&mytimer, 500, 1);
}


