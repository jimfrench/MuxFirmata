/*
MuxShields.h - Library for using Mayhew Labs' Mux Shield.
Created by Mark Mayhew, December 20, 2012.
Released into the public domain.
 
Modifications by Jim French, March 2016
 */

#ifndef MuxShields_h
#define MuxShields_h

#define DIGITAL_IN 0
#define DIGITAL_OUT 1
#define ANALOG_IN 2
#define DIGITAL_IN_PULLUP 3

#define PORTS 6                 // set this to <=3 to fallback to 1 shield (valid values are 1,2,3,6)
#define CHANNELS 16             // number of channels per port
#define ERF -1                  // error flag

#define ANALOG_PORT 1


class MuxShield {
    
public:
    MuxShield();
    MuxShield(int S0, int S1, int S2, int S3, int S4, int S5, int S6, int OUTMD, int IOS1, int IOS2, int IOS3, int IO1, int IO2, int IO3, int IO4, int IO5, int IO6);
    
    void setMode(int mux, int mode);
    int getMode(int mux);                       // added to return current mode of port
    
    void digitalWriteMS(int mux, int chan, int val);
    int digitalReadMS(int mux, int chan);
    int analogReadMS(int mux, int chan);    
    
private:
    int _S0, _S1, _S2;
    int _S3;
    int _S4, _S5, _S6;                          // added for SCLK lines to ports 4,5,6
    int _OUTMD;
    int _IOS1, _IOS2, _IOS3;
    int _IO1, _IO2, _IO3;
    int _IO4, _IO5, _IO6;                       // added for I/O ports 4,5,6
        
    void setAddress(int mux, int chan);                  // added to reduce code repetition 
    
};

#endif
