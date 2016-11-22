#ifndef OMNILOC_H
#define OMNILOC_H

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

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSSTR "[\"%02X:%02X:%02X:%02X:%02X:%02X\",\"%02X:%02X:%02X:%02X:%02X:%02X\",\"%02X:%02X:%02X:%02X:%02X:%02X\"],"
#define MACS2STR(a) (a)[4], (a)[5], (a)[6], (a)[7], (a)[8], (a)[9], (a)[10], (a)[11], (a)[12], (a)[13], (a)[14], (a)[15], (a)[16], (a)[17], (a)[18], (a)[19], (a)[20], (a)[21]

void ICACHE_FLASH_ATTR promisc_cb(uint8 *buf, uint16 len);

#endif
