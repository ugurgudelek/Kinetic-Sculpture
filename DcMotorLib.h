/*
 Name:		DcMotorLib.h
 Created:	11/3/2015 3:21:02 PM
 Author:	Ugur Gudelek
 Editor:	http://www.visualmicro.com
*/

#ifndef _DcMotorLib_h
#define _DcMotorLib_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define PROBLEM_ON_START 1
#define PROBLEM_USER_DEFINED 2
#define PROBLEM_RUN_TILL_THE_END_SWITCH 3
#define PROBLEM_END_SWITCH_PUSHED 4
#define PROBLEM_ENCODER_MISSED 5

class DcMotorLib{
public:
	DcMotorLib(uint8_t cwPin, uint8_t ccwPin, uint8_t encoderPin, uint8_t motorEnablePin, uint8_t endSwitchPin); //CwPin : in1 , CcwPin : in2

	uint8_t problemCode;

	typedef enum
	{
		DIRECTION_CCW = 1,  ///< Counter-Clockwise
		DIRECTION_CW = 0   ///< Clockwise

	} Direction;

	typedef enum
	{
		NO_CHANGE_UP = 0,
		NO_CHANGE_DOWN,
		STALL,

		DECELERATION_QUICK_UP,
		ACCELERATION_UP,
		DECELERATION_UP,

		DECELERATION_QUICK_DOWN,
		ACCELERATION_DOWN,
		DECELERATION_DOWN,


	} Acceleration;

	void	enableOutputs();
	long    distanceToGo();
	long    targetPosition();
	long    currentPosition();
	float   speed();
	bool	direction();
	int		encoderCounter();
	long	lastSavedPosition();


	void    setCurrentPosition(long position);
	void	setMinSpeedUp(float speed);
	void	setMinSpeedDown(float speed);
	void    setMaxSpeed(float speed);
	void	setStallSpeed(float speed);
	void	setGoBaseSpeed(uint8_t speed);
	void	setHoldSpeed(uint8_t speed);


	float	minSpeedUp();
	float	minSpeedDown();
	uint8_t goBaseSpeed();
	Acceleration accelerationMode();

	void    move(long relative);
	void    moveTo(long absolute);
	void	run();
	void	stall();
	void	runToTheOutputs();
	void	encoderRead();
	bool	runTillEndSwitch();  
	void	posCalibration();

	void	runEndlesslyWithoutAcceleration(uint8_t speed, Direction direction);

	void	accelerationWithTime();
	void	computeAccelerationMode();
	void	setAcceleration(float step, unsigned long minToMax);
	void	setAccelerationCoef();
	void	setAccelerationStep(float step);
	void	setAccelerationMinToMaxTime(unsigned long minToMaxTime);

	void	setEndSwithRespondTime(unsigned long time);
	void	setMaxPos(long maxPos);
	void	setQuickSlowDownDistanceDown(uint8_t distance);
	void	setQuickSlowDownDistanceUp(uint8_t distance);
	void	setQuickSlowDownSpeedDown(uint8_t speed);
	void	setMinSpeedAfterEndSwitch(uint8_t speed);
	float	getMinSpeedAfterEndSwitch();
	bool	isEnabled;
	bool	isEndSwitchEnabled;
	uint8_t getPosAcceptanceMode();
	void	setPosAcceptanceMode(uint8_t posAcceptanceMode);

	void	doNotLetMotorGoesInfinity();
	void setSlipTime(unsigned long time);
	void setSlipDistance(uint8_t distance);
	void setEnableDoNotLetMotorGoesInfinity(bool value);

	void setTargetPosition(uint8_t distance);

private:
	volatile int			_currentPos; 
	long			_targetPos;
	float			_speed;			
	Direction		_direction;
	float			_maxSpeed;
	float			_minSpeedUp;
	float			_minSpeedDown;
	float			_stallSpeed;
	uint8_t			_goBaseSpeed;
	uint8_t			_holdSpeed;
	float			_accelerationWithTimeStep;

	uint8_t			_ccwPin;
	uint8_t			_cwPin;
	uint8_t			_encoderPin;
	uint8_t			_motorEnablePin;
	uint8_t			_endSwitchPin;

	volatile int	_encoderCounter;
	bool			_encoderFlag;
	bool			_encoderValue;
	bool			_encoderOldValue;

	double			_accelerationWithTimeTimer;
	bool			_accelerateWithTimeFlag;
	Acceleration	_accelerationMode;
	float			_accelerationWithTimeCoef;
	unsigned long	_accelerationWithTimePeriod;
	unsigned long	_minToMaxTime;

	unsigned long	_endSwithRespondTime;
	
	long			_maxPos;
	uint8_t			_quickSlowDownDistanceDown;
	uint8_t			_quickSlowDownDistanceUp;
	uint8_t			_quickSlowDownSpeedDown;
	float			_minSpeedAfterEndSwitch;

	uint8_t			_posAcceptanceMode;
	long			_waitingTarget;

	double _slipTimer;
	unsigned long _slipTime;
	uint8_t _slipDistance;
	bool _enableDoNotLetMotorGoesInfinity;


	
};


#endif

