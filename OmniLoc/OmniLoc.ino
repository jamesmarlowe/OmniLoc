#ifdef ESP8266
extern "C" {
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
}
#endif
#include <ESP8266WiFi.h>

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSSTR "[\"%02X:%02X:%02X:%02X:%02X:%02X\",\"%02X:%02X:%02X:%02X:%02X:%02X\",\"%02X:%02X:%02X:%02X:%02X:%02X\"],"
#define MACS2STR(a) (a)[4], (a)[5], (a)[6], (a)[7], (a)[8], (a)[9], (a)[10], (a)[11], (a)[12], (a)[13], (a)[14], (a)[15], (a)[16], (a)[17], (a)[18], (a)[19], (a)[20], (a)[21]

const char* ssid     = ""; // local wifi network
const char* password = ""; // local wifi password
const char* host = "";     // Send collected data here
const int hostPort = 80;
const char* url = "/datacapture/";
char localMac[18] = { 0 };
String macBuffer = "";

// *len == 12* packets
struct RxControl {
  signed rssi: 8;
  unsigned rate: 4;
  unsigned is_group: 1;
  unsigned: 1;
  unsigned sig_mode: 2;
  unsigned legacy_length: 12;
  unsigned damatch0: 1;
  unsigned damatch1: 1;
  unsigned bssidmatch0: 1;
  unsigned bssidmatch1: 1;
  unsigned MCS: 7;
  unsigned CWB: 1;
  unsigned HT_length: 16;
  unsigned Smoothing: 1;
  unsigned Not_Sounding: 1;
  unsigned: 1;
  unsigned Aggregation: 1;
  unsigned STBC: 2;
  unsigned FEC_CODING: 1;
  unsigned SGI: 1;
  unsigned rxend_state: 8;
  unsigned ampdu_cnt: 8;
  unsigned channel: 4;
  unsigned: 12;
};

struct LenSeq {
  uint16_t length;
  uint16_t seq;
  uint8_t address3[6];
};

struct sniffer_buf {
  struct RxControl rx_ctrl;
  uint8_t buf[36];
  uint16_t cnt;
  struct LenSeq lenseq[1];
};

// *len == 128* packets
struct sniffer_buf_large {
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len;
};
struct sniffer_buf_large *sniffer;

// packet sniff callback function which buffers macs into string
void ICACHE_FLASH_ATTR promisc_cb(uint8 *buf, uint16 len) {
  if (len == 128) {
    sniffer = (struct sniffer_buf_large*) buf;
    char mac[62] = {0};
    sprintf(mac, MACSSTR , MACS2STR(sniffer->buf));
    macBuffer += mac;
  }
}

// packet sniffing setup funtion
void packet_sniff() {
  wifi_set_opmode(0x1);
  //wifi_set_channel(6);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  while (macBuffer.length() < 2000) {
    delay(50);
  }
}

// connect to wifi to send captured macs up
void wifi_connect() {
  if (strlen(password) == 0) {
    WiFi.begin(ssid);
  }
  else {
    WiFi.begin(ssid, password);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// send the data to the central server
void send_data() {
  wifi_connect();
  WiFiClient client;
  if (!client.connect(host, hostPort)) {
    Serial.println("connection failed");
    return;
  }

  // build and send HTTP POST request
  macBuffer.remove(macBuffer.length()-1);
  String body = String("{\"node\":\"") + localMac + "\",\"beacons\":[" + macBuffer + "]}";
  String request = String("POST ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + body.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   body;
  Serial.println(request);
  client.print(request);
  macBuffer = "";

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}

// called at start by arduino
void setup() {
  Serial.begin(115200); // set baud rate of serial
  delay(10);
  uint8_t mac[6];
  wifi_get_macaddr(SOFTAP_IF, mac); // get local wifi mac
  sprintf(localMac, MACSTR, MAC2STR(mac));
}

// called perpetually by arduino
void loop() {
  packet_sniff(); // gather data
  wifi_promiscuous_enable(0); // 0 disables: stop gathering
  send_data();    // send data
}

