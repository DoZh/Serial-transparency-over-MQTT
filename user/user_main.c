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

#define MEMLEAK_DEBUG

#define NORMAL_MODE 0
#define GPIO_CTRL_MODE 1
#define EDIT_CONF_MODE 2
#define AUTHING_MODE 3
#define RESET_MODE 4
#define UPDATE_JSON_MODE 5

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
#include "cJSON.h"


char mqtt_host[64];
int mqtt_port;
int mqtt_recv_buf_size;
int mqtt_keepalive;
char mqtt_user[32];
char mqtt_pass[64];
char mqtt_client_id_prefix[16];
int mqtt_qos;
int mqtt_extra_sub_channel_enable;
char mqtt_extra_sub_channel[32];
int rx_buff_size;
int tx_buff_size;
int mqtt_pub_pack_cycle;
int mqtt_reconnect_timeout;
int default_security;
int queue_buffer_size;
int mqtt_clean_session;
char config_passwd[32];

char mqtt_client_id[30];
char mqtt_send_channel[20],mqtt_recv_channel[20];
char mqtt_ctrl_channel[20];mqtt_state_channel[20];
char mqtt_extrecv_channel[20];

char mqtt_online_msg[21];
char mqtt_offline_msg[22];

uint8_t control_state = 0;
uint8_t expect_mode = 0;
uint8_t restart_command_count = 0;
uint8_t wrong_passwd_count = 0;

uint8_t Sub2TxQueue = 0;
static ETSTimer Rx2PubSender;
static ETSTimer Sub2TxSender;
static ETSTimer ControlTimer;
cJSON *jsonRoot = NULL;


QUEUE rxBuff, txBuff;
char tmpBufTx[128];
char tmpBuffPub[1500];


char *testStr="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";

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

bool ICACHE_FLASH_ATTR check_memleak_debug_enable(void)
{
    return MEMLEAK_DEBUG_ENABLE;
}

bool ICACHE_FLASH_ATTR parse_json_to_string(cJSON *item, char *namestr, char *writestr)
{
  cJSON *jsonObj;
  if(jsonObj = cJSON_GetObjectItem(item, namestr))
  {
    //INFO("%s\n", jsonObj->valuestring);
    os_sprintf(writestr, "%s", jsonObj->valuestring);
    return TRUE;
  }
  else
    return FALSE;
}

bool ICACHE_FLASH_ATTR parse_json_to_integer(cJSON *item, char *namestr, int *intvalue)
{
  cJSON *jsonObj;
  if(jsonObj = cJSON_GetObjectItem(item, namestr))
  {
    //INFO("%d\n", jsonObj->valueint);
    *intvalue = jsonObj->valueint;
    return TRUE;
  }
  else
    return FALSE;
}

bool ICACHE_FLASH_ATTR parse_json_from_flash(void)
{
  char configBuff[CONFIG_BUFF_SIZE];
  spi_flash_read(CONFIG_JSON_ADDR,configBuff,CONFIG_BUFF_SIZE);
  INFO("%s\n",configBuff);
  jsonRoot = cJSON_Parse(configBuff);
  if (jsonRoot)
    return true;
  else
    return false;
}

bool ICACHE_FLASH_ATTR write_json_to_flash(void)
{
  if (!jsonRoot)
  {
    INFO("Please parse jsonRoot before write it\n");
    return false;
  }
  char *out = NULL;
  int writeLen = 0;
  out = cJSON_Print(jsonRoot);
  INFO("\n%s\n",out);
  if (strlen(out)%4 == 0)
    writeLen = strlen(out);
  else
    writeLen = (strlen(out) / 4 + 1) * 4;
  spi_flash_erase_sector(CONFIG_JSON_ADDR/0x1000);
  spi_flash_write(CONFIG_JSON_ADDR,out,writeLen);
  os_free(out);
  return true;
}

bool ICACHE_FLASH_ATTR read_config(void)
{

  parse_json_from_flash();

  int returnvalue = (
  parse_json_to_string(jsonRoot, "mqtt_host", mqtt_host)&&
  parse_json_to_string(jsonRoot, "mqtt_client_id_prefix", mqtt_client_id_prefix)&&
  parse_json_to_string(jsonRoot, "mqtt_user", mqtt_user)&&
  parse_json_to_string(jsonRoot, "mqtt_pass", mqtt_pass)&&
  parse_json_to_string(jsonRoot, "mqtt_extra_sub_channel", mqtt_extra_sub_channel)&&
  parse_json_to_integer(jsonRoot, "mqtt_port", &mqtt_port)&&
  parse_json_to_integer(jsonRoot, "mqtt_recv_buf_size", &mqtt_recv_buf_size)&&
  parse_json_to_integer(jsonRoot, "mqtt_keepalive", &mqtt_keepalive)&&
  parse_json_to_integer(jsonRoot, "mqtt_qos", &mqtt_qos)&&
  parse_json_to_integer(jsonRoot, "mqtt_extra_sub_channel_enable", &mqtt_extra_sub_channel_enable)&&
  parse_json_to_integer(jsonRoot, "rx_buff_size", &rx_buff_size)&&
  parse_json_to_integer(jsonRoot, "tx_buff_size", &tx_buff_size)&&
  parse_json_to_integer(jsonRoot, "mqtt_pub_pack_cycle", &mqtt_pub_pack_cycle)&&
  parse_json_to_integer(jsonRoot, "mqtt_reconnect_timeout", &mqtt_reconnect_timeout)&&
  parse_json_to_integer(jsonRoot, "default_security", &default_security)&&
  parse_json_to_integer(jsonRoot, "queue_buffer_size", &queue_buffer_size)&&
  parse_json_to_integer(jsonRoot, "mqtt_clean_session", &mqtt_clean_session)&&
  parse_json_to_string(jsonRoot, "config_passwd", config_passwd)
  );
  //INFO ("%d", returnvalue);

  cJSON_Delete(jsonRoot);
  return returnvalue;
}


bool ICACHE_FLASH_ATTR isExpStr(uint8_t *str1,uint8_t *str2, uint8_t strlen)
{
  uint8_t i ;
  for (i = 0; i < strlen; i++)
  {
    if (str1[i] != str2[i])
      return FALSE;
  }
  if (str2[i] != '\0')
    return FALSE;
  return TRUE;
}

bool ICACHE_FLASH_ATTR isExpChannel(uint8_t *mqtt_channel_name,uint8_t *mqtt_type)
{
  uint8_t i = 0;
  while (mqtt_type[i])
  {
    if(mqtt_channel_name[i] != mqtt_type[i])
      return FALSE;
    i++;
  }
  return TRUE;
}

MQTT_Client mqttClient;

static void ICACHE_FLASH_ATTR Init_SerialBuff()
{
  //INFO("Init_SerialBuff\n");
  while(!(TX_FIFO_LEN(UART0)));
  QUEUE_Init(&rxBuff, rx_buff_size);
  QUEUE_Init(&txBuff, tx_buff_size);
  //*rxBuff = (char*)os_zalloc(rx_buff_size + 1);
  //*txBuff = (char*)os_zalloc(tx_buff_size + 1);
  //INFO("Comp_Init_SerialBuff\n");
}

static void ICACHE_FLASH_ATTR Rx2PubSend()
{
  //uint16_t pubBuffLen = rxBuff.rb.fill_cnt;
  uint16_t pubBuffLen = 0;
  uint16_t rxBuffLen,readedBuffLen;
  //INFO("rb.fill_cnt is %d\n",rxBuff.rb.fill_cnt);

  //char *tmpBuf = (char*)os_zalloc(rxBuff.rb.fill_cnt + 1);
  while (rxBuff.rb.fill_cnt > 10)
  {
    QUEUE_Gets(&rxBuff, (tmpBuffPub + pubBuffLen), &rxBuffLen, rx_buff_size);
    pubBuffLen += rxBuffLen;
    //INFO("$rb.fill_cnt is %d\n",rxBuff.rb.fill_cnt);
    //while(!(TX_FIFO_LEN(UART0)));
  }
  if (pubBuffLen)
    MQTT_Publish(&mqttClient, mqtt_send_channel, tmpBuffPub, pubBuffLen, mqtt_qos, 0);

  //uart0_tx_buffer(tmpBuf,pubBuffLen);
  //INFO("\nrb.fill_cnt is %d\n",rxBuff.rb.fill_cnt);

  //os_free(tmpBuf);
}

static void ICACHE_FLASH_ATTR init_Rx2PubSender()
{
  os_timer_disarm(&Rx2PubSender);
  os_timer_setfn(&Rx2PubSender, (os_timer_func_t *)Rx2PubSend, NULL);
  os_timer_arm(&Rx2PubSender, mqtt_pub_pack_cycle, TRUE);//Set Rx2PubSender cycle to mqtt_pub_pack_cycle,and repeat
}

static void ICACHE_FLASH_ATTR Sub2TxSend()
{
  //INFO("#");
  /*
  if(system_get_free_heap_size() > 30000)
  {
    INFO("Mem availble %lu\nSystem goes wrong\n", system_get_free_heap_size());
    system_show_malloc();
  }
  */
  uint8_t txFree,txNowLen=0;
  uint16_t txBuffLen;
  uint8_t QUEUE_Gets_Status;

  if(txBuff.rb.fill_cnt > 0)
  {
    txFree = 128 - TX_FIFO_LEN(UART1);
    //char tmpBuf[128];
    //char *tmpBuf = (char*)os_zalloc(txFree + 1);
    while (txBuff.rb.fill_cnt > 0 && txFree > 10)
    {
      INFO("$rb.fill_cnt is %d\n",txBuff.rb.fill_cnt);
      QUEUE_Gets_Status = QUEUE_Gets_Divided(&txBuff, tmpBufTx + txNowLen , &txBuffLen, txFree);
      INFO("%d\n",QUEUE_Gets_Status);
      txFree -= txBuffLen;
      txNowLen += txBuffLen;
      INFO("$getBuffSize %d\n",txBuffLen);
      INFO("$rb.fill_cnt is %d\n",txBuff.rb.fill_cnt);
    }

    uart1_tx_buffer(tmpBufTx, txNowLen);

    //os_free(tmpBuf);

  }

}

static void ICACHE_FLASH_ATTR init_Sub2TxSender()
{
  os_timer_disarm(&Sub2TxSender);
  os_timer_setfn(&Sub2TxSender, (os_timer_func_t *)Sub2TxSend, NULL);
  os_timer_arm(&Sub2TxSender, 10, TRUE);//Set Sub2TxSender cycle to 10ms,and repeat
}

static void ICACHE_FLASH_ATTR ControlTimeOut()
{
  wrong_passwd_count = 0;
  expect_mode = NORMAL_MODE;
  control_state = NORMAL_MODE;
  if (jsonRoot)
    cJSON_Delete(jsonRoot);
  MQTT_Publish(&mqttClient, mqtt_state_channel, "command timeout, back to NORMAL_MODE", 36, mqtt_qos, 0);
}


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
  MQTT_Subscribe(client, mqtt_recv_channel, mqtt_qos);
  MQTT_Subscribe(client, mqtt_ctrl_channel, mqtt_qos);
  //MQTT_Publish(client, mqtt_send_channel, "Hello", 6, mqtt_qos, 0);
  if (mqtt_extra_sub_channel_enable)
    MQTT_Subscribe(client, mqtt_extra_sub_channel, mqtt_qos);

  init_Rx2PubSender();
  init_Sub2TxSender();
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
  INFO("Receive topic: %s\r\n", topicBuf);
  //INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

  if(isExpChannel(topic,"/recv/"))
  {
    QUEUE_Puts(&txBuff, data, data_len);
    INFO("*\n");
  }

  else if(isExpChannel(topic,"/send/"))
  {
    QUEUE_Puts(&txBuff, data, data_len);
    INFO("*%d\n",data_len);
  }
  else if(isExpChannel(topic,"/ctrl/"))
  {
    os_timer_disarm(&ControlTimer);
    if (control_state == NORMAL_MODE)
    {
      if(isExpStr(data,"restart_system", data_len))
      {
        if(++restart_command_count >= 3)
        {
          restart_command_count = 0;
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Please enter password", 21, mqtt_qos, 0);
          control_state = AUTHING_MODE;
          expect_mode = RESET_MODE;
        }
        else
        {
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Repeat it, please.", 18, mqtt_qos, 0);
        }
      }
      else if(isExpStr(data,"edit_config", data_len))
      {
        restart_command_count = 0;
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Please enter password", 21, mqtt_qos, 0);
        control_state = AUTHING_MODE;
        expect_mode = EDIT_CONF_MODE;
      }
      else if(isExpStr(data,"control_gpio", data_len))
      {
        restart_command_count = 0;
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Going to GPIO_CTRL_MODE", 23, mqtt_qos, 0);
        control_state = GPIO_CTRL_MODE;
      }
      else if(isExpStr(data,"update_json_file", data_len))
      {
        restart_command_count = 0;
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Please enter password", 23, mqtt_qos, 0);
        control_state = AUTHING_MODE;
        expect_mode = UPDATE_JSON_MODE;
      }
      else
      {
        restart_command_count = 0;
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Command WRONG", 13, mqtt_qos, 0);
      }
    }
    else if (control_state == AUTHING_MODE)
    {
      if(isExpStr(data, config_passwd, data_len))
      {
        wrong_passwd_count = 0;
        if (expect_mode == RESET_MODE)
        {
          expect_mode = NORMAL_MODE;
          MQTT_Publish(&mqttClient, mqtt_state_channel, "System Restart NOW!", 19, mqtt_qos, 0);
          INFO("System Restart NOW!");
          system_restart();
        }
        else if(expect_mode == EDIT_CONF_MODE)
        {
          expect_mode = NORMAL_MODE;
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Going to EDIT_CONF_MODE", 23, mqtt_qos, 0);
          parse_json_from_flash();
          char *dataBuf = cJSON_Print(jsonRoot);
          MQTT_Publish(&mqttClient, mqtt_state_channel, dataBuf, os_strlen(dataBuf), mqtt_qos, 0);
          os_free(dataBuf);
          control_state = EDIT_CONF_MODE;
        }
        else if(expect_mode == UPDATE_JSON_MODE)
        {
          expect_mode = NORMAL_MODE;
          char configBuff[CONFIG_BUFF_SIZE];
          spi_flash_read(CONFIG_JSON_ADDR,configBuff,CONFIG_BUFF_SIZE);
          MQTT_Publish(&mqttClient, mqtt_state_channel, configBuff, CONFIG_BUFF_SIZE, mqtt_qos, 0);
          control_state = UPDATE_JSON_MODE;
        }
      }
      else
      {
        if (++wrong_passwd_count >= 3)
        {
          wrong_passwd_count = 0;
          expect_mode = NORMAL_MODE;
          control_state = NORMAL_MODE;
          MQTT_Publish(&mqttClient, mqtt_state_channel, "too much incorrect attempts", 27, mqtt_qos, 0);
        }
        else
        {
          MQTT_Publish(&mqttClient, mqtt_state_channel, "password WRONG, please retry", 28, mqtt_qos, 0);
        }
      }
    }
    else if (control_state == GPIO_CTRL_MODE)
    {
      if(isExpStr(data, "exit", data_len))
      {
        expect_mode = NORMAL_MODE;
        control_state = NORMAL_MODE;
        if (jsonRoot)
          cJSON_Delete(jsonRoot);
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Back to NORMAL_MODE", 19, mqtt_qos, 0);
      }
      else
      {
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Command WRONG", 13, mqtt_qos, 0);
      }
    }
    else if (control_state == EDIT_CONF_MODE)
    {
      if(isExpStr(data, "save_config_to_flash", data_len))
      {
        write_json_to_flash();
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Config saved to flash", 21, mqtt_qos, 0);
        expect_mode = NORMAL_MODE;
        control_state = NORMAL_MODE;
      }
      else if(isExpStr(data, "exit", data_len))
      {
        expect_mode = NORMAL_MODE;
        control_state = NORMAL_MODE;
        if (jsonRoot)
          cJSON_Delete(jsonRoot);
        MQTT_Publish(&mqttClient, mqtt_state_channel, "Back to NORMAL_MODE", 19, mqtt_qos, 0);
      }
      else
      {
        //INFO("%s\n",cJSON_Print(jsonRoot));
        uint8_t i;
        uint8_t keylen = 0;
        cJSON *childObj;
        char keyname[data_len + 1];
        for (i = 0; *(data + i) != ' ' && i < data_len; i++)
        {
          keyname[i] = *(data + i);
        }
        keylen = i;
        keyname[keylen] = '\0';
        //INFO("%s\n%d\n",keyname, keylen);
        while(!(TX_FIFO_LEN(UART0)));
        childObj = cJSON_GetObjectItem(jsonRoot, keyname);
        if (childObj && i < data_len)
        {
          //INFO("In cJSON_HasObjectItem\n");
          while(!(TX_FIFO_LEN(UART0)));
          uint8_t contentlen = 0;
          char content[data_len - keylen + 1];
          for (i++ ; *(data + i) != ' ' && i < data_len; i++)
          {
            contentlen = i - keylen;
            content[contentlen - 1] = *(data + i);
          }
          content[contentlen] = '\0';
          //INFO("%s\n%s\n",keyname, content);
          while(!(TX_FIFO_LEN(UART0)));
          if(cJSON_IsNumber(childObj))
          {
            //INFO("start cJSON_SetNumberValue\n");
            cJSON_SetNumberValue(childObj, atoi(content));
          }
          else
          {
            //INFO("start cJSON_ReplaceStringInObject\n");
            cJSON_ReplaceStringInObject(jsonRoot, keyname, content);
          }
          //DO NOT free childObj memory space!
          INFO("Comp Updated config\n");
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Updated config", 14, mqtt_qos, 0);
        }
        else
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Command WRONG", 13, mqtt_qos, 0);
        //INFO("%s\n",cJSON_Print(jsonRoot));
      }
    }
    else if (control_state == UPDATE_JSON_MODE)
    {
      if(data_len <= CONFIG_BUFF_SIZE)
      {
        char jsonstr[data_len + 1];
        os_memcpy(jsonstr, data, data_len);
        jsonstr[data_len + 1] = '\0';
        if (jsonRoot)
          cJSON_Delete(jsonRoot);
        jsonRoot = cJSON_Parse(jsonstr);
        if(jsonRoot)
        {
          char *out = NULL;
          int writeLen = 0;
          out = cJSON_Print(jsonRoot);
          INFO("\n%s\n",out);
          if (strlen(out)%4 == 0)
            writeLen = strlen(out);
          else
            writeLen = (strlen(out) / 4 + 1) * 4;
          spi_flash_erase_sector(CONFIG_JSON_ADDR/0x1000);
          spi_flash_write(CONFIG_JSON_ADDR,out,writeLen);
          os_free(out);
          cJSON_Delete(jsonRoot);
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Update Complete, System Restart NOW!", 36, mqtt_qos, 0);
          INFO("Update Complete, System Restart NOW!");
          system_restart();
        }
        else if(isExpStr(data,"exit", data_len))
        {
          expect_mode = NORMAL_MODE;
          control_state = NORMAL_MODE;
          if (jsonRoot)
            cJSON_Delete(jsonRoot);
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Back to NORMAL_MODE", 19, mqtt_qos, 0);
        }
        else
          MQTT_Publish(&mqttClient, mqtt_state_channel, "Json file ERROR", 15, mqtt_qos, 0);
      }
    }


    if (control_state != NORMAL_MODE)
    {
      os_timer_setfn(&ControlTimer, (os_timer_func_t *)ControlTimeOut, NULL);
      os_timer_arm(&ControlTimer, CONTROL_TIMEOUT, FALSE);
    }
  }

  os_free(topicBuf);
  os_free(dataBuf);
}


void ICACHE_FLASH_ATTR print_info()
{
  //INFO("TX_FIFO_LEN(UART0): %d\n",TX_FIFO_LEN(UART0));
  INFO("\r\n\r\n[INFO] BOOTUP...\r\n");
  INFO("[INFO] SDK: %s\r\n", system_get_sdk_version());
  INFO("[INFO] Chip ID: %08X\r\n", system_get_chip_id());
  INFO("[INFO] Memory info:\r\n");
  system_print_meminfo();

  INFO("[INFO] -------------------------------------------\n");
  INFO("[INFO] Build time: %s\n", BUID_TIME);
  INFO("[INFO] -------------------------------------------\n");

  //INFO("TX_FIFO_LEN(UART0): %d\n",TX_FIFO_LEN(UART0));
  //INFO("TX_FIFO_LEN(UART1): %d\n",TX_FIFO_LEN(UART1));
}

void ICACHE_FLASH_ATTR conf_mqtt_channel_name()
{
  uint8_t sta_mac[6];
  wifi_get_macaddr(STATION_IF,sta_mac);
  //os_sprintf(mqtt_send_channel, "/%s", ssid);
  os_sprintf(mqtt_client_id, "%s""%02x%02x%02x%02x%02x%02x", mqtt_client_id_prefix, MAC2STR(sta_mac));
  os_sprintf(mqtt_send_channel, "/send/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_recv_channel, "/recv/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_ctrl_channel, "/ctrl/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_state_channel, "/state/""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_online_msg, "online ""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));
  os_sprintf(mqtt_offline_msg, "offline ""%02x%02x%02x%02x%02x%02x", MAC2STR(sta_mac));

}

static void ICACHE_FLASH_ATTR app_init(void)
{

  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  print_info();

  read_config();

  conf_mqtt_channel_name();

  Init_SerialBuff();

  MQTT_InitConnection(&mqttClient, mqtt_host, mqtt_port, default_security);
  //MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

  if ( !MQTT_InitClient(&mqttClient, mqtt_client_id, mqtt_user, mqtt_pass, mqtt_keepalive, mqtt_clean_session) )
  {
    INFO("Failed to initialize properly. Check MQTT version.\r\n");
    return;
  }
  //MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);
  //MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
  MQTT_OnConnected(&mqttClient, mqttConnectedCb);
  MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
  MQTT_OnPublished(&mqttClient, mqttPublishedCb);
  MQTT_OnData(&mqttClient, mqttDataCb);
	WIFI_Connect(wifiConnectCb);
  //init_MQTT_Rx2PubSender();
  //cJSON_test();

  INFO("memleak_debug_enable %d\n",check_memleak_debug_enable());
  //INFO("TX_FIFO_LEN(UART0): %d\n",TX_FIFO_LEN(UART0));
	//INFO("*_*Comp app_init\n");

  MQTT_InitLWT(&mqttClient, mqtt_state_channel, mqtt_offline_msg, mqtt_qos, 0);
  MQTT_Publish(&mqttClient, mqtt_state_channel, mqtt_online_msg, 19, mqtt_qos, 0);

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
