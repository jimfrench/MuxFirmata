/*
  Boards.h - Hardware Abstraction Layer for Firmata library
  For use with Arduino UNO R3 and Mayhew Labs MuxShieldII only
 
  Copyright (c) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2009-2015 Jeff Hoefs.  All rights reserved.
  Copyright (C) 2016 Jim French. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

*/

#ifndef Firmata_Boards_h
#define Firmata_Boards_h

#include <inttypes.h>

#include <Arduino.h>

#define TOTAL_PORTS                     12                      // firmata ports are groups of 8 pins (1 byte for each firmata port = 1 bit per pin)
#define TOTAL_ANALOG_PINS               16                      // 16 is maximum analogue inputs firmata can cope with atm
#define TOTAL_PINS                      96                      // 2 muxshields * 3 ports per shield * 16 pins per port = 96

#define MUX_PORT_PINS                   16                      // number of pins per mux port
#define MUX_PORT_DISABLED               0
                                                                // analogue inputs must be within pin range 0 to 15 (first mux port)
#define MUX_PORT_1                      0                       // mux port # begins at this firmata pin #
#define MUX_PORT_2                      16
#define MUX_PORT_3                      32
#define MUX_PORT_4                      48
#define MUX_PORT_5                      64
#define MUX_PORT_6                      80

#define PIN_TO_MUX_PORT_1(p)            ((p) - MUX_PORT_1)      // return pin# for each mux port
#define PIN_TO_MUX_PORT_2(p)            ((p) - MUX_PORT_2)
#define PIN_TO_MUX_PORT_3(p)            ((p) - MUX_PORT_3)
#define PIN_TO_MUX_PORT_4(p)            ((p) - MUX_PORT_4)
#define PIN_TO_MUX_PORT_5(p)            ((p) - MUX_PORT_5)
#define PIN_TO_MUX_PORT_6(p)            ((p) - MUX_PORT_6)

#define PIN_TO_ANALOG(p)                (p)                     // return pin# of analogue pin
#define PIN_TO_DIGITAL(p)               (p)                     // return pin# of digital pin                                        

                                        // check if pin# is in digital or analogue mode
#define IS_PIN_ANALOG(p)                ((p) >= 0 && (p) < TOTAL_ANALOG_PINS)                       
#define IS_PIN_DIGITAL(p)               ((p) >= TOTAL_ANALOG_PINS && (p) <= TOTAL_PINS)

                                        // check if pin# is within valid range of a mux port during digital input                                                                
#define IS_PIN_DIGITAL_MUX_IN1(p)       MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_IN2(p)       ((p) >= MUX_PORT_2 && (p) < MUX_PORT_2 + MUX_PORT_PINS)
#define IS_PIN_DIGITAL_MUX_IN3(p)       ((p) >= MUX_PORT_3 && (p) < MUX_PORT_3 + MUX_PORT_PINS)
#define IS_PIN_DIGITAL_MUX_IN4(p)       ((p) >= MUX_PORT_4 && (p) < MUX_PORT_4 + MUX_PORT_PINS)
#define IS_PIN_DIGITAL_MUX_IN5(p)       ((p) >= MUX_PORT_5 && (p) < MUX_PORT_5 + MUX_PORT_PINS)
#define IS_PIN_DIGITAL_MUX_IN6(p)       MUX_PORT_DISABLED    

                                        // check if pin# is within valid range of a mux port during digital output
#define IS_PIN_DIGITAL_MUX_OUT1(p)      MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_OUT2(p)      MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_OUT3(p)      MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_OUT4(p)      MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_OUT5(p)      MUX_PORT_DISABLED
#define IS_PIN_DIGITAL_MUX_OUT6(p)      ((p) >= MUX_PORT_6 && (p) < MUX_PORT_6 + MUX_PORT_PINS)

                                        // check if pin# is in digital_in (no pullup) mode
#define IS_PIN_DIGITAL_IN(p)            MUX_PORT_DISABLED                                           

                                        // check if pin# is in digital_in_pullup mode
#define IS_PIN_DIGITAL_IN_PULLUP(p)     (IS_PIN_DIGITAL_MUX_IN1(p) || IS_PIN_DIGITAL_MUX_IN2(p) || IS_PIN_DIGITAL_MUX_IN3(p) || IS_PIN_DIGITAL_MUX_IN4(p) || IS_PIN_DIGITAL_MUX_IN5(p) || IS_PIN_DIGITAL_MUX_IN6(p))

                                        // check if pin# is in digital_out mode
#define IS_PIN_DIGITAL_OUT(p)           (IS_PIN_DIGITAL_MUX_OUT1(p) || IS_PIN_DIGITAL_MUX_OUT2(p) || IS_PIN_DIGITAL_MUX_OUT3(p) || IS_PIN_DIGITAL_MUX_OUT4(p) || IS_PIN_DIGITAL_MUX_OUT5(p) || IS_PIN_DIGITAL_MUX_OUT6(p))

#endif /* Firmata_Boards_h */
