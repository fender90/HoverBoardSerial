#ifndef HOVERBOARDSERIAL_H_
#define HOVERBOARDSERIAL_H_

#include "Arduino.h"

class HoverBoardSerial {
public:
    const int hoverBaud = 38400;

    HoverBoardSerial(void);
    HoverBoardSerial(Stream *hoverBoardSerialPort);
    HoverBoardSerial(Stream *hoverBoardSerialPort, Stream *debugSerialPort);

    void setHoverboardSerialPort(Stream *hoverBoardSerialPort);
    void setDebugSerialPort(Stream *debugSerialPort);
 
    typedef struct{
        uint16_t	start = 0xABCD;
        int16_t     steer = 0;
        int16_t     speed = 0;
        uint16_t    checksum = 0;
    } SerialCommand;

    typedef struct{
        uint16_t    start;
        int16_t 	cmd1;
        int16_t 	cmd2;
        int16_t 	speedR_meas;
        int16_t 	speedL_meas;
        int16_t 	batVoltage;
        int16_t 	boardTemp;
        uint16_t    cmdLed;
        uint16_t    checksum;
    } SerialFeedback;

    SerialFeedback Feedback;

    bool Receive(void);

    void setSpeed(int16_t speed);
    void setSteer(int16_t steer);

    void Send(void);
    void Send(int16_t speed, int16_t steer);

private:
    Stream *_hoverBoardSerialPort;
    Stream *_debugSerialPort;

    SerialFeedback _NewFeedback;
    SerialCommand _command;
    uint8_t _idx = 0;                        // Index for new data pointer
    uint16_t _bufStartFrame;                 // Buffer Start Frame
    byte *_p;                                // Pointer declaration for the new received data
    byte _incomingByte;
    byte _incomingBytePrev;
    const uint16_t _frameStart = 0xABCD;
};

#endif