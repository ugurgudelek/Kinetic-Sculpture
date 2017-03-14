
/*
Name:		Rs485.ino
Created:	11/2/2015 11:13:42 AM
Author:	Ugur Gudelek
Editor:	http://www.visualmicro.com
*/

#include "Rs485Lib.h"

#define ledPin 13
#define ledPin2	8
#define buttonPin 7


//SENDER
#if sender

byte add[2] = { 40, 41 };
byte led[3] = { 43, 44, 45 };

Rs485Lib rs(add, 42, led , 42, 132);

#endif

//RECEIVER
#if !sender

uint8_t add[2] = { 0, 30 };

//Rs485Lib rs(add);

#endif

void setup()
{
	Serial.begin(9600);

	pinMode(ledPin, OUTPUT);
	pinMode(ledPin2, OUTPUT);
	pinMode(buttonPin, INPUT_PULLUP);

	digitalWrite(ledPin, HIGH);
	digitalWrite(ledPin2, HIGH);

}

// the loop function runs over and over again until power down or reset
void loop()
{
	
}

