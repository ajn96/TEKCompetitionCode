#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    mobileGoalAngle, sensorPotentiometer)
#pragma config(Sensor, in2,    armAngle,       sensorPotentiometer)
#pragma config(Sensor, dgtl1,  tankDriveSignal, sensorLEDtoVCC)
#pragma config(Sensor, dgtl12, hasMobileGoal,  sensorTouch)
#pragma config(Sensor, I2C_1,  driveEncoderLeft, sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  driveEncoderRight, sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           armRight,      tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           goalRight,     tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           coneMotor,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           leftInteriorMotor, tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port6,           leftExteriorMotors, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           rightExteriorMotors, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           rightInteriorMotor, tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port9,           goalLeft,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          armLeft,       tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*        Description: Competition template for VEX EDR                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

// This code is for the VEX cortex platform
#pragma platform(VEX2)

// Select Download method as "competition"
#pragma competitionControl(Competition)

//Main competition background code...do not modify!
#include "Vex_Competition_Includes.c"

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the cortex has been powered on and    */
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

#define DEADZONE 30
#define ARM_SPEED 100
#define ROLLER_SPEED 100
#define GOAL_SPEED 100

//function declarations
void drive( bool isTank );
void moveArm(bool up, bool down, bool autoHoldHeight, int currentSpeed);
void rollRoller( bool up, bool down );
void moveGoal( bool up, bool down );
void holdArm(int overrideAngle, int currentSpeed);

//task declarations
task autoDropMobile();
task autoDropFixed();
task autoLoadHumanPlayerStation();

//globals
int GOAL_START_ANGLE;
int ARM_CURRENT_ANGLE;
int holdAngle;

bool autoLiftRunning;
bool armDownDecreased;
bool armUpDecreased;
bool armDownFound;
bool armUpFound;

//globals for PID
int armUpPower;
int armDownPower;

//button encoding
const int LIFT_UP_BTN = Btn6U,
				  LIFT_DOWN_BTN = Btn6D,
				  CONE_UP_BTN = Btn8L,
				  CONE_DOWN_BTN = Btn8D,
				  GOAL_UP_BTN = Btn8U,
				  GOAL_DOWN_BTN = Btn8R,
				  AUTO_LIFT_MOBILE = Btn5U,
				  AUTO_LIFT_FIXED = Btn5D,
				  AUTO_HOLD_HEIGHT_ON = Btn7D,
				  AUTO_HOLD_HEIGHT_OFF = Btn7R;

void pre_auton()
{
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks
  // running between Autonomous and Driver controlled modes. You will need to
  // manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;

	// Set bDisplayCompetitionStatusOnLcd to false if you don't want the LCD
	// used by the competition include file, for example, you might want
	// to display your team name on the LCD in this function.
	// bDisplayCompetitionStatusOnLcd = false;
  holdAngle = 1600;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

task autonomous()
{
	bool robotInPosition = false;
	clearTimer(T1);

	//Robot drives forward
	resetMotorEncoder(rightInteriorMotor);
	resetMotorEncoder(leftInteriorMotor);

	while(!robotInPosition)
	{
		if(time1[T1] < 750)
		{
			rollRoller(true, false);
		}
		if(time1[T1] > 1000 && time1[T1] < 1750)
		{
			motor[leftExteriorMotors] = motor[leftInteriorMotor] =  motor[rightExteriorMotors] = motor[rightInteriorMotor] = -50;
		}
		else if(time1[T1] < 2500)
		{
			motor[leftExteriorMotors] = motor[leftInteriorMotor] =  motor[rightExteriorMotors] = motor[rightInteriorMotor] = 50;
		}
		else if(time1[T1] < 3500)
		{
			motor[leftExteriorMotors] = motor[leftInteriorMotor] =  motor[rightExteriorMotors] = motor[rightInteriorMotor] = -50;
		}
		else
		{
			motor[leftExteriorMotors] = motor[leftInteriorMotor] =  motor[rightExteriorMotors] = motor[rightInteriorMotor] = 0;
		}
		if(SensorValue[armAngle] < 1600)
		{
			motor[armLeft] = motor[armRight] = 100;
		}
		else
		{
			motor[armLeft] = motor[armRight] = 0;
		}
		if(time1[T1] > 3500)
		{
			robotInPosition = true;
		}
	}
	sleep(1000);

	startTask(autoDropFixed);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */


task usercontrol()
{
	GOAL_START_ANGLE = SensorValue[mobileGoalAngle];
	ARM_CURRENT_ANGLE = SensorValue[armAngle];

	bool isTank = true;
	bool autoHoldHeight = true;

	int armPositionNew = SensorValue[armAngle];
	int armPositionOld = armPositionNew;
	int armRotationalSpeed;
	int tenMSPoll;

	autoLiftRunning = false;
	armDownDecreased = false;
	armUpDecreased = false;
	armDownFound = false;
	armUpFound = false;

	//clears timer2
	clearTimer(T2);

  while ( true ) {
  	//drive function
  	drive( isTank );

  	//manual arm control
  	if(!autoLiftRunning)
  	{
  		moveArm( vexRT[LIFT_UP_BTN] == 1, vexRT[LIFT_DOWN_BTN] == 1, autoHoldHeight, armRotationalSpeed);
  	}

  	//manual roller control
  	rollRoller( vexRT[CONE_UP_BTN] == 1, vexRT[CONE_DOWN_BTN] == 1 ); // ( up, down )

  	//manual goal control
  	moveGoal( vexRT[GOAL_UP_BTN] == 1, vexRT[GOAL_DOWN_BTN] == 1 ); // ( up, down )

  	//Auto Lift and Drop Activation
  	if( vexRT[AUTO_LIFT_MOBILE])
  	{
  		startTask( autoDropMobile );
  	}

  	if(vexRT[AUTO_LIFT_FIXED])
  	{
  		startTask( autoDropFixed );
  	}

  	//turns on and off auto hold height
  	if(vexRT[AUTO_HOLD_HEIGHT_ON])
  	{
  			autoHoldHeight = true;
  	}
  	if(vexRT[AUTO_HOLD_HEIGHT_OFF])
  	{
  			autoHoldHeight = false;
  	}

  	//run every 10ms
  	if(time1[T2] - tenMSPoll > 10)
  	{
  		tenMSPoll = time1[T2];
  		armPositionOld = armPositionNew;
  		armPositionNew = SensorValue[armAngle];
  		armRotationalSpeed = armPositionNew - armPositionOld;
  	}

  	if(vexRT[Btn7L])
  	{
  		startTask(autonomous);
  	}
  }
}

//task to automatically pick up cone from human player station
//sets arm to appropriate height
//requires robot to start at correct distance and orientation
task autoLoadHumanPlayerStation()
{
	clearTimer(T1);
	bool coneLoaded = false;
	int clearHPS;
	int pickupHeight;
	while(!coneLoaded)
	{
		if(time1[T1] < 1000)
		{
			holdArm(clearHPS,0);
		}
		else if(time1[T1] < 2000)
		{
	 		holdArm(pickupHeight, 0);
	 		rollRoller(true, false);
		}
		else
		{
			coneLoaded = true;
		}
	}
	startTask(autoDropMobile);
}

//raises arm to mobile goal drop point and drops cone
task autoDropMobile(){
	int motorSpeed;
	bool isDropped = false;
	autoLiftRunning = true;
	const int ARM_DROP_ANGLE_MOBILE = 1650;
	//raise at full speed to 1200
	while(SensorValue[armAngle] < 1200)
	{
		motor[armLeft] = motor[armRight] = 127;
	}

	while(SensorValue[armAngle] < ARM_DROP_ANGLE_MOBILE)
	{
		float speedMultiplier = (SensorValue[armAngle] - 1200)/ (ARM_DROP_ANGLE_MOBILE - 1200);
		speedMultiplier = speedMultiplier * 127;
		motorSpeed = 127 - (int) speedMultiplier;
		motor[armLeft] = motor[armRight] = motorSpeed;
	}
	motor[armLeft] = motor[armRight] = 0;
	clearTimer(T1);
	while(!isDropped)
	{
		if( SensorValue[armAngle] > ARM_DROP_ANGLE_MOBILE + 50 )
		{
			motor[armLeft] = motor[armRight] = -30;
		}
		else if ( SensorValue[armAngle] < ARM_DROP_ANGLE_MOBILE -50)
		{
			motor[armLeft] = motor[armRight] = 30;
		}
		else
		{
			motor[armLeft] = motor[armRight] = 0;
		}
		if (time1[T1] > 1000)
		{
			motor[coneMotor] = -ROLLER_SPEED;
		}
		if(time1[T1] > 2000)
		{
			motor[coneMotor] = 0;
			isDropped = true;
		}
	}
	autoLiftRunning = false;
}

//drops a cone on the fixed goal
task autoDropFixed()
{
	int motorSpeed;
	bool isDropped = false;
	autoLiftRunning = true;
	const int ARM_DROP_ANGLE_FIXED = 2150;
	while(SensorValue[armAngle] < 1600)
	{
		motor[armLeft] = motor[armRight] = 127;
	}
	while(SensorValue[armAngle] < ARM_DROP_ANGLE_FIXED)
	{
		float speedMultiplier = (SensorValue[armAngle] - 1200)/ (ARM_DROP_ANGLE_FIXED - 1200);
		speedMultiplier = speedMultiplier * 127;
		motorSpeed = 127 - (int) speedMultiplier;
		motor[armLeft] = motor[armRight] = motorSpeed;
	}
	clearTimer(T1);
	while(!isDropped)
	{
		if( SensorValue[armAngle] > ARM_DROP_ANGLE_FIXED + 50 )
		{
			motor[armLeft] = motor[armRight] = -30;
		}
		else if ( SensorValue[armAngle] < ARM_DROP_ANGLE_FIXED -50)
		{
			motor[armLeft] = motor[armRight] = 30;
		}
		else
		{
			motor[armLeft] = motor[armRight] = 0;
		}
		if (time1[T1] > 1000)
		{
			motor[coneMotor] = -ROLLER_SPEED;
		}
		if(time1[T1] > 2000)
		{
			motor[coneMotor] = 0;
			isDropped = true;
		}
	}
	autoLiftRunning = false;
}

//drive code for the robot
//@param isTank: bool indicating if tank or arcade drive
void drive( bool isTank ) {
	if ( isTank ) {
		int leftSpeed = vexRT[Ch3];
		int rightSpeed = vexRT[Ch2];
		if ( abs( leftSpeed ) <= DEADZONE )	{
			leftSpeed = 0;
		}
		if ( abs( rightSpeed ) <= DEADZONE ) {
			rightSpeed = 0;
		}
		motor[rightInteriorMotor] = rightSpeed;
		motor[leftExteriorMotors] = leftSpeed;
		motor[rightExteriorMotors] = rightSpeed;
		motor[leftInteriorMotor] = leftSpeed;
	} else {
			motor[leftExteriorMotors] = motor[rightInteriorMotor]  = (vexRT[Ch3] - vexRT[Ch4]) / 2;  // (y + x)/2
    	motor[rightExteriorMotors] = motor[leftInteriorMotor] = (vexRT[Ch3] + vexRT[Ch4]) / 2;  // (y - x)/2
  }
}

void holdArm(int overrideAngle, int currentSpeed)
{
	if(overrideAngle != 0)
	{
		holdAngle = overrideAngle;
	}
	if (SensorValue[armAngle] > holdAngle + 50)
	{
		if(!armDownFound)
		{
			if(currentSpeed > 0)
			{
				armDownPower--;
				armDownDecreased = true;
			}
			if(currentSpeed < 1)
			{
				armDownPower++;
				if(armDownDecreased)
				{
					armDownFound = true;
				}
			}
		}
		motor[armRight] = motor[armLeft] = -30; //armDownPower;
	}
	if(SensorValue[armAngle] < holdAngle - 50)
	{
		if(!armUpFound)
		{
			if(currentSpeed < 0)
			{
				armUpPower++;
				if(armUpDecreased)
				{
					armUpFound = true;
				}
			}
			if(currentSpeed > -1)
			{
				armUpPower--;
				armUpDecreased = true;
			}
		}
		motor[armRight] = motor[armLeft] = 50; //armUpPower;
	}
	else
	{
		motor[armRight] = motor[armLeft] = 0;
	}
}

//controls the movement of the arm
void moveArm( bool up, bool down, bool autoHoldHeight, int currentSpeed) {
	if ( up ) {
		motor[armRight] = motor[armLeft] = ARM_SPEED;
		holdAngle = SensorValue[armAngle];
		armUpPower = 50;
		armDownPower = -30;
		armDownDecreased = false;
		armUpDecreased = false;
		armDownFound = false;
		armUpFound = false;
	}
	else if ( down ) {
		motor[armRight] = motor[armLeft] = -ARM_SPEED;
		holdAngle = SensorValue[armAngle];
		armUpPower = 50;
		armDownPower = -30;
		armDownDecreased = false;
		armUpDecreased = false;
		armDownFound = false;
		armUpFound = false;
	}
	else if (autoHoldHeight){
		holdArm(0, currentSpeed);
	}
	else{
		motor[armRight] = motor[armLeft] = 0;
	}
}

void rollRoller( bool up, bool down ) {
	if ( up ) {
		motor[coneMotor] = ROLLER_SPEED;
	} else if ( down ) {
		motor[coneMotor] = -ROLLER_SPEED;
	} else {
		motor[coneMotor] = 0;
	}
}

void moveGoal( bool up, bool down ) {
	if ( up ) {
		motor[goalRight] = motor[goalLeft] = GOAL_SPEED;
	} else if ( down ) {
		motor[goalRight] = motor[goalLeft] = -GOAL_SPEED;
	} else {
		motor[goalRight] = motor[goalLeft] = 0;
	}
}
