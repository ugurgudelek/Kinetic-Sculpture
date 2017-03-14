
/*
Name:		Master.ino
Created:	11/2/2015 11:13:42 AM
Author:	Ugur Gudelek
Editor:	http://www.visualmicro.com
*/


	



#include <Adafruit_NeoPixel.h>
#include "../Rrs485/Rs485Lib.h"





#define NUM_OF_PIXELS 3
#define RECEIVE_ENABLE_PIN 3

#define STRIP_PIN 9
#define DEBUG_LED_PIN 13




	//Public Settings
#define RUN_ALL 0




Rs485Lib rs(RECEIVE_ENABLE_PIN, false); //default : master
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_OF_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

const uint8_t addressRowPins[5] = { 14, 15, 16, 17, 18 };  //A0, A1, A2, A3, A4


void colorWipe(uint32_t c);

uint8_t rowAddress;
uint8_t columnAddress = 0;

unsigned long tic = 0;
uint8_t howManyBlink = 10;


void setup()
{
	//start serial with 57600 BaudRate
	Serial.begin(57600);

	strip.begin();
	strip.show();

	for (int i = 0; i < 5; i++)
	{
		pinMode(addressRowPins[i], INPUT_PULLUP);
		rowAddress = rowAddress + (!digitalRead(addressRowPins[i]) << i);
	}
	
	rs.enableOutputs();
	rowAddress = 1;
	rs.setAddress(rowAddress, columnAddress);
	
	pinMode(DEBUG_LED_PIN, OUTPUT);


	delay(300); //timer0

	tic = millis();

}

// the loop function runs over and over again until power down or reset
void loop()
{
	Serial.println("loop");
	if (rs.receive() == 2) //Gen has come
	{
		rs.send();
		delay(100);

		Serial.print("whole package begin: enable: ");
		Serial.println(digitalRead(RECEIVE_ENABLE_PIN));
		for (int i = 0; i < 108; i++)
		{
			Serial.print(rs.getMasterDataPacket()[i]);
			
				if((i+1)%6 == 0)
				{
					Serial.println();
				}

		}
		Serial.print("whole package end: enable: ");
		Serial.println(digitalRead(RECEIVE_ENABLE_PIN));


		
		howManyBlink = 10;
	}
	if (millis() > tic + 100)
	{
		if (howManyBlink != 0)
		{
			digitalWrite(DEBUG_LED_PIN, !digitalRead(DEBUG_LED_PIN));
			howManyBlink--;
		}
		
		tic = millis();
	}


}





