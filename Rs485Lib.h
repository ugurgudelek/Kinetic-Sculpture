/*
 Name:		Rs485Lib.h
 Created:	11/2/2015 11:13:42 AM
 Author:	Ugur Gudelek
 Editor:	http://www.visualmicro.com
*/

#ifndef _Rs485Lib_h
#define _Rs485Lib_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif



#define MASTER_PACKET_SIZE 108 //6*18
#define SLAVE_PACKET_SIZE 6
#define GENERAL_ADDRESS_0 30
#define GENERAL_ADDRESS_1 30
#define COLUMN_NUMBER 18
#define ROW_NUMBER 25

class Rs485Lib{
public:


	//Class Constructors
	Rs485Lib(uint8_t receiveEnablePin, bool imSlave); //Slave or Master


	
	//Class Functions
	uint8_t getMotorPos();
	uint8_t getG();
	uint8_t getR();
	uint8_t getB();
	uint8_t* getMasterDataPacket();
	uint8_t* getSlaveDataPacket();

	void setMotorPos(uint8_t motorPos);
	void setG(uint8_t g);
	void setR(uint8_t r);
	void setB(uint8_t b);

	void setAddress(uint8_t rowAddress, uint8_t columnAddress);


	void send();
	uint8_t receive();

	void debug();

	uint8_t* getOptions();

	void setMeMaster();

	void setMeSlave();

	void enableOutputs();

	


	
private:
	void setPacketForSend(uint8_t rowAddress, uint8_t columnAddress, uint8_t motorPos, uint8_t G, uint8_t R, uint8_t B);

	uint8_t _receiveEnablePin;
	bool _imSlave;


	uint8_t _masterDataPacket[MASTER_PACKET_SIZE];
	uint8_t _slaveDataPacket[SLAVE_PACKET_SIZE];
	uint8_t _rowAddress;
	uint8_t _columnAddress;
	uint8_t _motorPos;
	uint8_t _G;
	uint8_t _R;
	uint8_t _B;

	uint8_t _options[4];




};


#endif

