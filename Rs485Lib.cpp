/*
 Name:		Rs485Lib.cpp
 Created:	11/2/2015 11:13:42 AM
 Author:	Ugur Gudelek
 Editor:	http://www.visualmicro.com
*/
#include "Rs485Lib.h"



//		CONTRUCTORS
Rs485Lib::Rs485Lib(uint8_t receiveEnablePin, bool imSlave)
{

	_options[0] = 0;
	_options[1] = 0;
	_options[2] = 0;
	_options[3] = 0;

	

	_receiveEnablePin = receiveEnablePin;
	_imSlave = imSlave;
	
	_motorPos = 0;
	_G = 0;
	_B = 0;
	_R = 0;

}


//		****SLAVE****

//		PUBLIC FUNCTIONS


void Rs485Lib::setAddress(uint8_t rowAddress, uint8_t columnAddress)
{
	_rowAddress = rowAddress;
	_columnAddress = columnAddress;
}


uint8_t Rs485Lib::receive()
{
	
	while(Serial.available() > 0)
	{
		Serial.readBytes(_slaveDataPacket, SLAVE_PACKET_SIZE);
		if(_imSlave)//Slave
		{
			if ((_slaveDataPacket[0] == _rowAddress) && (_slaveDataPacket[1] == _columnAddress)) 
			{
				_motorPos = _slaveDataPacket[2];
				_G = _slaveDataPacket[3];
				_R = _slaveDataPacket[4];
				_B = _slaveDataPacket[5];
				return 1;
			}
			
		}

		else // Master
		{
			if (_slaveDataPacket[0] == _rowAddress) 
			{

				_masterDataPacket[0 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[0];
				_masterDataPacket[1 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[1];
				_masterDataPacket[2 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[2];
				_masterDataPacket[3 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[3];
				_masterDataPacket[4 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[4];
				_masterDataPacket[5 + (_slaveDataPacket[1] - 1)*SLAVE_PACKET_SIZE] = _slaveDataPacket[5];

					return 3;
			}
			

		}

		if ((_slaveDataPacket[0] == GENERAL_ADDRESS_0) && (_slaveDataPacket[1] == GENERAL_ADDRESS_1)) //General
		{
			//general things to do

			_options[0] = _slaveDataPacket[2];
			_options[1] = _slaveDataPacket[3];
			_options[2] = _slaveDataPacket[4];
			_options[3] = _slaveDataPacket[5];
			return 2;
		}
		
	}
	

	return 0;
}

uint8_t Rs485Lib::getMotorPos()
{
	return _motorPos;
}
uint8_t Rs485Lib::getG()
{
	return _G;
}
uint8_t Rs485Lib::getR()
{
	return _R;
}
uint8_t Rs485Lib::getB()
{
	return _B;
}
uint8_t* Rs485Lib::getOptions()
{
	return _options;
}

void Rs485Lib::setMotorPos(uint8_t motorPos)
{
	_motorPos = motorPos;
}
void Rs485Lib::setG(uint8_t g)
{
	_G = g;
}
void Rs485Lib::setR(uint8_t r)
{
	_R = r;
}
void Rs485Lib::setB(uint8_t b)
{
	_B = b;
}


//		*****MASTER*****

//		PUBLIC FUNCTIONS

void Rs485Lib::send()
{
	setMeMaster();
	delay(10);
	for (int i = 1; i <= COLUMN_NUMBER; i++)
	{
		setPacketForSend(_rowAddress, i, 
			_masterDataPacket[2 + (i - 1) * SLAVE_PACKET_SIZE], 
			_masterDataPacket[3 + (i - 1) * SLAVE_PACKET_SIZE],
			_masterDataPacket[4 + (i - 1) * SLAVE_PACKET_SIZE],
			_masterDataPacket[5 + (i - 1) * SLAVE_PACKET_SIZE]);
		
			Serial.write(_slaveDataPacket,SLAVE_PACKET_SIZE);
	}
	setPacketForSend(30, 30, 30, 30, 0, 0);
	Serial.write(_slaveDataPacket, SLAVE_PACKET_SIZE);

	delay(10);
	setMeSlave();

}

void Rs485Lib::setPacketForSend(uint8_t rowAddress, uint8_t columnAddress, uint8_t motorPos, uint8_t G, uint8_t R, uint8_t B)
{
	_slaveDataPacket[0] = rowAddress;
	_slaveDataPacket[1] = columnAddress;
	_slaveDataPacket[2] = motorPos;
	_slaveDataPacket[3] = G;
	_slaveDataPacket[4] = R;
	_slaveDataPacket[5] = B;
}

uint8_t* Rs485Lib::getMasterDataPacket()
{
	return _masterDataPacket;
}

uint8_t* Rs485Lib::getSlaveDataPacket()
{
	return _slaveDataPacket;
}


void Rs485Lib::setMeMaster()
{
	digitalWrite(_receiveEnablePin, HIGH);
}

void Rs485Lib::setMeSlave()
{
	digitalWrite(_receiveEnablePin, LOW);
}

void Rs485Lib::enableOutputs()
{
	pinMode(_receiveEnablePin, OUTPUT);
	digitalWrite(_receiveEnablePin, LOW);

}