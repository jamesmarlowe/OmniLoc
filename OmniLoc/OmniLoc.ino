// Omniloc hackathon project
// Members:
//   James Marlowe
//   Michael Orlando
//   Jonathan Leek
//   Daniel Benedict
// Objective:
//   Passively gather MAC addresses
//   Report them to server

#include <OmniLoc.h>

const char* ssid     = ""; // local wifi network
const char* password = ""; // local wifi password
const char* host = "";     // Send collected data here
const int hostPort = 80;
const char* url = "/datacapture/";
char localMac[18] = { 0 };
String macBuffer = "";

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

// craft HTTP POST to send
String build_request(){
  String body = String("{\"node\":\"") + localMac + "\",\"beacons\":[" + macBuffer + "]}";
  return String("POST ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + body.length() + "\r\n" +
                   "Connection: close\r\n\r\n" +
                   body;
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
  String request = build_request();
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
  client.flush();
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
