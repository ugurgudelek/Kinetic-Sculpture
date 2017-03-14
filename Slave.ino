
/*
Name:		Slave.ino
Created:	11/2/2015 11:13:42 AM
Author:	Ugur Gudelek
Editor:	http://www.visualmicro.com
*/

#include <Adafruit_NeoPixel.h>
#include "../DcMotor/DcMotorLib.h"
#include "../Rrs485/Rs485Lib.h"


#define IM_THERE_INTERVAL 1000
#define IM_NOT_THERE_INTERVAL 500
#define NEW_DATA_INTERVAL 250
#define PLEASE_STOP_INTERVAL 100

#define NUM_OF_PIXELS 3

#define END_SWITCH_PIN 12
#define STRIP_PIN 9
#define CW_PIN	6
#define CCW_PIN	5
#define ENCODER_PIN	2
#define	MOTOR_ENABLE_PIN	7
#define DEBUG_LED_PIN 13
#define RECEIVE_ENABLE_PIN 3


//Private Settings
#define RUN						0					//30:30:row:column:0:NaN
#define RUN_TILL_THE_ENDSWITCH	1					//30:30:row:column:1:NaN
#define CHANGE_MIN_SPEED_UP		2					//30:30:row:column:2:value(0..255)
#define CHANGE_MIN_SPEED_DOWN	3					//30:30:row:column:3:value(0..255)
#define CHANGE_MAX_SPEED		4					//30:30:row:column:4:value(0..255)
#define CHANGE_STALL_SPEED		5					//30:30:row:column:5:value(0..255)
#define CHANGE_GO_BASE_SPEED	6					//30:30:row:column:6:value(0..255)
#define CHANGE_ACCEL_STEP		7					//30:30:row:column:7:value(0..255)
#define CHANGE_ACCEL_MINTOMAX	8					//30:30:row:column:8:value(0..255)
#define CHANGE_END_SWITCH_RESPOND_TIME		9		//30:30:row:column:9:value(0..255)
#define CHANGE_MAX_POS						10		//30:30:row:column:10:value(0..255)
#define CHANGE_CURRENT_POS					11		//30:30:row:column:11:value(0..255)
#define CHANGE_QUICK_SLOWDOWN_DISTANCE_DOWN	12		//30:30:row:column:12:value(0..255)
#define CHANGE_QUICK_SLOWDOWN_DISTANCE_UP	13		//30:30:row:column:13:value(0..255)
#define CHANGE_QUICK_SLOWDOWN_SPEED_DOWN	14		//30:30:row:column:14:value(0..255)
#define CHANGE_MIN_SPEED_AFTER_END_SWITCH	15		//30:30:row:column:15:value(0..255)
#define CHANGE_POS_ACCEPTANCE_MODE			16		//30:30:row:column:16:value(0..255)
#define CHANGE_HOLD_SPEED					17		//30:30:row:column:17:value(0..255)

#define MAKE_ENDSWITCH_ENABLE_OR_DISABLE	18		//30:30:row:column:18:value(0||1)
#define MAKE_ME_ENABLE_OR_DISABLE	19				//30:30:row:column:19:value(0||1)

#define CHANGE_SLIP_TIME		21
#define CHANGE_SLIP_DISTANCE	22
#define SLIP_FUNCTION_ENABLE_OR_DISABLE 23
#define DEBUG_ENABLE_OR_DISABLE 24
#define GIVE_ME_DATA 25
//Public
#define RUN						0		//30:30:row:column:0:NaN
#define RUN_TILL_THE_ENDSWITCH	1		//30:30:row:column:0:NaN
#define GIVE_ME_ROW_ADDRESS		20		//30:30:row:column:20:NaN


//GiveMeData
#define RETURN_PROBLEM_CODE	1    //30:30:row:column:25:1
#define RETURN_POS			2
#define RETURN_COLOR		3

#define PROBLEM_ON_START 1
#define PROBLEM_USER_DEFINED 2
#define PROBLEM_RUN_TILL_THE_END_SWITCH 3
#define PROBLEM_END_SWITCH_PUSHED 4
#define PROBLEM_ENCODER_MISSED 5



#define TIMER0_ADJUST (unsigned long)64


const uint8_t addressRowPins[5] = { 14, 15, 16, 17, 18 };  //A0, A1, A2, A3, A4
const uint8_t addressColumnPins[5] = { 19, 4, 8, 10, 11 };		// A5, 4,8,10,11
uint8_t rowAddress = 0; //address[0] : column     address[1]: row
uint8_t columnAddress = 0;

Rs485Lib rs = Rs485Lib(RECEIVE_ENABLE_PIN, true);
DcMotorLib dc = DcMotorLib(CW_PIN, CCW_PIN, ENCODER_PIN, MOTOR_ENABLE_PIN, END_SWITCH_PIN);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_OF_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

//typedef enum ColorMode
//{
//	Direct,
//	Interpolated,
//};
//uint8_t colorMode = Interpolated;
//unsigned long colorInterpolationPeriod = 10; //10ms
//uint32_t oldColor = strip.Color(0, 0, 0);

int motorPos = 0;
int G = 0;
int R = 0;
int B = 0;



uint8_t option[4] = {0,0,0,0};



// imThereMode : motor went where we want and waiting for new coordinates
// newDataHasArrivedMode: incoming data has been confirmed
// stopPleaseMode : endSwitch showed up and need to stop

void colorWipe();
void modeSelector(long _distanceToGo, uint8_t _dataReceived, int _endSwitchNoticed);
void processFlags(bool _imThere, bool _newDataHasArrived, bool _stopPlease, bool _generalDataHasArrived);
void debugLedBlink(uint8_t _modes);
void whetherBlinkOrNot(unsigned long _period);
void optionControl();
void giveMeData();


bool runAllFlag = false;
unsigned long tic = 0;
double imThereTimer = -1;

 double enoughTic = -1;

 bool debugOn = true;
 bool posCalibrated = false;
 uint8_t posCalibrationCounter = 0;

 

void setup()
{
	//delay(1500); //timer0

	//start serial with 57600 BaudRate
	Serial.begin(57600);

	//get addressRowPins
	for (int i = 0; i < 5; i++)
	{
		pinMode(addressRowPins[i], INPUT_PULLUP);
		rowAddress = rowAddress + (!digitalRead(addressRowPins[i])<<i);
		
	}

	//get addressColumnPins
	for (int i = 0; i < 5; i++)
	{
		pinMode(addressColumnPins[i], INPUT_PULLUP);
		columnAddress = columnAddress + (!digitalRead(addressColumnPins[i])<<i);
	}

	//now address is set and we can continue
	rowAddress = 6;
	columnAddress = 4;
	rs.setAddress(rowAddress,columnAddress);


	//all strips are off
	strip.begin();
	strip.show();

	//Serial.println(rowAddress);
	//Serial.println(columnAddress);

	// DcMotor default settings
	dc.enableOutputs();
	//attachInterrupt(0, isr, FALLING);


	//Pwm 5 - 6 frequency setting
	//Setting 	Divisor 	Frequency
	//  0x01 	 	1 	 	62500
	//  0x02  		8 	 	7812.5
	//  0x03  		64 	 	976.5625   <--DEFAULT
	//	0x04 	 	256 	244.140625
	//	0x05 	 	1024 	61.03515625
	TCCR0B = (TCCR0B & 0b11111000) | 0x01; //62,5kHz

	pinMode(DEBUG_LED_PIN, OUTPUT);

	rs.enableOutputs();

	//dc.isEnabled = true;
	//dc.runTillEndSwitch();


	
	delay(300 * TIMER0_ADJUST); //timer0

	
	
	tic = millis();

}

// the loop function runs over and over again until power down or reset
void loop()
{
	uint8_t receive = rs.receive();

	//method for debug led and colorStrip
	modeSelector(dc.distanceToGo(), receive, !digitalRead(END_SWITCH_PIN));

	// With run function, dc motor runs with respect to distanceToGo().
	
		//Serial.print("speed: "); Serial.println(dc.speed());

	if(runAllFlag && dc.isEnabled)
	{
		dc.run();
		colorWipe();
		
		if(enoughTic == -1)
		{
			enoughTic = millis();
		}
		// 5000 saniye icerisinde hala konumuna gidememiþse gitmiþ varsayalým
		else if(millis() > enoughTic + (double)5000*TIMER0_ADJUST)
		{
			Serial.println(millis() - enoughTic);
			dc.setTargetPosition(dc.currentPosition());
			enoughTic = -1;
		}
	}

	if(dc.distanceToGo() == 0)
	{
		enoughTic = -1;
	}


}




// Fill the dots one after the other with a color
void colorWipe() 
{
	//if(colorMode == Direct)
	//{
		uint32_t c = strip.Color(R, G, B);
		for (uint16_t i = 0; i<strip.numPixels(); i++)
		{
			strip.setPixelColor(i, c);
			strip.show();
		}
	//}

	//else if(colorMode == Interpolated)
	//{
	//	uint8_t rDiff = R - oldR;
	//	unsigned long rCycle = colorInterpolationPeriod / rDiff; // 0 --> 200 : 100ms*64 / 200

	//	uint8_t rAmount = 
	//	double changeCycle = colorInterpolationPeriod / colorDiff;
	//	double changeAmount = colorInterpolationPeriod / colorDiff;
	//	if(millis()%(unsigned long)(changeCycle*TIMER0_ADJUST) == 0)
	//	{
	//		for (uint16_t i = 0; i<strip.numPixels(); i++)
	//		{
	//			strip.setPixelColor(i, c + changeCycle);
	//			strip.show();
	//		}
	//	}
	//}

}

void modeSelector(long _distanceToGo, uint8_t _dataReceived, int _endSwitchNoticed)
{
	bool imThereFlag = false;
	bool newDataHasArrivedFlag = false;
	bool stopPleaseFlag = false;
	bool generalDataHasArrivedFlag = false;

	

	if(_distanceToGo == 0)
	{
		imThereFlag = true;
	}
	
	if(_endSwitchNoticed == 1)
	{
		stopPleaseFlag = true;
	}

	if (_dataReceived == 1)
	{
		newDataHasArrivedFlag = true;
	}
	if (_dataReceived == 2)
	{
		newDataHasArrivedFlag = true;
		generalDataHasArrivedFlag = true;
	}
	

	processFlags(imThereFlag, newDataHasArrivedFlag, stopPleaseFlag, generalDataHasArrivedFlag);

}

void processFlags(bool _imThere, bool _newDataHasArrived, bool _stopPlease, bool _generalDataHasArrived)
{
	uint8_t modes = 0;

	if(_imThere)
	{
		if(runAllFlag == true && imThereTimer == -1)
		{
			imThereTimer = millis();
		}
		else if(runAllFlag == true && millis() > imThereTimer + 100*TIMER0_ADJUST)
		{
			runAllFlag = false;
			modes += 1;
			imThereTimer = -1;
		}
		else if(!runAllFlag)
		{
			modes += 1;
		}
	}
	else
	{
		modes += 2;
	}


	if(_newDataHasArrived)
	{
		if(_generalDataHasArrived)
		{
			optionControl();
		}
		else
		{
			motorPos = rs.getMotorPos();
			G = rs.getG();
			R = rs.getR();
			B = rs.getB();
			dc.moveTo(motorPos);
		}
		imThereTimer = -1;
		modes += 4;
	}

	if (_stopPlease)
	{
		//if(dc.isEndSwitchEnabled && dc.isEnabled)
		//{
		//	modes += 8;

		//	dc.posCalibration();
		//	delay(100*TIMER0_ADJUST);
		//	bool endSwitchChecked = !digitalRead(END_SWITCH_PIN);
		//	if(endSwitchChecked)
		//	{
		//		dc.runEndlesslyWithoutAcceleration(dc.goBaseSpeed(), dc.DIRECTION_CCW);
		//		delay(1000*TIMER0_ADJUST);
		//		dc.stall();
		//		posCalibrationCounter++;
		//	}
		//	else
		//	{
		//		posCalibrationCounter = 0;
		//	}


		//	if(posCalibrationCounter == 3)
		//	{
		//		
		//		//End_Switch_Pushed

		//		dc.problemCode = PROBLEM_END_SWITCH_PUSHED ;
		//		dc.runEndlesslyWithoutAcceleration(0, dc.DIRECTION_CCW);
		//		dc.isEnabled = false;
		//	}
		//}

		if (dc.isEndSwitchEnabled && dc.isEnabled)
		{
			modes += 8;

			dc.posCalibration();
			delay(100 * TIMER0_ADJUST);
			bool endSwitchChecked = !digitalRead(END_SWITCH_PIN);
			if (endSwitchChecked)
			{
				dc.runEndlesslyWithoutAcceleration(dc.goBaseSpeed(), dc.DIRECTION_CCW);
				delay(1000 * TIMER0_ADJUST);
				dc.stall();
				posCalibrationCounter++;
			}
			else
			{
				posCalibrationCounter = 0;
			}


			if (posCalibrationCounter == 3)
			{

				//End_Switch_Pushed
				dc.problemCode = PROBLEM_END_SWITCH_PUSHED;
				dc.runEndlesslyWithoutAcceleration(0, dc.DIRECTION_CCW);
				dc.isEnabled = false;
			}
		}
	}
	

	debugLedBlink(modes);
}

void debugLedBlink(uint8_t _modes)
{
	// 0 : NA
	// 1 : imThere
	// 2,3 : imNotThere
	// 4,5,6,7 : newData
	// 8,..,15 : stopPlease

	switch(_modes)   
	{
		case 1:
		{
			whetherBlinkOrNot(IM_THERE_INTERVAL*TIMER0_ADJUST);
			break;
		}
		case 2:
		case 3:
		{
			whetherBlinkOrNot(IM_NOT_THERE_INTERVAL*TIMER0_ADJUST);
			break;
		}
		case 4:
		case 5:
		case 6:
		case 7:
		{
			whetherBlinkOrNot(NEW_DATA_INTERVAL*TIMER0_ADJUST);
			break;
		}
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		{
			whetherBlinkOrNot(PLEASE_STOP_INTERVAL*TIMER0_ADJUST);
			break;
		}
	}
}

void whetherBlinkOrNot(unsigned long _period)
{
	if(dc.isEnabled)
	{
		if(debugOn)
		{
			if (millis() > _period + tic)
			{
				digitalWrite(DEBUG_LED_PIN, !digitalRead(DEBUG_LED_PIN));
				tic = millis();
			}
		}

		else
		{
			digitalWrite(DEBUG_LED_PIN, LOW);
		}


	}
	else
	{
		digitalWrite(DEBUG_LED_PIN, HIGH);
	}
	
	
}

void optionControl()
{

	option[0] = rs.getOptions()[0];
	option[1] = rs.getOptions()[1];
	option[2] = rs.getOptions()[2];
	option[3] = rs.getOptions()[3];

	//PRIVATE SETTINGS
	if (option[0] == rowAddress && option[1] == columnAddress)
	{
		switch (option[2]) //check option2 which is selectInput
		{
		case RUN:
		{
			if (dc.isEnabled)
			{
				runAllFlag = true;
			}
			break;
		}
		case RUN_TILL_THE_ENDSWITCH:
		{
			if (dc.isEnabled)
			{
				if (!dc.runTillEndSwitch())
				{
					digitalWrite(DEBUG_LED_PIN, HIGH);
				}
			}


			break;
		}
		case CHANGE_MIN_SPEED_UP:
		{
			dc.setMinSpeedUp(option[3]);
			break;
		}
		case CHANGE_MIN_SPEED_DOWN:
		{
			dc.setMinSpeedDown(option[3]);
			break;
		}
		case CHANGE_MAX_SPEED:
		{
			dc.setMaxSpeed(option[3]);
			break;
		}
		case CHANGE_STALL_SPEED:
		{
			dc.setStallSpeed(option[3]);
			break;
		}
		case CHANGE_GO_BASE_SPEED:
		{
			dc.setGoBaseSpeed(option[3]);
			break;
		}
		case CHANGE_ACCEL_STEP:
		{
			dc.setAccelerationStep(option[3]);
			break;
		}
		case CHANGE_ACCEL_MINTOMAX:
		{
			dc.setAccelerationMinToMaxTime(option[3] * 10);
			break;
		}
		case CHANGE_END_SWITCH_RESPOND_TIME:
		{
			dc.setEndSwithRespondTime(option[3] * 50);
			break;
		}
		case CHANGE_MAX_POS:
		{
			dc.setMaxPos(option[3]);
			break;
		}
		case CHANGE_CURRENT_POS:
		{
			dc.setCurrentPosition(option[3]);
			break;
		}
		case CHANGE_QUICK_SLOWDOWN_DISTANCE_DOWN:
		{
			dc.setQuickSlowDownDistanceDown(option[3]);
			break;
		}
		case CHANGE_QUICK_SLOWDOWN_DISTANCE_UP:
		{
			dc.setQuickSlowDownDistanceUp(option[3]);
			break;
		}
		case CHANGE_QUICK_SLOWDOWN_SPEED_DOWN:
		{
			dc.setQuickSlowDownSpeedDown(option[3]);
			break;
		}
		case CHANGE_MIN_SPEED_AFTER_END_SWITCH:
		{
			dc.setMinSpeedAfterEndSwitch(option[3]);
			break;
		}
		case CHANGE_POS_ACCEPTANCE_MODE:
		{
			dc.setPosAcceptanceMode(option[3]);
			break;
		}
		case CHANGE_HOLD_SPEED:
		{
			dc.setHoldSpeed(option[3]);
			break;
		}
		case MAKE_ENDSWITCH_ENABLE_OR_DISABLE :
		{
			if (option[3] == 0)
				dc.isEndSwitchEnabled = false;
			else
				dc.isEndSwitchEnabled = true;
			break;
		}
		case MAKE_ME_ENABLE_OR_DISABLE:
		{
			if (option[3] == 0)
			{
				dc.runEndlesslyWithoutAcceleration(0, dc.DIRECTION_CCW);
				dc.setCurrentPosition(0);
				//User_Defined
				dc.problemCode = PROBLEM_USER_DEFINED;

				dc.isEnabled = false;
			}
			else
			{
				//User_Defined
				dc.problemCode = 0;
				dc.isEnabled = true;
			}
			break;
		}

		case CHANGE_SLIP_TIME:
		{
			dc.setSlipTime(option[3]*5);
			break;
		}
		case CHANGE_SLIP_DISTANCE:
		{
			dc.setSlipDistance(option[3]);
			break;
		}

		case SLIP_FUNCTION_ENABLE_OR_DISABLE:
		{
			dc.setEnableDoNotLetMotorGoesInfinity(option[3]);
			break;
		}

		case DEBUG_ENABLE_OR_DISABLE:
		{
			if(option[3] == 0)
			{
				debugOn = false;
			}
			else
			{
				debugOn = true;
			}
			break;
		}

		case GIVE_ME_DATA:
		{
			switch (option[3])
			{
				case RETURN_PROBLEM_CODE:
				{
					rs.setMeMaster();
					delay(10 * TIMER0_ADJUST);
					Serial.write(dc.problemCode);
					delay(10 * TIMER0_ADJUST);
					rs.setMeSlave();
					break;
				}

				case RETURN_POS:
				{
					rs.setMeMaster();
					delay(10 * TIMER0_ADJUST);
					Serial.write((uint8_t)dc.currentPosition());
					delay(10 * TIMER0_ADJUST);
					rs.setMeSlave();
					break;
				}
				case RETURN_COLOR:
				{
					uint8_t colorBuff[3] = { 0,0,0 };
					colorBuff[0] = G;
					colorBuff[1] = R;
					colorBuff[2] = B;
					rs.setMeMaster();
					delay(10 * TIMER0_ADJUST);
					Serial.write(colorBuff,3);
					delay(10 * TIMER0_ADJUST);
					rs.setMeSlave();
					break;
				}
			}

			break;
		}
		

		}
	}

	else if(option[0] == 30 && option[1] == 30)
	{
		switch (option[2])
		{
			case RUN:
			{
				if(dc.isEnabled)
				{
					runAllFlag = true;
				}
				break;
			}
			case RUN_TILL_THE_ENDSWITCH:
			{
				if (dc.isEnabled)
				{
					if (!dc.runTillEndSwitch())
					{
						digitalWrite(DEBUG_LED_PIN, HIGH);
					}
				}
				break;
			}
			case GIVE_ME_ROW_ADDRESS:
			{

				if (columnAddress == 1)
				{
					rs.setMeMaster();
					delay(10 * TIMER0_ADJUST);
					Serial.write(rowAddress);
					delay(10 * TIMER0_ADJUST);
					rs.setMeSlave();
				}
				digitalWrite(DEBUG_LED_PIN, HIGH);
				delay(100 * TIMER0_ADJUST);
				digitalWrite(DEBUG_LED_PIN, LOW);
				delay(100 * TIMER0_ADJUST);
				digitalWrite(DEBUG_LED_PIN, HIGH);
				delay(100 * TIMER0_ADJUST);
				break;
			}

			case DEBUG_ENABLE_OR_DISABLE:
			{
				if(option[3] == 0)
				{
					debugOn = false;
				}
				else
				{
					debugOn = true;
				}
				break;
			}
		}
	}
}

