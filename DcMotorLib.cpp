/*
 Name:		DcMotorLib.cpp
 Created:	11/3/2015 3:21:02 PM
 Author:	Ugur Gudelek
 Editor:	http://www.visualmicro.com
*/

#include "DcMotorLib.h"
#define TIMER0_ADJUST (unsigned long)64



DcMotorLib::DcMotorLib(uint8_t cwPin, uint8_t ccwPin, uint8_t encoderPin, uint8_t motorEnablePin, uint8_t endSwitchPin)
{
	_currentPos = _targetPos = 0;
	_minSpeedUp		= _speed = 150; //150
	_minSpeedDown	= 100;
	_maxSpeed		= 255;
	_stallSpeed		= 80; // 80
	_goBaseSpeed	= 150;
	_holdSpeed		= 40; // 40
	_direction		= DIRECTION_CW;
	_maxPos			= 150;
	_quickSlowDownDistanceDown = 3; //3
	_quickSlowDownSpeedDown = 0;
	_quickSlowDownDistanceUp = 2; //2
	_minSpeedAfterEndSwitch = 100;
	_posAcceptanceMode = 3;


	_accelerationWithTimeStep	= 20.0;
	_minToMaxTime				= 140;
	_accelerationWithTimePeriod = (float)_minToMaxTime / _accelerationWithTimeStep;
	_accelerationWithTimeCoef	= (_maxSpeed - _minSpeedUp) / ((float)_minToMaxTime / _accelerationWithTimePeriod);
	_accelerationWithTimeTimer	= -1;
	_accelerateWithTimeFlag		= false;
	_accelerationMode			= STALL;


	_cwPin			= cwPin;
	_ccwPin			= ccwPin;
	_encoderPin		= encoderPin;
	_motorEnablePin = motorEnablePin;
	_endSwitchPin	= endSwitchPin;

	_encoderCounter		= 0;
	_encoderOldValue	= _encoderValue = digitalRead(_encoderPin);
	_encoderFlag		= false;

	_endSwithRespondTime = 10000;  //10000

	isEnabled = false;
	problemCode = PROBLEM_ON_START;
	isEndSwitchEnabled = true;
	_targetPos = _waitingTarget = 0;

	_slipTimer = -1;
	_slipTime = 250;
	_slipDistance = 5;
	_enableDoNotLetMotorGoesInfinity = true; //true




}

void	DcMotorLib::enableOutputs()
{
	pinMode(_motorEnablePin, OUTPUT);
	pinMode(_ccwPin, OUTPUT);
	pinMode(_cwPin, OUTPUT);
	pinMode(_encoderPin, INPUT);
	pinMode(_endSwitchPin, INPUT);

	digitalWrite(_motorEnablePin, HIGH);
	digitalWrite(_cwPin, LOW);
	digitalWrite(_ccwPin, LOW);
}

long	DcMotorLib::distanceToGo()
{
	return _targetPos - _currentPos;
}

long	DcMotorLib::targetPosition()
{
	return _targetPos;
}

long	DcMotorLib::currentPosition()
{
	return _currentPos;
}

float	DcMotorLib::speed()
{
	return _speed;
}

bool	DcMotorLib::direction()
{
	return _direction;
}

int		DcMotorLib::encoderCounter()
{
	return _encoderCounter;
}

void	DcMotorLib::setHoldSpeed(uint8_t speed)
{
	_holdSpeed = speed;
}

void	DcMotorLib::setCurrentPosition(long position)
{
	_currentPos = position;
	_targetPos = position;
	_waitingTarget = position;
	_encoderCounter = 0;
}

void	DcMotorLib::setMinSpeedUp(float speed)
{
	if (_minSpeedUp != speed)
	{
		_minSpeedUp = speed;
		setAccelerationCoef();
	}
}

void	DcMotorLib::setMinSpeedDown(float speed)
{
	if (_minSpeedDown != speed)
	{
		_minSpeedDown = speed;
		setAccelerationCoef();
	}
}

void	DcMotorLib::setMaxSpeed(float speed)
{
	if (_maxSpeed != speed)
	{
		_maxSpeed = speed;
		setAccelerationCoef();
	}
}

void	DcMotorLib::setStallSpeed(float speed)
{
	if (_stallSpeed != speed)
	{
		_stallSpeed = speed;
	}
}

void	DcMotorLib::setGoBaseSpeed(uint8_t speed)
{
	_goBaseSpeed = speed;
}

void	DcMotorLib::setAcceleration(float step, unsigned long minToMaxTime)
{
	_accelerationWithTimeStep = step;
	_minToMaxTime = minToMaxTime;
	_accelerationWithTimePeriod = (float)_minToMaxTime / _accelerationWithTimeStep;
	_accelerationWithTimeCoef = (_maxSpeed - _minSpeedUp) / _accelerationWithTimePeriod;
}

void	DcMotorLib::setAccelerationStep(float step)
{
	_accelerationWithTimeStep = step;
	setAccelerationCoef();
}

void	DcMotorLib::setAccelerationMinToMaxTime(unsigned long minToMaxTime)
{
	_minToMaxTime = minToMaxTime;
	setAccelerationCoef();
}

void	DcMotorLib::setAccelerationCoef()
{
	_accelerationWithTimeCoef = (_maxSpeed - _minSpeedUp) / (_minToMaxTime / _accelerationWithTimePeriod);
}

void	DcMotorLib::setEndSwithRespondTime(unsigned long time)
{
	_endSwithRespondTime = time;
}

void	DcMotorLib::setMaxPos(long maxPos)
{
	_maxPos = maxPos;
}

void	DcMotorLib::setQuickSlowDownDistanceDown(uint8_t distance)
{
	_quickSlowDownDistanceDown = distance;
}
void	DcMotorLib::setQuickSlowDownDistanceUp(uint8_t distance)
{
	_quickSlowDownDistanceUp = distance;
}

void	DcMotorLib::setQuickSlowDownSpeedDown(uint8_t speed)
{
	_quickSlowDownSpeedDown = speed;
}

void	DcMotorLib::setMinSpeedAfterEndSwitch(uint8_t speed)
{
	_minSpeedAfterEndSwitch = speed;
}
float	DcMotorLib::getMinSpeedAfterEndSwitch()
{
	return _minSpeedAfterEndSwitch;
}

void DcMotorLib::setSlipDistance(uint8_t distance)
{
	_slipDistance = distance;
}

void DcMotorLib::setSlipTime(unsigned long time)
{
	_slipTime = time;
}

void DcMotorLib::setEnableDoNotLetMotorGoesInfinity(bool value)
{
	_enableDoNotLetMotorGoesInfinity = value;
}





float	DcMotorLib::minSpeedUp()
{
	return _minSpeedUp;
}

float	DcMotorLib::minSpeedDown()
{
	return _minSpeedDown;
}

DcMotorLib::Acceleration DcMotorLib::accelerationMode()
{
	return _accelerationMode;
}


void	DcMotorLib::move(long relative)
{
	moveTo(_currentPos + relative);
}

void	DcMotorLib::moveTo(long absolute)
{
	if (_targetPos != absolute && absolute <= _maxPos)
	{
		switch(_posAcceptanceMode)
		{
		case 1:
		{
			_targetPos = absolute;
			_waitingTarget = _targetPos;
			_encoderCounter = 0;
			return;
		}

		case 2:
		{
			if (_accelerationMode != STALL)
			{
				if (_direction == DIRECTION_CCW)
				{
					if (absolute < _currentPos)
					{
						_targetPos = absolute;
						_waitingTarget = _targetPos;
						_encoderCounter = 0;
						return;
					}
					else
					{
						if (_accelerationMode == DECELERATION_QUICK_UP)
						{
							_targetPos = absolute;
							_waitingTarget = _targetPos;
							_encoderCounter = 0;
							return;
						}
					}
				}
				else
				{
					if (absolute > _currentPos)
					{
						_targetPos = absolute;
						_waitingTarget = _targetPos;
						_encoderCounter = 0;
						return;
					}
					else
					{
						if (_accelerationMode == DECELERATION_QUICK_DOWN)
						{
							_targetPos = absolute;
							_waitingTarget = _targetPos;
							_encoderCounter = 0;
							return;
						}
					}
				}
			}

			else
			{
				_targetPos = absolute;
				_waitingTarget = _targetPos;
				_encoderCounter = 0;
				return;
			}
			
		}

		case 3:
		{
			if (_accelerationMode != STALL)
			{
				if (_direction == DIRECTION_CCW)
				{
					if (absolute < _currentPos)
					{
						_targetPos = absolute;
						_waitingTarget = _targetPos;
						_encoderCounter = 0;
						return;
					}
					else //new data has wrong direction
					{
						if (_accelerationMode == DECELERATION_QUICK_UP)
						{
							_targetPos = absolute;
							_waitingTarget = _targetPos;
							_encoderCounter = 0;
							return;
						}
						else // need to slowdown Up
						{
							_waitingTarget = absolute;
							if (_accelerationMode == DECELERATION_UP)
							{
								return;
							}
							else
							{
								_targetPos = _currentPos - _accelerationWithTimeStep; // immediately goes into DECELERATION_UP mode
								return;
							}
						}
					}
				}
				else
				{
					if (absolute > _currentPos)
					{
						_targetPos = absolute;
						_encoderCounter = 0;
						_waitingTarget = _targetPos;
						return;
					}
					else
					{

						if (_accelerationMode == DECELERATION_QUICK_DOWN)
						{
							_targetPos = absolute;
							_encoderCounter = 0;
							_waitingTarget = _targetPos;
							return;
						}
						else // need to slowdown Down
						{
							_waitingTarget = absolute;
							if (_accelerationMode == DECELERATION_DOWN)
							{
								return;
							}
							else
							{
								_targetPos = _currentPos + _accelerationWithTimeStep; // immediately goes into DECELERATION_DOWN mode
								return;
							}
						}
					}
				}
			}

			else
			{
				_targetPos = absolute;
				_encoderCounter = 0;
				_waitingTarget = _targetPos;
				return;
			}
		}

		}
		
	
	}
}

void	DcMotorLib::run()
{
	encoderRead();
	if (distanceToGo() == 0)
	{
		stall();
		_slipTimer = -1;
	}
	if(_enableDoNotLetMotorGoesInfinity)
	{
		doNotLetMotorGoesInfinity();
	}
	computeAccelerationMode();

	if (_encoderFlag || _accelerateWithTimeFlag)
	{
		runToTheOutputs();
		if (distanceToGo() == 0)
		{
			stall();
			_slipTimer = -1;
		}
		_encoderFlag = false;
	}
}

bool	DcMotorLib::runTillEndSwitch()
{
	
		unsigned long tic = millis();
		move(3);
		while (distanceToGo() != 0 && (millis() < tic + 500 * TIMER0_ADJUST))	//wait to go 3
		{
			run();
		}
		stall();
		setCurrentPosition(0);
		delay(10 * TIMER0_ADJUST);

		tic = millis();

		if(isEndSwitchEnabled)
		{
			while (true)
			{

				//go some random distance to the reverse direction to obtain endSwitch
				runEndlesslyWithoutAcceleration(_goBaseSpeed, DIRECTION_CCW);
				delay(10 * TIMER0_ADJUST);

				if (digitalRead(_endSwitchPin) == 0)   //Change direction for much precise endSwitch Calibration
				{
					stall();	//halt now!
								//delay(500*TIMER0_ADJUST); //timer0
					delay(200 * TIMER0_ADJUST);

					move(10);
					float minSpeedTemp = _minSpeedUp;
					_minSpeedUp = _minSpeedAfterEndSwitch;
					tic = millis();
					while (distanceToGo() != 0 && (millis() < tic + 500 * TIMER0_ADJUST))
					{
						run();
					}

					stall();
					_minSpeedUp = minSpeedTemp;
					setCurrentPosition(0);
					isEnabled = true;
					problemCode = 0;
					_direction = DIRECTION_CCW;
					return true;
				}
				else if (millis() > tic + _endSwithRespondTime * TIMER0_ADJUST)
				{
					problemCode = PROBLEM_RUN_TILL_THE_END_SWITCH;
					setCurrentPosition(0);

					_direction = DIRECTION_CCW;
					runEndlesslyWithoutAcceleration(0, DIRECTION_CCW);
					isEnabled = false;

					return false;
				}
			}
		}
		
		else
		{
			runEndlesslyWithoutAcceleration(_goBaseSpeed, DIRECTION_CCW);
			delay(10000 * TIMER0_ADJUST); // Pull up blindly 10 sec
			stall();
			move(10);
			tic = millis();
			while (distanceToGo() != 0 && (millis() < tic + 500 * TIMER0_ADJUST))
			{
				run();
			}
			stall();
			setCurrentPosition(0);
			_direction = DIRECTION_CCW;
			isEnabled = true;
			problemCode = 0;
			return true;
		}
		
}

uint8_t DcMotorLib::goBaseSpeed()
{
	return _goBaseSpeed;
}


void	DcMotorLib::posCalibration()
{
		double toc = millis();
		stall();

		move(10);

		while (distanceToGo() != 0 || (millis() < toc + 500 * TIMER0_ADJUST))
		{
			run();
		}
		stall();
		setCurrentPosition(0);
		_direction = DIRECTION_CW;

	
}

void	DcMotorLib::runEndlesslyWithoutAcceleration(uint8_t speed, Direction direction)
{
	if(isEnabled)
	{
		if (direction == DIRECTION_CCW)
		{
			// Counterclockwise 
			digitalWrite(_cwPin, LOW);
			analogWrite(_ccwPin, speed);

		}
		else //if (_direction == DIRECTION_CWW)
		{
			// Clockwise 
			digitalWrite(_ccwPin, LOW);
			analogWrite(_cwPin, speed);

		}
	}
	
}

void	DcMotorLib::stall()
{
	_accelerationMode = STALL;
	runToTheOutputs();
	moveTo(_waitingTarget);
}


void	DcMotorLib::computeAccelerationMode()
{

	long distanceTo = distanceToGo(); // +ve is clockwise from curent location

	if (distanceTo > 0)
	{
		_direction = DIRECTION_CW;
		// We are clockwise from the target
		//SlowDown Now
		if (distanceTo < _accelerationWithTimeStep)
		{
			if(_accelerationMode == NO_CHANGE_DOWN || _accelerationMode == STALL || _accelerationMode == NO_CHANGE_UP)
			{
				_accelerationMode = NO_CHANGE_DOWN;
			}
			else
			{
				if (distanceTo <= _quickSlowDownDistanceDown)
				{
					_accelerationMode = DECELERATION_QUICK_DOWN;
				}
				else
				{
					//deceleration
					_accelerationMode = DECELERATION_DOWN;

				}
			}

		}

		else // (distanceTo >(_targetPos - _lastSavedPos - _accelerationWithTimeStep))
		{
			//acceleration
			_accelerationMode = ACCELERATION_DOWN;
		}
		//else
		//{
		//	//perfect speed
		//	_accelerationMode = NO_CHANGE;
		//}
	}
	else if (distanceTo < 0)
	{
		_direction = DIRECTION_CCW;
		distanceTo = -distanceTo;
		// We are clockwise from the target
		if (distanceTo < _accelerationWithTimeStep)
		{
			if (_accelerationMode == NO_CHANGE_UP || _accelerationMode == STALL || _accelerationMode == NO_CHANGE_DOWN)
			{
				_accelerationMode = NO_CHANGE_UP;
			}
			else
			{
				if (distanceTo == _quickSlowDownDistanceUp)
				{
					_accelerationMode = DECELERATION_QUICK_UP;
				}
				else
				{
					//deceleration
					_accelerationMode = DECELERATION_UP;
				}
			}
		}
		else //(distanceTo >(_targetPos - _lastSavedPos - _accelerationWithTimeStep))
		{
			//acceleration
			_accelerationMode = ACCELERATION_UP;
		}
		//else
		//{
		//	//perfect speed
		//	_accelerationMode = NO_CHANGE;
		//}
	}
	else
	{
		_accelerationMode = STALL;
	}

	accelerationWithTime();
}

void	DcMotorLib::accelerationWithTime()
{

	if (_accelerationWithTimeTimer == -1)
	{
		_accelerationWithTimeTimer = millis();
		_accelerateWithTimeFlag = false;
	}
	else if (millis() > _accelerationWithTimeTimer + _accelerationWithTimePeriod*TIMER0_ADJUST)
	{
		//Serial.println(millis()/TIMER0_ADJUST);
		
		switch (_accelerationMode)
		{
		case ACCELERATION_UP:
		{
			_speed += _accelerationWithTimeCoef;
			_speed = constrain(_speed, _minSpeedUp, _maxSpeed);
			break;
		}
		case ACCELERATION_DOWN:
		{
			_speed += _accelerationWithTimeCoef/2;
			_speed = constrain(_speed, _minSpeedDown, _maxSpeed);
			break;
		}
		case DECELERATION_UP:
		{
			_speed -= _accelerationWithTimeCoef/1.2;
			_speed = constrain(_speed, _minSpeedUp, _maxSpeed);
			break;
		}
		case DECELERATION_DOWN:
		{
			_speed -= _accelerationWithTimeCoef*2.4;
			_speed = constrain(_speed, _minSpeedDown, _maxSpeed);
			break;
		}
		case DECELERATION_QUICK_UP:
		{
			_speed = _minSpeedUp;
			moveTo(_waitingTarget);
			break;
		}
		case DECELERATION_QUICK_DOWN:
		{
			_speed = _quickSlowDownSpeedDown;
			moveTo(_waitingTarget);
			break;
		}
		case NO_CHANGE_UP:
		{
			_speed = _minSpeedUp;
			break;
		}
		case NO_CHANGE_DOWN:
		{
			_speed = _minSpeedDown;
			break;
		}

		case STALL:
		{
			stall();
			break;
		}
		}

		
		_accelerationWithTimeTimer = -1;
		_accelerateWithTimeFlag = true;
	}

}


void	DcMotorLib::runToTheOutputs()
{
	if(isEnabled)
	{
		if (_direction == DIRECTION_CCW)
		{
			// Counterclockwise 
			// There is no need to stall
			if (_accelerationMode != STALL)
			{
				digitalWrite(_cwPin, LOW);
				analogWrite(_ccwPin, _speed);
			}
			else
			{
				digitalWrite(_ccwPin, LOW);
				digitalWrite(_cwPin, LOW);
				delay(10 * TIMER0_ADJUST); //timer0
				digitalWrite(_cwPin, LOW);
				analogWrite(_ccwPin, _holdSpeed);

			}

		}
		else //if (_direction == DIRECTION_CW)
		{
			// Clockwise 
			if (_accelerationMode != STALL)
			{
				digitalWrite(_ccwPin, LOW);
				analogWrite(_cwPin, _speed);
			}
			else
			{
				digitalWrite(_cwPin, LOW);
				analogWrite(_ccwPin, _stallSpeed);
			}
		}
	}
	

}

void	DcMotorLib::encoderRead()
{
	_encoderValue = digitalRead(_encoderPin);
	if ((_encoderOldValue != _encoderValue)) //check if any transition occured
	{
		_slipTimer = -1;
		_encoderOldValue = _encoderValue;
		
		_encoderCounter++; //count up
		_encoderFlag = true;

		if (_direction == DIRECTION_CCW)
		{
			_currentPos--;
		}
		else
		{
			_currentPos++;
		}
		delay(1* TIMER0_ADJUST); //timer0
	}
}

uint8_t DcMotorLib::getPosAcceptanceMode()
{
	return _posAcceptanceMode;
}

void DcMotorLib::setPosAcceptanceMode(uint8_t posAcceptanceMode)
{
	_posAcceptanceMode = posAcceptanceMode;
}


void DcMotorLib::doNotLetMotorGoesInfinity()
{
	if(_direction == DIRECTION_CW && distanceToGo() >= _slipDistance)
	{
		if (_slipTimer == -1)
		{
			//start slipTimer
			_slipTimer = millis();
		}

		else if (millis() > _slipTimer + _slipTime * TIMER0_ADJUST)
		{
			//stall now and enter disable mode
			stall();
			runEndlesslyWithoutAcceleration(_goBaseSpeed, DIRECTION_CCW);
			delay(10000 * TIMER0_ADJUST); // Pull up blindly 10 sec

			runEndlesslyWithoutAcceleration(0, DIRECTION_CCW);

			setCurrentPosition(0);

			isEnabled = false;
			problemCode = PROBLEM_ENCODER_MISSED;


			_slipTimer = -1;
		}
	}
}


void DcMotorLib::setTargetPosition(uint8_t distance)
{
	_targetPos = distance;
}








#if 0
void	DcMotorLib::setUpDownSpeedCoef(float upDownSpeedCoef)
{
	_upDownSpeedCoef = upDownSpeedCoef;
}


void	DcMotorLib::upDownSpeedCalibration()
{
	if (_speed != _stallSpeed)
	{
		
		if (_direction == DIRECTION_CCW)
		{
			_speed = constrain(_speed, _minSpeed * _upDownSpeedCoef, ((_maxSpeed * _upDownSpeedCoef) > 255.0) ? 255.0 : (_maxSpeed * _upDownSpeedCoef));
		}

		else if (_direction == DIRECTION_CW)
		{
			_speed = constrain(_speed, _minSpeed, _maxSpeed);
		}

	}
}



void	DcMotorLib::speedUpWithTime()
{
	long distanceTo = distanceToGo();

	if (distanceTo != 0)
	{
		if (_speedUpSaveTicFlag)
		{
			_distanceSavedForSpeedUp = distanceTo;



			_speedUpSaveTicFlag = false;
			_speedUpCheckTicFlag = true;

			_speedUpCheckTic = millis();

		}

		if (_speedUpCheckTicFlag && millis() > _speedUpCheckTic + _speedUpWithTimePeriod)
		{

			if (_distanceSavedForSpeedUp == distanceTo)
			{
				//speedUpNow
				_speed = _speed * _speedUpWithTimeCoef;



				if (_direction == DIRECTION_CCW)
				{
					_speed = constrain(_speed, _minSpeed * _upDownSpeedCoef, ((_maxSpeed * _upDownSpeedCoef) > 255.0) ? 255.0 : (_maxSpeed * _upDownSpeedCoef));
				}

				else if (_direction == DIRECTION_CW)
				{
					_speed = constrain(_speed, _minSpeed, _maxSpeed);
				}

				_speedUpWithTimeFlag = true;
			}

			_speedUpSaveTicFlag = true;
			_speedUpCheckTicFlag = false;
		}
	}
}

void	DcMotorLib::setSpeedUpWithTimePeriod(unsigned long speedUpWithTimePeriod)
{
	_speedUpWithTimePeriod = speedUpWithTimePeriod * TIMER0_ADJUST; //*64 comes from timer0 change
}

void	DcMotorLib::setSpeedUpWithTimeCoef(float speedUpWithTimeCoef)
{
	_speedUpWithTimeCoef = speedUpWithTimeCoef;
}

#endif


