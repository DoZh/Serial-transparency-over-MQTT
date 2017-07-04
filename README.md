**Serial transparency over MQTT**
==========
This is a serial transparency application that can across Internet, run on ESP8266, use MQTT client library by Tuan PM



**Features:**

 * Support serial transparency application across Internet
 * Communication over "raw" MQTT Message that can easily interactive with other application
 * Use airkiss and esptouch to config WiFi connection
 * Easy to setup and use

***Prerequire:***

- ESPTOOL.PY: https://github.com/themadinventor/esptool
- SDK 2.0 or higher: http://bbs.espressif.com/viewtopic.php?f=46&t=2451
- ESP8266 compiler:
    + OSX or Linux: http://tuanpm.net/esp8266-development-kit-on-mac-os-yosemite-and-eclipse-ide/
    + Windows: http://programs74.ru/udkew-en.html

**Compile:**

- Copy file `include/user_config.sample.h` to `include/user_config.local.h` and change settings, included: SSID, PASS, MQTT configurations ...


Make sure to add PYTHON PATH and compile PATH to Eclipse environment variable if using Eclipse


```bash
git clone --recursive https://github.com/DoZh/Serial-transparency-over-MQTT.git
cd esp_mqtt
#clean
make clean
#make
make SDK_BASE=/tools/esp8266/sdk/ESP8266_NONOS_SDK ESPTOOL=tools/esp8266/esptool/esptool.py all
#flash
make ESPPORT=/dev/ttyUSB0 flash
```

**Usage**

To be continue........
Otherwise, you can diff the code with Tuan PM 's esp_mqtt repo, I think it's easily to understand.



**Contributing:**

Feel free to contribute to the project in any way you like!


**Authors:**
[DoZh](https://github.com/DoZh)


**LICENSE - "MIT License"**
