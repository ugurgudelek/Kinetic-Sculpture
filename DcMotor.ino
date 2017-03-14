/*
Name:		DcMotor.ino
Created:	11/3/2015 3:21:02 PM
Author:	Ugur Gudelek
Editor:	http://www.visualmicro.com
*/

#include "DcMotorLib.h"

DcMotorLib dc(3, 4, 2, 11, 12);

void setup()
{

	dc.enableOutputs();

	Serial.begin(115200);
	Serial.println("---------------------------------------");

	dc.moveTo(-24);
	
	//dc.debugSetup();
}

void loop()
{

	

	if (dc.distanceToGo() == 0)
	{
		Serial.println("New target has been set!");
		//dc.moveTo(-dc.currentPosition());
		dc.move(-24);
		delay(1000);
	}

		Serial.print(dc.speed());			Serial.print("\t");
		Serial.print(dc.currentPosition()); Serial.print("\t");
		Serial.print(dc.targetPosition());	Serial.print("\t");
		Serial.print(dc.distanceToGo());	Serial.print("\t");
		Serial.print(dc.encoderCounter());	Serial.print("\t");
		Serial.println();

	dc.run();
}
