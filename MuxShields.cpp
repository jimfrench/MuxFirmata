/*
MuxShields.cpp - Library for using Mayhew Labs' Mux Shield.
Created by Mark Mayhew, December 29, 2012.
Released into the public domain.
 * 
 * Modifications by Jim French, March 2016
 * 
 * CHANGELOG
 * Added fallback to default configuration of 3 ports
 * Added functions to expand I/O to 6 ports by stacking two MuxShields as shown in schematics
 * Reduced functions as result of above changes, due to running out of Arduino digital I/O pins:
 *      Additional ports 4, 5, 6 on second MuxShield require manual selection of I/O mode in hardware.
 *      Ports 1, 2, 3 continue to have their I/O mode selectable in software as per original design.
 * Added error handling in case of invalid input to member functions
 * Added getMode member function
 * Added setAddress member function


 */

#include <Arduino.h>

#include "MuxShields.h"

int _shiftReg1[16]={0};
int _shiftReg2[16]={0};
int _shiftReg3[16]={0};
int _shiftReg4[16]={0};
int _shiftReg5[16]={0};
int _shiftReg6[16]={0};

int _muxMode[6] = {0};      // added to store current mode of ports


MuxShield::MuxShield(int S0, int S1, int S2, int S3, int S4, int S5, int S6, int OUTMD ,int IOS1, int IOS2, int IOS3, int IO1, int IO2, int IO3, int IO4, int IO5, int IO6)
{
                            // ARDUINO R3 pin definitions, see schematic for details of changes    
    if(PORTS==6){           // Pins have dual function: serial clock during output, address buss during input
                            // _____________________________________________________
                            // During Output:        |  During Input:               |
        _S4 = S4;           // SCLK for port 4       |  ADDR0 for ports 4,5,6       |
        _S5 = S5;           // SCLK for port 5       |  ADDR1 for ports 4,5,6       |
        _S6 = S6;           // SCLK for port 6       |  ADDR2 for ports 4,5,6       |
                            // ______________________|______________________________|
        
        _IO4 = IO4;         // I/O port 4
        _IO5 = IO5;         // I/O port 5
        _IO6 = IO6;         // I/O port 6
        
        pinMode(_S4,OUTPUT);
        pinMode(_S5,OUTPUT);
        pinMode(_S6,OUTPUT);       
    }                   
                            // Pins have dual function: serial clock during output, address buss during input
                            // _____________________________________________________
                            // During Output:        |  During Input:               |
    _S0 = S0;               // SCLK for port 1       |  ADDR0 for port  1,2,3       |
    _S1 = S1;               // SCLK for port 2       |  ADDR1 for port  1,2,3       |
    _S2 = S2;               // SCLK for port 3       |  ADDR2 for port  1,2,3       |
    _S3 = S3;               // LCLK for all 6 ports  |  ADDR3 for ports 1,2,3,4,5,6 |
                            // ______________________|______________________________|
    
    _OUTMD = OUTMD;         // high during output mode to enable shift registers, low during output mode to enable multiplexers
    
    _IOS1 = IOS1;           // I/O mode for port 1
    _IOS2 = IOS2;           // I/O mode for port 2
    _IOS3 = IOS3;           // I/O mode for port 3
                            // (must select mode in hardware for ports 4,5,6)
    
    _IO1 = IO1;             // I/O port 1
    _IO2 = IO2;             // I/O port 2
    _IO3 = IO3;             // I/O port 3
    
    pinMode(_S0,OUTPUT);
    pinMode(_S1,OUTPUT);
    pinMode(_S2,OUTPUT);
    pinMode(_S3,OUTPUT);    
    
    pinMode(_OUTMD,OUTPUT);
    digitalWrite(_OUTMD,LOW);
    
    pinMode(_IOS1,OUTPUT);
    pinMode(_IOS2,OUTPUT);
    pinMode(_IOS3,OUTPUT);
    
}

MuxShield::MuxShield()
{
                                
    if(PORTS==6){
        _S4 = 9;
        _S5 = 3;
        _S6 = 5;
        
        _IO4 = A3;
        _IO5 = A4;
        _IO6 = A5;
        
        pinMode(_S4,OUTPUT);
        pinMode(_S5,OUTPUT);
        pinMode(_S6,OUTPUT);
    }
    
    _S0 = 2;
    _S1 = 4;
    _S2 = 6;
    _S3 = 7;
    
    _OUTMD = 8;
    
    _IOS1 = 10;
    _IOS2 = 11;
    _IOS3 = 12;
    
    _IO1 = A0;
    _IO2 = A1;
    _IO3 = A2;

    
    pinMode(_S0,OUTPUT);
    pinMode(_S1,OUTPUT);
    pinMode(_S2,OUTPUT);
    pinMode(_S3,OUTPUT);    
    
    pinMode(_OUTMD,OUTPUT);
    digitalWrite(_OUTMD,LOW);
    
    pinMode(_IOS1,OUTPUT);
    pinMode(_IOS2,OUTPUT);
    pinMode(_IOS3,OUTPUT);    
    
}


int MuxShield::getMode(int mux)                 // added to return current mode of port
{
    if(mux>=1 && mux <=6) return _muxMode[mux-1]; else return ERF;
}

void MuxShield::setMode(int mux, int mode)      // mode of MUX ports 4,5,6 set in hardware (function must still called to set desired mode of ARD pin)
{
    
    if(mux>=1 && mux<=6) _muxMode[mux-1] = mode;
    
    switch (mux) {        
        case 1:
            switch (mode) {
                case DIGITAL_IN:
                    pinMode(_IO1,INPUT);
                    digitalWrite(_IOS1,LOW);
                    break;
                case DIGITAL_IN_PULLUP:
                    pinMode(_IO1,INPUT_PULLUP);
                    digitalWrite(_IOS1,LOW);                    
                    break;
                case DIGITAL_OUT:
                    pinMode(_IO1,OUTPUT);
                    digitalWrite(_IOS1,HIGH);
                    break;
                case ANALOG_IN:
                    digitalWrite(_IOS1,LOW);
                    break;
                default:
                    break;
            }
            break;
            
        case 2:
            switch (mode) {
                case DIGITAL_IN:
                    pinMode(_IO2,INPUT);
                    digitalWrite(_IOS2,LOW);
                    break;
                case DIGITAL_IN_PULLUP:
                    pinMode(_IO2,INPUT_PULLUP);
                    digitalWrite(_IOS2,LOW);                    
                    break;
                case DIGITAL_OUT:
                    pinMode(_IO2,OUTPUT);
                    digitalWrite(_IOS2,HIGH);
                    break;
                case ANALOG_IN:
                    digitalWrite(_IOS2,LOW);
                    break;
                default:
                    break;
            }
            break;
            
        case 3:
            switch (mode) {
                case DIGITAL_IN:
                    pinMode(_IO3,INPUT);
                    digitalWrite(_IOS3,LOW);
                    break;
                case DIGITAL_IN_PULLUP:
                    pinMode(_IO3,INPUT_PULLUP);
                    digitalWrite(_IOS3,LOW);                    
                    break;
                case DIGITAL_OUT:
                    pinMode(_IO3,OUTPUT);
                    digitalWrite(_IOS3,HIGH);
                    break;
                case ANALOG_IN:
                    digitalWrite(_IOS3,LOW);
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
            
    if(PORTS==6){
        switch (mux){
            case 4:
                switch (mode) {
                    case DIGITAL_IN:
                        pinMode(_IO4,INPUT);
                        break;
                    case DIGITAL_IN_PULLUP:
                        pinMode(_IO4,INPUT_PULLUP);                        
                        break;
                    case DIGITAL_OUT:
                        pinMode(_IO4,OUTPUT);
                        break;
                    default:
                        break;
                }
                break;

            case 5:
                switch (mode) {
                    case DIGITAL_IN:
                        pinMode(_IO5,INPUT);
                        break;
                    case DIGITAL_IN_PULLUP:
                        pinMode(_IO5,INPUT_PULLUP);                        
                        break;
                    case DIGITAL_OUT:
                        pinMode(_IO5,OUTPUT);
                        break;
                    default:
                        break;
                }
                break;

            case 6:
                switch (mode) {
                    case DIGITAL_IN:
                        pinMode(_IO6,INPUT);
                        break;
                    case DIGITAL_IN_PULLUP:
                        pinMode(_IO6,INPUT_PULLUP);                        
                        break;
                    case DIGITAL_OUT:
                        pinMode(_IO6,OUTPUT);
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
}

void MuxShield::digitalWriteMS(int mux, int chan, int val)      // modified to accept writes to ports 4,5,6
{
    int i=16;
    
    if(chan<CHANNELS){
        digitalWrite(_S3,LOW);                              //S3 here is LCLK
        digitalWrite(_OUTMD,HIGH);                          //set to output mode

        switch (mux) {
            case 1:
                _shiftReg1[chan] = val;                     //store value until updated again            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S0,LOW);                  //S0 here is i/o1 _sclk
                    digitalWrite(_IO1,_shiftReg1[i]);       //put value
                    digitalWrite(_S0,HIGH);                 //latch in value
                }
                break;

            case 2:
                _shiftReg2[chan] = val;            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S1,LOW);
                    digitalWrite(_IO2,_shiftReg2[i]);
                    digitalWrite(_S1,HIGH);
                }
                break;

            case 3:
                _shiftReg3[chan] = val;            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S2,LOW);
                    digitalWrite(_IO3,_shiftReg3[i]);
                    digitalWrite(_S2,HIGH);
                }
                break;

            case 4:                                         //added for port 4
                _shiftReg4[chan] = val;            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S4,LOW);
                    digitalWrite(_IO4,_shiftReg4[i]);
                    digitalWrite(_S4,HIGH);
                }
                break;

            case 5:                                         //added for port 5
                _shiftReg5[chan] = val;            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S5,LOW);
                    digitalWrite(_IO5,_shiftReg5[i]);
                    digitalWrite(_S5,HIGH);
                }
                break;

            case 6:                                         //added for port 6
                _shiftReg6[chan] = val;            
                for (i=15; i>=0; i--) {
                    digitalWrite(_S6,LOW);
                    digitalWrite(_IO6,_shiftReg6[i]);
                    digitalWrite(_S6,HIGH);
                }
                break;           

            default:
                break;   
        }

        digitalWrite(_S3,HIGH);                     //latch in ports 1 to 6
        digitalWrite(_OUTMD,LOW);                   //Exit output mode
    }
    
}

void MuxShield::setAddress(int mux, int chan)            // added to reduce code repetition in read functions below
{    
    int i0 = _S0;
    int i1 = _S1;
    int i2 = _S2;
    
    if(PORTS==6 && mux >= 4){
        i0 = _S4;
        i1 = _S5;
        i2 = _S6;
    }
    
    digitalWrite(i0, (chan&1));    
    digitalWrite(i1, (chan&3)>>1); 
    digitalWrite(i2, (chan&7)>>2); 
    digitalWrite(_S3, (chan&15)>>3);
    
}

int MuxShield::digitalReadMS(int mux, int chan)         // modified to accept reads from ports 4,5,6
{
    int val = ERF;                                      // returns -1 if invalid input to function

    if(chan>=0 && chan<CHANNELS){                       // added error handling if invalid input
    
        digitalWrite(_OUTMD,LOW);                       //Set outmode off (i.e. set as input mode)
        setAddress(mux, chan);

        if(PORTS==6){                                 // only read from second shield if enabled
        
            switch (mux) {
                case 4:                    
                    val = digitalRead(_IO4); 
                    break;

                case 5:                    
                    val = digitalRead(_IO5); 
                    break;

                case 6:                    
                    val = digitalRead(_IO6); 
                    break;

                default:
                    break;
            }
        }
        
        switch (mux) {
            case 1:                
                val = digitalRead(_IO1); 
                break;

            case 2:                
                val = digitalRead(_IO2); 
                break;

            case 3:                
                val = digitalRead(_IO3); 
                break;

            default:
                break;
        }
    }
    
    if (val==0) val=1; else if (val==1) val=0;
    
    return val;
}

int MuxShield::analogReadMS(int mux, int chan)
{
    int val = ERF;                                      // returns -1 if invalid input to function
    
    if(chan>=0 && chan<CHANNELS){                       // added error handling if invalid input

        digitalWrite(_OUTMD,LOW);
        setAddress(mux, chan);

        if(PORTS==6){                                 // only read from second shield if enabled
        
            switch (mux) {
                case 4:                    
                    val = analogRead(_IO4); 
                    break;

                case 5:                    
                    val = analogRead(_IO5); 
                    break;

                case 6:                    
                    val = analogRead(_IO6); 
                    break;

                default:
                    break;
            }
        }
        
        switch (mux) {
            case 1:                
                val = analogRead(_IO1); 
                break;

            case 2:                
                val = analogRead(_IO2); 
                break;

            case 3:                
                val = analogRead(_IO3); 
                break;

            default:
                break;
        }
        
    }
    return val;
}