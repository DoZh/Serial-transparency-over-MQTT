/* main.c -- MQTT client example
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"

#include "ip_addr.h"
#include "smartconfig.h"
#include "airkiss.h"

char mqtt_send_channel[20],mqtt_recv_channel[20],mqtt_ctrl_channel[20];
char mqtt_extsend_channel[20],mqtt_extrecv_channel[20];

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}

bool ICACHE_FLASH_ATTR isExpChannel(uint8_t *mqtt_channel_name,uint8_t *mqtt_type)
{
  uint8_t i = 0;
  for(i = 0; i <= 4; i++)
  {
    if(mqtt_channel_name[i+1] != mqtt_type[i])
      return FALSE;
  }
  return TRUE;
}

MQTT_Client mqttClient;
static void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
  if (status == STATION_GOT_IP) {
    MQTT_Connect(&mqttClient);
  } else {
    MQTT_Disconnect(&mqttClient);
  }
}
static void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Connected\r\n");
  MQTT_Subscribe(client, mqtt_recv_channel, MQTT_QOS);
  MQTT_Subscribe(client, mqtt_ctrl_channel, MQTT_QOS);
  MQTT_Publish(client, mqtt_send_channel, "hello", 6, MQTT_QOS, 0);
}

static void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Disconnected\r\n");
}

static void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
  MQTT_Client* client = (MQTT_Client*)args;
  INFO("MQTT: Published\r\n");
}

static void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
  char *topicBuf = (char*)os_zalloc(topic_len + 1),
        *dataBuf = (char*)os_zalloc(data_len + 1);

  MQTT_Client* client = (MQTT_Client*)args;
  os_memcpy(topicBuf, topic, topic_len);
  topicBuf[topic_len] = 0;
  os_memcpy(dataBuf, data, data_len);
  dataBuf[data_len] = 0;
  INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

  if(isExpChannel(topic,"recv"))
  {
    uint16_t  remainDataLen = data_len;
    uint8_t txNow = 0;
    do
    {
      txNow = TX_BUFF_SIZE - TX_FIFO_LEN(UART1);
      if(txNow > remainDataLen)
      {
        uart1_tx_buffer(dataBuf + (data_len - remainDataLen), remainDataLen);
        remainDataLen = 0;
      }
      else
        {
          uart1_tx_buffer(dataBuf + (data_len - remainDataLen), txNow);
          remainDataLen = remainDataLen - txNow;
        }
    }
    while(remainDataLen);
  }

  os_free(topicBuf);
  os_free(dataBuf);
}

void ICACHE_FLASH_ATTR print_info()
{
  INFO("\r\n\r\n[INFO] BOOTUP...\r\n");
  INFO("[INFO] SDK: %s\r\n", system_get_sdk_version());
  INFO("[INFO] Chip ID: %08X\r\n", system_get_chip_id());
  INFO("[INFO] Memory info:\r\n");
  system_print_meminfo();

  INFO("[INFO] -------------------------------------------\n");
  INFO("[INFO] Build time: %s\n", BUID_TIME);
  INFO("[INFO] -------------------------------------------\n");

}

void ICACHE_FLASH_ATTR conf_mqtt_channel_name()
{
  uint8_t sta_mac[6];
  wifi_get_macaddr(STATION_IF,sta_mac);
  //os_sprintf(mqtt_send_channel, "/%s", ssid);
  os_sprintf(mqtt_send_channel, "/send/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_recv_channel, "/recv/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_ctrl_channel, "/ctrl/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));

}

static void ICACHE_FLASH_ATTR app_init(void)
{

  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  print_info();
  conf_mqtt_channel_name();
  MQTT_InitConnection(&mqttClient, MQTT_HOST, MQTT_PORT, DEFAULT_SECURITY);
  //MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

  if ( !MQTT_InitClient(&mqttClient, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, MQTT_KEEPALIVE, MQTT_CLEAN_SESSION) )
  {
    INFO("Failed to initialize properly. Check MQTT version.\r\n");
    return;
  }
  //MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);
  MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
  MQTT_OnConnected(&mqttClient, mqttConnectedCb);
  MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
  MQTT_OnPublished(&mqttClient, mqttPublishedCb);
  MQTT_OnData(&mqttClient, mqttDataCb);
	WIFI_Connect(wifiConnectCb);
	INFO("*_*Comp app_init\n");
	//smartconfig_start(smartconfig_done);
}
void user_init(void)
{

  os_printf("SDK version:%s\n", system_get_sdk_version());
  smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
  //wifi_set_opmode(STATION_MODE);
	//smartconfig_start(smartconfig_done);

  system_init_done_cb(app_init);

}
