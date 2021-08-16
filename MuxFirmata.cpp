/*
  MuxFirmata is a modified version of StandardFirmata
  It depends on a HAL defined in a modified version of boards.h
  which is for use with Arduino UNO R3 and Mayhew Labs MuxShieldII only

  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.
  Copyright (C) 2016 Jim French. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

*/

#include <Arduino.h>

#include "SendOnlySoftwareSerial.h"
#include "MuxShields.h"
#include "Firmata.h"

extern "C" {
    #include <string.h>
}

extern HardwareSerial Serial;

SendOnlySoftwareSerial SerialOut(13, false);

MuxShield Mux;

static boolean const bDebug = true;
static boolean const bRunOnce = false;
static boolean const bSelfTest = false;
static boolean const bSendStatus = true;
static boolean const bSampleAnalog = true;
static boolean const bSampleDigital = true;
static boolean const bUseDigitalRate= true;

static unsigned long const ulRateHardware = 57600;
static unsigned long const ulRateSoftware = 19200;

static byte const bPortOff = 0;
static byte const bPortOn = 255;

static byte const bDebugLen = 220;

static char const sStatusSerialUp[] PROGMEM     = "MuxFirmata Debugger";
static char const sStatusRateHardware[] PROGMEM = "Serial rate main I/O  (bps): | ";
static char const sStatusRateSoftware[] PROGMEM = "Serial rate debug out (bps): | ";   
static char const sStatusASample[] PROGMEM      = "Sample rate analogue   (mS): | ";
static char const sStatusDSample[] PROGMEM      = "Sample rate digital    (mS): | ";
static char const sStatusSelfTest[] PROGMEM     = "         Self test mode is : | ";

static char const sStatusModeAIN[] PROGMEM      = "| Analogue input                                                  ";
static char const sStatusModeDIN[] PROGMEM      = "| Digital input    ";
static char const sStatusModeDOUT[] PROGMEM     = "| Digital output   ";
static char const sStatusModeDINP[] PROGMEM     = "| Digital input PU ";
static char const sStatusModeNONE[] PROGMEM     = "| Unsupported      ";

static char const sStatusPorts[] PROGMEM        = "  Status of: | I/O PORT 6:      | I/O PORT 5:      | I/O PORT 4:      | I/O PORT 3:      | I/O PORT 2:      | I/O PORT 1:                                                     | SYSTEM:";
static char const sStatusModes[] PROGMEM        = "  Port mode: ";
static char const sStatusRead[] PROGMEM         = "  Port read:";
static char const sStatusUptime[] PROGMEM       = "Up: ";
static char const sStatusMem[] PROGMEM          = "Bytes free: ";

static char const s16spaces[] PROGMEM           = "                ";
static char const s7bits[] PROGMEM              = "0000000";
static char const s6bits[] PROGMEM              = "000000";
static char const s5bits[] PROGMEM              = "00000";
static char const s4bits[] PROGMEM              = "0000";
static char const s3bits[] PROGMEM              = "000";
static char const s2bits[] PROGMEM              = "00";
static char const s1bits[] PROGMEM              = "0";

static char const sStatusLine[] PROGMEM         = "_";
static char const sStatusDel2[] PROGMEM         = "|";
static char const sStatusDel[] PROGMEM          = " | ";
static char const sStatusOff[] PROGMEM          = "<OFF>";
static char const sStatusOn[] PROGMEM           = "<ON>";

int aAnalogRead[TOTAL_ANALOG_PINS] = {0};

byte reportPINs[TOTAL_PORTS];
byte previousPINs[TOTAL_PORTS];
byte portConfigInputs[TOTAL_PORTS];

boolean isResetting = false;
boolean isUp = false;
boolean isOnce = false;

byte testPort = MUX_PORT_6;
byte testCount = MUX_PORT_6;
byte testPrevious = 0;
boolean testMode = false;

char ulBuf[sizeof(unsigned long)*8+1];

char sPBuf[256];

unsigned long ulUptimeSecs = 0;
int uFreeRAM = 0;

unsigned long ulUptimeRate = 1000;
unsigned long ulUptimeC = 0, ulUptimeP = 0;

unsigned long ulSampleRate = 19;
unsigned long ulSampleC = 0, ulSampleP = 0;

unsigned long ulDSampleRate = 4;
unsigned long ulDSampleC = 0, ulDSampleP = 0;

unsigned long ulDWriteRate = 250;
unsigned long ulDWriteC = 0, ulDWriteP = 0;

unsigned long ulDebugRate = 100;
unsigned long ulDebugC = 0, ulDebugP = 0;


class Printer : public Print{
public:
    virtual size_t write(uint8_t)  { return 1; }
};

Printer PrintDebug;

char* pmstr(const char* str){
    strcpy_P(sPBuf, (char*)str);
    return sPBuf;
}

static inline unsigned char readPort(byte, byte) __attribute__((always_inline, unused));
static inline unsigned char readPort(byte port, byte bitmask)
{
    unsigned char out = 0, pin = port * 8;   
    
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN1(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(1,PIN_TO_MUX_PORT_1(pin + 7))) out |= 0x80;
    
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN2(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(2,PIN_TO_MUX_PORT_2(pin + 7))) out |= 0x80;

    if (IS_PIN_DIGITAL_MUX_IN3(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN3(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(3,PIN_TO_MUX_PORT_3(pin + 7))) out |= 0x80;

    if (IS_PIN_DIGITAL_MUX_IN4(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN4(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(4,PIN_TO_MUX_PORT_4(pin + 7))) out |= 0x80;
    
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN5(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(5,PIN_TO_MUX_PORT_5(pin + 7))) out |= 0x80;
    
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 0) && (bitmask & 0x01) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 0))) out |= 0x01;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 1) && (bitmask & 0x02) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 1))) out |= 0x02;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 2) && (bitmask & 0x04) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 2))) out |= 0x04;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 3) && (bitmask & 0x08) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 3))) out |= 0x08;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 4) && (bitmask & 0x10) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 4))) out |= 0x10;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 5) && (bitmask & 0x20) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 5))) out |= 0x20;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 6) && (bitmask & 0x40) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 6))) out |= 0x40;
    if (IS_PIN_DIGITAL_MUX_IN6(pin + 7) && (bitmask & 0x80) && Mux.digitalReadMS(6,PIN_TO_MUX_PORT_6(pin + 7))) out |= 0x80;

    return out;
}


static inline unsigned char writePort(byte, byte, byte) __attribute__((always_inline, unused));
static inline unsigned char writePort(byte port, byte value, byte bitmask)
{

    byte pin = port * 8;

    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT1(pin + 0)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT1(pin + 1)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT1(pin + 2)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT1(pin + 3)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT1(pin + 4)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT1(pin + 5)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT1(pin + 6)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT1(pin + 7)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin + 7), (value & 0x80));
    
    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT2(pin + 0)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT2(pin + 1)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT2(pin + 2)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT2(pin + 3)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT2(pin + 4)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT2(pin + 5)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT2(pin + 6)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT2(pin + 7)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin + 7), (value & 0x80));
    
    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT3(pin + 0)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT3(pin + 1)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT3(pin + 2)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT3(pin + 3)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT3(pin + 4)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT3(pin + 5)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT3(pin + 6)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT3(pin + 7)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin + 7), (value & 0x80));
    
    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT4(pin + 0)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT4(pin + 1)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT4(pin + 2)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT4(pin + 3)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT4(pin + 4)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT4(pin + 5)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT4(pin + 6)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT4(pin + 7)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin + 7), (value & 0x80));
    
    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT5(pin + 0)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT5(pin + 1)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT5(pin + 2)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT5(pin + 3)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT5(pin + 4)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT5(pin + 5)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT5(pin + 6)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT5(pin + 7)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin + 7), (value & 0x80));

    if ((bitmask & 0x01) && IS_PIN_DIGITAL_MUX_OUT6(pin + 0)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 0), (value & 0x01));
    if ((bitmask & 0x02) && IS_PIN_DIGITAL_MUX_OUT6(pin + 1)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 1), (value & 0x02));
    if ((bitmask & 0x04) && IS_PIN_DIGITAL_MUX_OUT6(pin + 2)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 2), (value & 0x04));
    if ((bitmask & 0x08) && IS_PIN_DIGITAL_MUX_OUT6(pin + 3)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 3), (value & 0x08));
    if ((bitmask & 0x10) && IS_PIN_DIGITAL_MUX_OUT6(pin + 4)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 4), (value & 0x10));
    if ((bitmask & 0x20) && IS_PIN_DIGITAL_MUX_OUT6(pin + 5)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 5), (value & 0x20));
    if ((bitmask & 0x40) && IS_PIN_DIGITAL_MUX_OUT6(pin + 6)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 6), (value & 0x40));
    if ((bitmask & 0x80) && IS_PIN_DIGITAL_MUX_OUT6(pin + 7)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin + 7), (value & 0x80));

    pin = 1;
    return pin;
}


void outputPort(byte portNumber, byte portValue, byte forceSend)
{  
    portValue = (portValue & portConfigInputs[portNumber]);
  
    if (forceSend || previousPINs[portNumber] != portValue) {      
        Firmata.sendDigitalPort(portNumber, portValue);
        previousPINs[portNumber] = portValue;
    }
}


void checkDigitalInputs(void)
{
    
    size_t uLen = 0;
    
    if (TOTAL_PORTS > 0 && reportPINs[0]) outputPort(0, readPort(0, portConfigInputs[0]), false);  
    if (TOTAL_PORTS > 1 && reportPINs[1]) outputPort(1, readPort(1, portConfigInputs[1]), false);
    if (TOTAL_PORTS > 2 && reportPINs[2]) outputPort(2, readPort(2, portConfigInputs[2]), false);
    if (TOTAL_PORTS > 3 && reportPINs[3]) outputPort(3, readPort(3, portConfigInputs[3]), false);
    if (TOTAL_PORTS > 4 && reportPINs[4]) outputPort(4, readPort(4, portConfigInputs[4]), false);
    if (TOTAL_PORTS > 5 && reportPINs[5]) outputPort(5, readPort(5, portConfigInputs[5]), false);
    if (TOTAL_PORTS > 6 && reportPINs[6]) outputPort(6, readPort(6, portConfigInputs[6]), false);
    if (TOTAL_PORTS > 7 && reportPINs[7]) outputPort(7, readPort(7, portConfigInputs[7]), false);
    if (TOTAL_PORTS > 8 && reportPINs[8]) outputPort(8, readPort(8, portConfigInputs[8]), false);
    if (TOTAL_PORTS > 9 && reportPINs[9]) outputPort(9, readPort(9, portConfigInputs[9]), false);
    if (TOTAL_PORTS > 10 && reportPINs[10]) outputPort(10, readPort(10, portConfigInputs[10]), false);
    if (TOTAL_PORTS > 11 && reportPINs[11]) outputPort(11, readPort(11, portConfigInputs[11]), false);
    if (TOTAL_PORTS > 12 && reportPINs[12]) outputPort(12, readPort(12, portConfigInputs[12]), false);
    if (TOTAL_PORTS > 13 && reportPINs[13]) outputPort(13, readPort(13, portConfigInputs[13]), false);
    if (TOTAL_PORTS > 14 && reportPINs[14]) outputPort(14, readPort(14, portConfigInputs[14]), false);
    if (TOTAL_PORTS > 15 && reportPINs[15]) outputPort(15, readPort(15, portConfigInputs[15]), false);

    if(bDebug){
        ulDebugC = millis();
        if (ulDebugC - ulDebugP > ulDebugRate){
            ulDebugP = ulDebugC;
            
            SerialOut.print(pmstr(s16spaces));
            SerialOut.print(pmstr(sStatusRead));
                        
            for(int uPort = TOTAL_PORTS-1; uPort >=0; uPort--){
                if (uPort % 2 > 0) SerialOut.print(pmstr(sStatusDel));
                
                if(reportPINs[uPort]){                  // digital in
                    
                    uLen= PrintDebug.print(previousPINs[uPort], BIN);
                    if(uLen==1) SerialOut.print(pmstr(s7bits));
                    if(uLen==2) SerialOut.print(pmstr(s6bits));
                    if(uLen==3) SerialOut.print(pmstr(s5bits));
                    if(uLen==4) SerialOut.print(pmstr(s4bits));
                    if(uLen==5) SerialOut.print(pmstr(s3bits));
                    if(uLen==6) SerialOut.print(pmstr(s2bits));
                    if(uLen==7) SerialOut.print(pmstr(s1bits)); 
                    
                    SerialOut.print(previousPINs[uPort], BIN);
                }
                
                else if (portConfigInputs[uPort]){      // analogue in
                }
                
                else {                                  // digital out
                    if (uPort % 2 > 0) SerialOut.print(pmstr(s16spaces));                    
                }
            }
                       
            for(int pin = TOTAL_ANALOG_PINS-1; pin >=0; pin--){
                        
                uLen= PrintDebug.print(aAnalogRead[pin], HEX);
                if(uLen==1) SerialOut.print(pmstr(s2bits));
                if(uLen==2) SerialOut.print(pmstr(s1bits));

                SerialOut.print(aAnalogRead[pin], HEX);
                if(pin > 0) SerialOut.print(pmstr(sStatusDel2));
            }
            
            SerialOut.print(pmstr(sStatusDel));

            SerialOut.print(pmstr(sStatusUptime));
            SerialOut.print(ulUptimeSecs);

            SerialOut.print(pmstr(sStatusDel));        
            SerialOut.print(pmstr(sStatusMem));    
            SerialOut.print(uFreeRAM);
            SerialOut.write(13);
        }
    }
}


void reportAnalogCallback(byte analogPin, int value)
{
    if (analogPin < TOTAL_ANALOG_PINS) {
        if (value != 0) {
          if (!isResetting) {
            Firmata.sendAnalog(analogPin, Mux.analogReadMS(ANALOG_PORT,analogPin));
          }
        }
      }

}

void reportDigitalCallback(byte port, int value)
{
  if (port < TOTAL_PORTS) {
    reportPINs[port] = (byte)value;

    if (value) outputPort(port, readPort(port, portConfigInputs[port]), true);
  }
}


void setPinModeCallback(byte pin, int mode)
{   
    
    if (Firmata.getPinMode(pin) == PIN_MODE_IGNORE) return;

    if (IS_PIN_DIGITAL(pin)) {
        if (mode == INPUT || mode == PIN_MODE_PULLUP) {
            portConfigInputs[pin / 8] |= (1 << (pin & 7));
        }    
        else {
            portConfigInputs[pin / 8] &= ~(1 << (pin & 7));
        }
    }

    Firmata.setPinState(pin, 0);

    switch (mode) {
        case PIN_MODE_ANALOG:
        if (IS_PIN_ANALOG(pin)) {
            Firmata.setPinMode(pin, PIN_MODE_ANALOG);            
        }
        break;

        case INPUT:
        if (IS_PIN_DIGITAL(pin)) {
            Firmata.setPinMode(pin, INPUT);            
        }
        break;

        case PIN_MODE_PULLUP:
        if (IS_PIN_DIGITAL(pin)) {
            Firmata.setPinMode(pin, PIN_MODE_PULLUP);
            Firmata.setPinState(pin, 1);            
        }
        break;

        case OUTPUT:
        if (IS_PIN_DIGITAL(pin)) {
            Firmata.setPinMode(pin, OUTPUT);            
        }
        break;        

        default:
            Firmata.sendString(pmstr(sStatusModeNONE));        
    }
  
}


void setPinValueCallback(byte pin, int value)
{
 
 if (pin < TOTAL_PINS && IS_PIN_DIGITAL_OUT(pin)) {
    if (Firmata.getPinMode(pin) == OUTPUT) {
        
        Firmata.setPinState(pin, value);

        if      (IS_PIN_DIGITAL_MUX_OUT1(pin)) Mux.digitalWriteMS(1,PIN_TO_MUX_PORT_1(pin), value);
        else if (IS_PIN_DIGITAL_MUX_OUT2(pin)) Mux.digitalWriteMS(2,PIN_TO_MUX_PORT_2(pin), value);
        else if (IS_PIN_DIGITAL_MUX_OUT3(pin)) Mux.digitalWriteMS(3,PIN_TO_MUX_PORT_3(pin), value);
        else if (IS_PIN_DIGITAL_MUX_OUT4(pin)) Mux.digitalWriteMS(4,PIN_TO_MUX_PORT_4(pin), value);
        else if (IS_PIN_DIGITAL_MUX_OUT5(pin)) Mux.digitalWriteMS(5,PIN_TO_MUX_PORT_5(pin), value);
        else if (IS_PIN_DIGITAL_MUX_OUT6(pin)) Mux.digitalWriteMS(6,PIN_TO_MUX_PORT_6(pin), value);
      
    }
  }
}


void digitalWriteCallback(byte port, int value)
{
  byte pin, lastPin, pinValue, mask = 1, pinWriteMask = 0;

  if (port < TOTAL_PORTS) {
    
    lastPin = port * 8 + 8;
    if (lastPin > TOTAL_PINS) lastPin = TOTAL_PINS;
    for (pin = port * 8; pin < lastPin; pin++) {
      
      if (IS_PIN_DIGITAL(pin)) {
        
        if (Firmata.getPinMode(pin) == OUTPUT || Firmata.getPinMode(pin) == INPUT) {
          pinValue = ((byte)value & mask) ? 1 : 0;
          if (Firmata.getPinMode(pin) == OUTPUT) {
            pinWriteMask |= mask;
          } 
          Firmata.setPinState(pin, pinValue);
        }
      }
      mask = mask << 1;
    }
    writePort(port, (byte)value, pinWriteMask);
       
  }
}


void systemResetCallback()
{    
    byte bMuxPort = 0;
    
    isResetting = true;
    
    if (bDebug){
        SerialOut.print(pmstr(s16spaces));
        SerialOut.println(pmstr(sStatusPorts));
        
        SerialOut.print(pmstr(s16spaces));
        SerialOut.print(pmstr(sStatusModes));
    }   

    for (byte bPort = TOTAL_PORTS; bPort >=2 ; bPort-=2) {
        bMuxPort = bPort/2;
        
        switch(Mux.getMode(bMuxPort)){
            case DIGITAL_IN:
                reportPINs[bPort-2] = bPortOn;
                reportPINs[bPort-1] = bPortOn;
                portConfigInputs[bPort-2] = bPortOn;
                portConfigInputs[bPort-1] = bPortOn;
                if (bDebug) SerialOut.print(pmstr(sStatusModeDIN));
                break;
                
            case DIGITAL_OUT:
                reportPINs[bPort-2] = bPortOff;
                reportPINs[bPort-1] = bPortOff;
                portConfigInputs[bPort-2] = bPortOff;
                portConfigInputs[bPort-1] = bPortOff;                
                if (bDebug) SerialOut.print(pmstr(sStatusModeDOUT));
                break;
                
            case ANALOG_IN:
                reportPINs[bPort-2] = bPortOff;
                reportPINs[bPort-1] = bPortOff;
                portConfigInputs[bPort-2] = bPortOn;
                portConfigInputs[bPort-1] = bPortOn;
                if (bDebug) SerialOut.print(pmstr(sStatusModeAIN));
                break;
                
            case DIGITAL_IN_PULLUP:
                reportPINs[bPort-2] = bPortOn;
                reportPINs[bPort-1] = bPortOn;
                portConfigInputs[bPort-2] = bPortOn;
                portConfigInputs[bPort-1] = bPortOn;
                if (bDebug) SerialOut.print(pmstr(sStatusModeDINP));                
                break;
                
            default:
                reportPINs[bPort-2] = bPortOff;
                reportPINs[bPort-1] = bPortOff;
                portConfigInputs[bPort-2] = bPortOff;
                portConfigInputs[bPort-1] = bPortOff;
                if (bDebug) SerialOut.print(pmstr(sStatusModeNONE));
        }      

        previousPINs[bPort-2] = 0;
        previousPINs[bPort-1] = 0;        
    }
    
    if (bDebug) SerialOut.println(pmstr(sStatusDel2));
    
    for (byte i = 0; i < TOTAL_PINS; i++) {
        if (IS_PIN_ANALOG(i)) setPinModeCallback(i, PIN_MODE_ANALOG);
        else if (IS_PIN_DIGITAL_IN_PULLUP(i)) setPinModeCallback(i, PIN_MODE_PULLUP);
        else if (IS_PIN_DIGITAL_OUT(i)) setPinModeCallback(i,OUTPUT);        
        else if (IS_PIN_DIGITAL_IN(i)) setPinModeCallback(i,INPUT);
    }
    
    isResetting = false;  
}


int getFreeRAM()
{
    extern char __bss_end;
    extern char *__brkval;

    int free_memory;

    if((int)__brkval == 0)
        free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
        free_memory = ((int)&free_memory) - ((int)__brkval);

    return free_memory;
}


void beginFirmata()
{
    
    Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
    
    Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
    Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
    Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);    
    //Firmata.attach(SET_PIN_MODE, setPinModeCallback);
    
    Firmata.attach(SET_DIGITAL_PIN_VALUE, setPinValueCallback);
    //Firmata.attach(START_SYSEX, sysexCallback);
    Firmata.attach(SYSTEM_RESET, systemResetCallback);

    Serial.begin(ulRateHardware);    
    while (!Serial);
    
    Firmata.begin(Serial);   
    
    systemResetCallback();
    
}


void beginMuxShields()
{
    byte muxFirstPin = 0;
    for (byte muxPort = 1; muxPort <= PORTS; muxPort++){
        
        muxFirstPin = (muxPort-1) * MUX_PORT_PINS;
        if (IS_PIN_DIGITAL_IN(muxFirstPin)) Mux.setMode(muxPort,DIGITAL_IN);
        else if (IS_PIN_DIGITAL_IN_PULLUP(muxFirstPin)) Mux.setMode(muxPort,DIGITAL_IN_PULLUP);
        else if (IS_PIN_DIGITAL_OUT(muxFirstPin)) Mux.setMode(muxPort,DIGITAL_OUT);
        else if (IS_PIN_ANALOG(muxFirstPin)) Mux.setMode(muxPort,ANALOG_IN);
    }
}


void beginSerialOut()
{
                                                                                    // TODO better handling
    SerialOut.begin((long)ulRateSoftware);
    delay(2000);
    
    SerialOut.write(12);
    SerialOut.write(13);
    
    for (byte col=0; col <= bDebugLen; col++) SerialOut.print(pmstr(sStatusLine));
    SerialOut.println();
    
    SerialOut.println(pmstr(sStatusSerialUp));    
    
    SerialOut.print(pmstr(sStatusRateHardware));
    SerialOut.println(ulRateHardware);
    
    SerialOut.print(pmstr(sStatusRateSoftware));    
    SerialOut.println(ulRateSoftware);
    
    SerialOut.print(pmstr(sStatusASample));
    SerialOut.println(ulSampleRate);
    
    SerialOut.print(pmstr(sStatusDSample));
    SerialOut.println(ulDSampleRate);

    SerialOut.print(pmstr(sStatusSelfTest));
    if (bSelfTest) SerialOut.println(pmstr(sStatusOn)); else SerialOut.println(pmstr(sStatusOff));
    
    for (byte col=0; col <= bDebugLen; col++) SerialOut.print(pmstr(sStatusLine));
    SerialOut.println();
}


void setup()
{        
    if(bDebug) beginSerialOut();
    
    beginMuxShields();
    
    beginFirmata();       
    
}


void loop()
{
    
    if (!(bRunOnce && isOnce)){
        isOnce = true;
    
        
        if (!isUp){        
            while (millis() % 1000 != 0);
            isUp = true;
        }
        
        if (bDebug){            
            ulUptimeC = millis();    
            if (ulUptimeC - ulUptimeP >= ulUptimeRate){
                ulUptimeP = ulUptimeC;
                ulUptimeSecs = ulUptimeC / 1000;                 
                uFreeRAM = getFreeRAM();                           
            }
        }        
        
        if (bSelfTest){
            ulDWriteC = millis();
            if (ulDWriteC - ulDWriteP > ulDWriteRate){
                ulDWriteP = ulDWriteC;
                
                setPinValueCallback(testPrevious, 0);
                setPinValueCallback(testCount, 1);
                
                testPrevious = testCount;
                if (testMode){
                    testCount++;
                    if (testCount >= testPort + MUX_PORT_PINS){
                        testCount = testPort + MUX_PORT_PINS -2;
                        testMode = false;  
                    }
                }               
                    
                else{
                    testCount--;
                    if (testCount <= testPort){
                        testCount = testPort;
                        testMode = true;
                    }
                }                       
                
            }
        }
           
        if (bSampleDigital){
            if (bUseDigitalRate){
                ulDSampleC = millis();
                if (ulDSampleC - ulDSampleP > ulDSampleRate){
                    ulDSampleP = ulDSampleC;
                    checkDigitalInputs();             
                }
            }
            else checkDigitalInputs();                
        }        

        while (Firmata.available()) Firmata.processInput();    

        if (bSampleAnalog){
            ulSampleC = millis();
            if (ulSampleC - ulSampleP > ulSampleRate){
                ulSampleP = ulSampleC;
                
                for (byte pin = 0; pin < TOTAL_ANALOG_PINS; pin++){                    
                    aAnalogRead[pin] = Mux.analogReadMS(ANALOG_PORT,pin);
                    Firmata.sendAnalog(pin, aAnalogRead[pin]);
                    
                    if (aAnalogRead[1] +1 ==512) setPinValueCallback(1,1); else setPinValueCallback(1,0);
                    
                    if (aAnalogRead[3] +1 ==512) setPinValueCallback(3,1); else setPinValueCallback(3,0);
                    
                    if (aAnalogRead[6] +1 ==512) setPinValueCallback(6,1); else setPinValueCallback(6,0);
                    
                    if (aAnalogRead[6] +1 ==512) setPinValueCallback(11,1); else setPinValueCallback(11,0);
                }
            }   
        }
            
    }
}
