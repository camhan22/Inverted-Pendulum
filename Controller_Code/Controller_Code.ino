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
int PWM_Pin = 9;
int IN3_Pin = 2;
int IN4_Pin = 3;
int Power_Pin = 13;// Pin used to provide power to the H-Bridge for convience
int Angle_Sensor_Pin = A5;
//END PIN DEFINITIONS//

//SYSTEM PARAMETERS//
int Max_Sensor_Value = 1023;// Max sensor value at system limits
int Min_Sensor_Value = 0;// Min sensor value at system limts
int Sample_Period = 100;// Time between samples
unsigned long Current_Time = 0;
unsigned long Last_Time = 0;
//END SYSTEM PARAMETERS//

//CONTROLLER PARAMETERS//
double Control_Signal = 75;
int Current_Value;
int Previous_Value;// Holds the previous value of the sensor during the swing
bool is_Controlled = true;// Variable to hold whether the system should swing or control the arm
bool is_Updated = false;// Tells if the swing direction and speed need to be updated
//By default it should swing since it will be at the bottom of the arc
int Swing_Limit = 75;// Variable to hold the value at which the controller will switch to controlled mode
int Swing_Rate = 10;// Dictates how fast the system will get to the controlling point
Pid Controller(100, 'm', 517, 0.74);// Create the controller
//END CONTROLLER PARAMETERS//

//FUNCTION PROTOTYPES//
void SetSpeed(double cs);
void SetDirection(double cs);
double MapDouble(int x, int in_min, int in_max, int out_min, int out_max);
//END FUNCTION PROTOTYPES//

void setup() {
  //Setup the I/O pins
  pinMode(Power_Pin, OUTPUT);
  digitalWrite(Power_Pin, HIGH);// Set the pin to output 5v for H_Bridge
  pinMode(PWM_Pin, OUTPUT);
  pinMode(Angle_Sensor_Pin, INPUT);
  pinMode(IN3_Pin, OUTPUT);
  pinMode(IN4_Pin, OUTPUT);
  Serial.begin(9600);// Start serial communication for debugging
  
  while(!is_Controlled)// While the system swings
  {
	  Current_Time = millis();
	  if(Current_Time - Last_Time >= Sample_Period)// Once we have elapsed the sample period, we need a new value
	  {
      Current_Value = analogRead(Angle_Sensor_Pin);
		  //CHECK DIRECTION AND SEE IF UPDATE IS NEEDED//
		  if(Control_Signal > 0){// If we are rotating in the clockwise direction
			  if(Current_Value - Previous_Value <= 0 && Current_Value < (Max_Sensor_Value + Min_Sensor_Value)/2 && abs(Current_Value-Previous_Value) < 100){
				  // If the new value is less than the old value which can only happen when the direction reverses due to gravity, reverse the direction.
				  //Also check to see that the difference is not greater than 200 which happens at the bottom of the swing due to crossover
				  is_Updated = false;
			  }
		  }
		  else{// Counter-clockwise direction
			  if(Current_Value - Previous_Value >= 0 && Current_Value > (Max_Sensor_Value + Min_Sensor_Value)/2 && abs(Current_Value-Previous_Value) < 100){
				  is_Updated = false;// We have given it new commands and we shoudl do them
			  }
		  }
		  //END CHECK DIRECTION AND SEE IF UPDATE IS NEEDED//

      if(millis()-Last_Time > 500){// Only update the output only twice a second to avoid large torques applied to belt
		    //UPDATE THE SWING//
		    if(!is_Updated){// Only do this if we need to change direction
			    Control_Signal+= (Control_Signal > 0) ? Swing_Rate : -1*Swing_Rate;
			    // If the value of C_S is above zero, we add swing rate, if less than zero, we subtract. This ensures that we are increasing the power each time
			    Control_Signal *= -1;// Reverse swing direction
			    SetDirection(Control_Signal);
			    SetSpeed(Control_Signal);
			    is_Updated = true;// Tell the controller the state has been updated
		    }
		    //END UPDATE THE SWING//
		
		    //DO THESE EVERY SAMPLE//
        Last_Time = Current_Time;
		    Previous_Value = Current_Value;// Update the previous value
		  }
	  }

    int Value = analogRead(Angle_Sensor_Pin);
    Serial.println(Value);
    // Check this every time through the loop, we do not want to miss this 
    if (Value > (Max_Sensor_Value + Min_Sensor_Value)/2-Swing_Limit && Value < (Max_Sensor_Value + Min_Sensor_Value)/2+Swing_Limit){// Check to see if it is within the limit for PID to take over
    is_Controlled = true;// If so, then we will allow the PID to take over
    SetSpeed(0);// Stop the motor
    break;// And we need to exit the loop
	  }
  //If we have not gotten a sample, continue to do the same thing. i.e. change nothing
  }
}

void loop() {

  Current_Time = millis();  
  if(Current_Time - Last_Time >= Sample_Period)
  {
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
  if(cs > 0)
  {
    digitalWrite(IN3_Pin, LOW);
    digitalWrite(IN4_Pin, HIGH);
  }
  else
  {
    digitalWrite(IN3_Pin, HIGH);
    digitalWrite(IN4_Pin, LOW);
  }
}

double MapDouble(int x, int in_min, int in_max, int out_min, int out_max){
 return ((double)x - (double)in_min) * ((double)out_max - (double)out_min) / ((double)in_max - (double)in_min) + (double)out_min;
 }
//END FUNCTIONS//
