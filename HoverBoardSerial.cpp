#include "HoverBoardSerial.h"

#define DEBUG 1
#define DEBUG_RX 1
#define DEBUG_TX 0


HoverBoardSerial::HoverBoardSerial(void)
{
    _hoverBoardSerialPort = &Serial;
    _debugSerialPort = &Serial;
}

HoverBoardSerial::HoverBoardSerial(Stream *hoverBoardSerialPort)
{
    _hoverBoardSerialPort = hoverBoardSerialPort;
}

HoverBoardSerial::HoverBoardSerial(Stream *hoverBoardSerialPort, Stream *debugSerialPort)
{
    _hoverBoardSerialPort = hoverBoardSerialPort;
    _debugSerialPort = debugSerialPort;
}

void HoverBoardSerial::setHoverboardSerialPort(Stream *hoverBoardSerialPort)
{
    _hoverBoardSerialPort = hoverBoardSerialPort;
}

void HoverBoardSerial::setDebugSerialPort(Stream *debugSerialPort)
{
    _debugSerialPort = debugSerialPort;
}

void HoverBoardSerial::setSpeed(int16_t speed)
{
	_command.speed    = (int16_t)speed;
	_command.checksum = (uint16_t)(_command.start ^ _command.steer ^ _command.speed);
}

void HoverBoardSerial::setSteer(int16_t steer)
{
	_command.steer    = (int16_t)steer;
	_command.checksum = (uint16_t)(_command.start ^ _command.steer ^ _command.speed);
}


void HoverBoardSerial::Send(int16_t speed, int16_t steer)
{
  setSpeed((int16_t) speed);
  setSteer((int16_t) steer);
  
  Send();
}

void HoverBoardSerial::Send(void)
{
    _hoverBoardSerialPort->write((uint8_t *) &_command, sizeof(_command));
#if DEBUG_TX
	_debugSerialPort->print("0b");
	_debugSerialPort->print(_command.start, BIN);
	_debugSerialPort->print(" 0b");
	_debugSerialPort->print(_command.speed, BIN);
	_debugSerialPort->print(" 0b");
	_debugSerialPort->print(_command.steer, BIN);
	_debugSerialPort->print(" 0b");
	_debugSerialPort->println(_command.checksum, BIN);
#endif
}

bool HoverBoardSerial::Receive(void)
{
	// Check for new data availability in the Serial buffer
	if (_hoverBoardSerialPort->available()) {
		_incomingByte 	  = _hoverBoardSerialPort->read();	// Read the incoming byte
		_bufStartFrame	= ((uint16_t)(_incomingByte) << 8) | _incomingBytePrev;	  // Construct the start frame		
	}
	else {
		return false;
	}

#if DEBUG_RX
    _debugSerialPort->print(_incomingByte);
#endif
	
	// Copy received data
	if (_bufStartFrame == _frameStart) {	                    // Initialize if new data is detected
		_p 		= (byte *)&_NewFeedback;
        *_p++   = _incomingBytePrev;
		*_p++ 	= _incomingByte;
		_idx 	= 2;	
	} else if (_idx >= 2 && _idx < sizeof(SerialFeedback)) {	// Save the new received data
		*_p++ 	= _incomingByte; 
		_idx++;
	}	
	
	// Check if we reached the end of the package
	if (_idx == sizeof(SerialFeedback)) {  	
		uint16_t checksum;
		checksum = (uint16_t)(_NewFeedback.start ^ _NewFeedback.cmd1 ^ _NewFeedback.cmd2 ^ _NewFeedback.speedR_meas ^ _NewFeedback.speedL_meas
					^ _NewFeedback.batVoltage ^ _NewFeedback.boardTemp ^ _NewFeedback.cmdLed);
	
		// Check validity of the new data
		if (_NewFeedback.start == _frameStart && checksum == _NewFeedback.checksum) {
			// Copy the new data
			memcpy(&Feedback, &_NewFeedback, sizeof(SerialFeedback));

#if DEBUG
			// Print data to built-in Serial
			_debugSerialPort->print("1: ");   _debugSerialPort->print(Feedback.cmd1);
			_debugSerialPort->print(" 2: ");  _debugSerialPort->print(Feedback.cmd2);
			_debugSerialPort->print(" 3: ");  _debugSerialPort->print(Feedback.speedR_meas);
			_debugSerialPort->print(" 4: ");  _debugSerialPort->print(Feedback.speedL_meas);
			_debugSerialPort->print(" 5: ");  _debugSerialPort->print(Feedback.batVoltage);
			_debugSerialPort->print(" 6: ");  _debugSerialPort->print(Feedback.boardTemp);
			_debugSerialPort->print(" 7: ");  _debugSerialPort->println(Feedback.cmdLed);
#endif

		} else {
#if DEBUG
		  _debugSerialPort->println("Non-valid data skipped");
#endif
		}
		_idx = 0;	// Reset the index (it prevents to enter in this if condition in the next cycle)
	}
 	
	// Update previous states
	_incomingBytePrev 	= _incomingByte;

    return true;
}