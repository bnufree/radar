#ifndef RADARCCONTROLDEFINES_H
#define RADARCCONTROLDEFINES_H

#include <stdint.h>

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#pragma pack(push,1)
struct RadarReport_01C4_18 {  // 01 C4 with length 18
    uint8_t	what;                 // 0   0x01
    uint8_t	command;              // 1   0xC4
    uint8_t	radar_status;         // 2
    uint8_t	field3;               // 3
    uint8_t	field4;               // 4
    uint8_t	field5;               // 5
    uint16_t	field6;              // 6-7
    uint16_t	field8;              // 8-9
    uint16_t	field10;             // 10-11
};

struct RadarReport_02C4_99 {     // length 99
    uint8_t	what;                    // 0   0x02
    uint8_t	command;                 // 1 0xC4
    uint32_t	range;                  //  2-3   0x06 0x09
    uint16_t	field4;                 // 6-7    0
    uint32_t	field8;                 // 8-11   1
    uint8_t	gain;                    // 12
    uint8_t	field13;                 // 13  ==1 for sea auto
    uint8_t	field14;                 // 14
    uint16_t	field15;                // 15-16
    uint32_t	sea;                    // 17-20   sea state (17)
    uint8_t	field21;                 // 21
    uint8_t	rain;                    // 22   rain clutter
    uint8_t	field23;                 // 23
    uint32_t	field24;                // 24-27
    uint32_t	field28;                // 28-31
    uint8_t	field32;                 // 32
    uint8_t	field33;                 // 33
    uint8_t	interference_rejection;  // 34
    uint8_t	field35;                 // 35
    uint8_t	field36;                 // 36
    uint8_t	field37;                 // 37
    uint8_t	target_expansion;        // 38
    uint8_t	field39;                 // 39
    uint8_t	field40;                 // 40
    uint8_t	field41;                 // 41
    uint8_t	target_boost;            // 42
};

struct RadarReport_03C4_129 {
    uint8_t	what;
    uint8_t	command;
    uint8_t	radar_type;  // I hope! 01 = 4G and new 3G, 08 = 3G, 0F = BR24, 00 = HALO
    uint8_t	u00[55];     // Lots of unknown
    uint16_t	firmware_date[16];
    uint16_t	firmware_time[16];
    uint8_t	u01[7];
};

struct RadarReport_04C4_66 {  // 04 C4 with length 66
    uint8_t	what;                 // 0   0x04
    uint8_t	command;              // 1   0xC4
    uint32_t	field2;              // 2-5
    uint16_t	bearing_alignment;   // 6-7
    uint16_t	field8;              // 8-9
    uint16_t	antenna_height;      // 10-11
};

struct RadarReport_08C4_18 {           // 08 c4  length 18
    uint8_t what;                          // 0  0x08
    uint8_t command;                       // 1  0xC4
    uint8_t field2;                        // 2
    uint8_t local_interference_rejection;  // 3
    uint8_t scan_speed;                    // 4
    uint8_t sls_auto;                      // 5 installation: sidelobe suppression auto
    uint8_t field6;                        // 6
    uint8_t field7;                        // 7
    uint8_t field8;                        // 8
    uint8_t side_lobe_suppression;         // 9 installation: sidelobe suppression
    uint16_t field10;                      // 10-11
    uint8_t noise_rejection;               // 12    noise rejection
    uint8_t target_sep;                    // 13
};

struct RadarReport_08C4_21 {
  RadarReport_08C4_18 old;
  uint8_t doppler_state;
  uint16_t doppler_speed;
};

struct RadarReport_12C4_66 {  // 12 C4 with length 66
  // Device Serial number is sent once upon network initialization only
  uint8_t what;          // 0   0x12
  uint8_t command;       // 1   0xC4
  uint8_t serialno[12];  // 2-13 Device serial number at 3G (All?)
};

struct PackedAddress {
  struct in_addr addr;
  uint16_t port;
};

struct RadarReport_01B2 {
  char serialno[16];  // ASCII serial number, zero terminated
  uint8_t u1[18];
  PackedAddress addr1;   // EC0608201970
  uint8_t u2[4];         // 11000000
  PackedAddress addr2;   // EC0607161A26
  uint8_t u3[10];        // 1F002001020010000000
  PackedAddress addr3;   // EC0608211971
  uint8_t u4[4];         // 11000000
  PackedAddress addr4;   // EC0608221972
  uint8_t u5[10];        // 10002001030010000000
  PackedAddress addr5;   // EC0608231973
  uint8_t u6[4];         // 11000000
  PackedAddress addr6;   // EC0608241974
  uint8_t u7[4];         // 12000000
  PackedAddress addr7;   // EC0608231975
  uint8_t u8[10];        // 10002002030010000000
  PackedAddress addr8;   // EC0608251976
  uint8_t u9[4];         // 11000000
  PackedAddress addr9;   // EC0608261977
  uint8_t u10[4];        // 12000000
  PackedAddress addr10;  // EC0608251978
  uint8_t u11[10];       // 12002001030010000000
  PackedAddress addr11;  // EC0608231979
  uint8_t u12[4];        // 11000000
  PackedAddress addr12;  // EC060827197A
  uint8_t u13[4];        // 12000000
  PackedAddress addr13;  // EC060823197B
  uint8_t u14[10];       // 12002002030010000000
  PackedAddress addr14;  // EC060825197C
  uint8_t u15[10];       // 11000000
  PackedAddress addr15;  // EC060828197D
  uint8_t u16[10];       // 12000000
  PackedAddress addr16;  // EC060825197E
};

#pragma pack(pop)

#endif // RADARCCONTROLDEFINES_H
