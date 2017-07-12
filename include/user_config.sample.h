#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define MQTT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define mqtt_host     "58.242.95.172" //or "mqtt.yourdomain.com"
#define mqtt_port     1883
#define mqtt_recv_process_buf_size   1500   //Recv Buff Size
#define mqtt_keepalive    120  /*second*/

#define mqtt_client_id_prefix    "Serial_test_"
#define mqtt_user     "USER"
#define mqtt_pass     "PASS"
#define mqtt_clean_session 1
#define mqtt_keepalive 120
#define mqtt_qos 0

//#define mqtt_extra_sub_channel "/send/60019480e975"
#define mqtt_extra_sub_channel "/send/60019480dd2d"

//it can't process mqtt package grater than MTU size
#define rx_buff_size 1400
#define tx_buff_size 5120

#define mqtt_pub_pack_cycle 100  /*microsecond*/

#define mqtt_reconnect_timeout  5 /*second*/

#define default_security  0
#define queue_buffer_size       5120   //Send Queue Size

#define PROTOCOL_NAMEv31  /*MQTT version 3.1 compatible with Mosquitto v0.15*/
//PROTOCOL_NAMEv311     /*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#if defined(DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

#endif // __MQTT_CONFIG_H__
