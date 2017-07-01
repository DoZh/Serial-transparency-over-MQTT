#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define MQTT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST     "58.242.95.172" //or "mqtt.yourdomain.com"
#define MQTT_PORT     1883
#define MQTT_BUF_SIZE   1500   //Recv Buff Size
#define MQTT_KEEPALIVE    120  /*second*/

#define MQTT_CLIENT_ID_PREFIX    "Serial_test_"
#define MQTT_USER     "USER"
#define MQTT_PASS     "PASS"
#define MQTT_CLEAN_SESSION 1
#define MQTT_KEEPALIVE 120
#define MQTT_QOS 0

//#define MQTT_EXTRA_SUB_CHANNEL "/send/60019480e975"
#define MQTT_EXTRA_SUB_CHANNEL "/send/60019480dd2d"

//it can't process mqtt package grater than MTU size
#define RX_BUFF_SIZE 1400
#define TX_BUFF_SIZE 5120

#define MQTT_PUB_PACK_CYCLE 100  /*microsecond*/

#define MQTT_RECONNECT_TIMEOUT  5 /*second*/

#define DEFAULT_SECURITY  0
#define QUEUE_BUFFER_SIZE       5120   //Send Queue Size

#define PROTOCOL_NAMEv31  /*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311     /*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#if defined(DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

#endif // __MQTT_CONFIG_H__
