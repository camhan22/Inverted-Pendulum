#include "PID.h"// Include the PID alogirthm

/*
 * Author:Cameron Hanson
 * Creation Date: 05/13/2020
 * Last Updated: 05/13/2020
 * Revision: 1.0
 * 
 * //REVISION HISTORY//
 * 
 * //END REVISION HISTORY//
 * 
 * //TO DO
 * //END TO DO
 */

//PIN DEFINITIONS//
int PWM_Pin = 10;
int IN3_Pin = 2;
int IN4_Pin = 3;
int Power_Pin = 13;// Pin used to provide power to the H-Bridge for convience
int Power_Pin_Pot = 4;// Pin used to give 5V to potentiometer
int Pot_Ground = 5;
int Mode_Pin_1 = 7;//Pin to determine the Mode the stem is is (Red wire)
int Mode_Pin_2 = 8;//Pin to determine what Mode the system is in (Blue Wire)
int Angle_Sensor_Pin = A5;
int Pot_Pin = A4;
int Track_Pin = A3;
//END PIN DEFINITIONS//

//SYSTEM PARAMETERS//
int Max_Sensor_Value = 1023;// Max sensor value at system limits
int Min_Sensor_Value = 0;// Min sensor value at system limts
int Sample_Period = 10000;// Time between samples
unsigned long Current_Time = 0;
unsigned long Last_Time = 0;
int Mode = 0;
//END SYSTEM PARAMETERS//

//CONTROLLER PARAMETERS//
double Control_Signal = 40;
int Current_Value;
int Reference = 508;
int Previous_Value;// Holds the previous value of the sensor during the swing
bool is_Controlled = false;// Variable to hold whether the system should swing or control the arm
bool is_Updated = false;// Tells if the swing direction and speed need to be updated
//By default it should swing since it will be at the bottom of the arc
int Swing_Limit = 15;// Variable to hold the value at which the controller will switch to controlled Mode
int Swing_Rate = 5;// Dictates how fast the system will get to the controlling point
long Swing_Update_Count = 0;
long Swing_Update_Rate = 50;// How fast the system will respond (Only affects the outputs, the inputs will run at the sample period)
Pid Controller(Sample_Period, 'u', Reference, 2, 5.29 ,0.125);// Create the controller
//END CONTROLLER PARAMETERS//

//FUNCTION PROTOTYPES//
void SetSpeed(double cs);
void SetDirection(double cs);
double MapDouble(int x, int in_min, int in_max, int out_min, int out_max);
bool ModeChanged();
void(* resetFunc) (void) = 0;// Needed to reset the arduino by software
//END FUNCTION PROTOTYPES//

void setup() {
  delay(2000);
  Serial.begin(115200);
  //Setup the I/O pins
  pinMode(Power_Pin, OUTPUT);
  digitalWrite(Power_Pin, HIGH);// Set the pin to output 5v for H_Bridge
  pinMode(Power_Pin_Pot, OUTPUT);
  digitalWrite(Power_Pin_Pot, HIGH);// Set the pin to output 5v for Potentiometer
  pinMode(Pot_Ground, OUTPUT);
  digitalWrite(Pot_Ground, LOW);// Set the pin to output 5v for Potentiometer
  
  pinMode(PWM_Pin, OUTPUT);
  pinMode(Angle_Sensor_Pin, INPUT);
  pinMode(IN3_Pin, OUTPUT);
  pinMode(IN4_Pin, OUTPUT);

  pinMode(Mode_Pin_1,INPUT_PULLUP);
  pinMode(Mode_Pin_2,INPUT_PULLUP);

  //READ AND SELECT MODE//
  if(!digitalRead(Mode_Pin_1) && !digitalRead(Mode_Pin_2))
  {
    Mode = 1;// Follows the potentiometer reading
  }
  else if(!digitalRead(Mode_Pin_1) && digitalRead(Mode_Pin_2))
  {
    Mode = 0;// Goes to the pre-defined reference value stored
  }
  else
  {
    Mode = 2;// Follows an external voltage source
  }
  //READ AND SELECT MODE//

  //REFERENCE SETUP BASED ON Mode//
  switch (Mode)
  {
    case 1:
      Controller.setReference(map(analogRead(Pot_Pin),0,1023,1023,0));
      break;

    case 2:
      Controller.setReference(analogRead(Track_Pin));
      break;
      
    default:
      break;
  }
  //END REFERENCE SETUP BASED ON Mode//

  //START-UP SWING//
  while(!is_Controlled)// While the system swings
  {
	  Current_Time = micros();
	  if(Current_Time - Last_Time >= Sample_Period)// Once we have elapsed the sample period, we need a new value
	  {
      Current_Value = analogRead(Angle_Sensor_Pin);
		  //CHECK DIRECTION AND SEE IF UPDATE IS NEEDED//
		  if(Control_Signal > 0){// If we are rotating in the clockwise direction
			  if(Current_Value - Previous_Value <= 0 && Current_Value < (Max_Sensor_Value + Min_Sensor_Value)/2 && abs(Current_Value-Previous_Value) < 10){
				  // If the new value is less than the old value which can only happen when the direction reverses due to gravity, reverse the direction.
				  //Also check to see that the difference is not greater than 200 which happens at the bottom of the swing due to crossover
				  is_Updated = false;
			  }
		  }
		  else{// Counter-clockwise direction
			  if(Current_Value - Previous_Value >= 0 && Current_Value > (Max_Sensor_Value + Min_Sensor_Value)/2 && abs(Current_Value-Previous_Value) < 10){
				  is_Updated = false;// We have given it new commands and we shoudl do them
			  }
		  }
      Swing_Update_Count++;// Increment the number of cycles that have passed. Once they get beyond
      if(Swing_Update_Count >= Swing_Update_Rate){// Only update the output only twice a second to avoid large torques applied to belt
		    //UPDATE THE SWING//
		    if(!is_Updated){// Only do this if we need to change direction
			    Control_Signal+= (Control_Signal > 0) ? Swing_Rate : -1*Swing_Rate;
			    // If the value of C_S is above zero, we add swing rate, if less than zero, we subtract. This ensures that we are increasing the power each time
			    Control_Signal *= -1;// Reverse swing direction
			    SetDirection(Control_Signal);
			    SetSpeed(Control_Signal);
			    is_Updated = true;// Tell the controller the state has been updated
		    }
         Swing_Update_Count = 0;//Reset the count
		  }
      //DO THESE EVERY SAMPLE//
      Last_Time = Current_Time;
      Previous_Value = Current_Value;// Update the previous value
      if (Current_Value > Controller.getReference()-Swing_Limit && Current_Value < Controller.getReference()+Swing_Limit){// Check to see if it is within the limit for PID to take over
        is_Controlled = true;// If so, then we will allow the PID to take over
        SetSpeed(0);// Stop the motor
        break;// And we need to exit the loop
      }
	  }
  }
  //END START-UP SWING//
  Serial.println("Controlled");
}

void loop() {
  Current_Time = micros();  
  if(Current_Time - Last_Time >= Sample_Period)
  {
    if(ModeChanged())// Check if the Mode changed, if so, reset the arduino
    {
      SetSpeed(0);//Turn the PWM off to allow the arm to fall
      resetFunc();// Reset the Arduino
    }
    
    switch (Mode)
    {
      case 1:
        Controller.setReference(map(analogRead(Pot_Pin),0,1023,1023,0));
        break;
      case 2:
        Controller.setReference(analogRead(Track_Pin));
        break; 
      default:// Don't do anything
        break;
    }
    Controller.SetInput(analogRead(Angle_Sensor_Pin));// Read the sensor and put it into the controller
    Control_Signal = Controller.ControlValue();// Calculate the control value of the system
    SetSpeed(Control_Signal);// Set the speed of the controller
    SetDirection(Control_Signal);// Set the dirction of the controller
	  Last_Time = Current_Time;// Update the last time the loop exectuted
  }
  //If we have not gotten a sample, continue to do the same thing. i.e. change nothing
}

//FUNCTIONS//
void SetSpeed(double cs)
{
  analogWrite(PWM_Pin, (abs(cs)>255 ? 255 : abs(cs)));// Output a PWM signal that corresponds to the strength of the error. If the error is large, we set the maximum power the system has
  //otherwise, we set it to the control value. This removes complexity by not having to figure out the max signal values.
}
void SetDirection(double cs)
{
  if(cs > 0)// Turn Clockwise
  {
    digitalWrite(IN3_Pin, LOW);
    digitalWrite(IN4_Pin, HIGH);
  }
  else// Turn Counter-Clockwise
  {
    digitalWrite(IN3_Pin, HIGH);
    digitalWrite(IN4_Pin, LOW);
  }
}

double MapDouble(int x, int in_min, int in_max, int out_min, int out_max){
 return ((double)x - (double)in_min) * ((double)out_max - (double)out_min) / ((double)in_max - (double)in_min) + (double)out_min;
 }
bool ModeChanged()
{
  int currentmode = Mode;
  if(!digitalRead(Mode_Pin_1) && !digitalRead(Mode_Pin_2))
  {
    Mode = 1;// Follows the potentiometer reading
  }
  else if(!digitalRead(Mode_Pin_1) && digitalRead(Mode_Pin_2))
  {
    Mode = 0;// Goes to the pre-defined reference value stored
  }
  else
  {
    Mode = 2;// Follows an external voltage source
  }
  if(currentmode == Mode)// If the modes are the same, it hasn't changed
  {
    return false;
  }
  else// Otherwise they have
  {
    return true;
  }
}
//END FUNCTIONS//
