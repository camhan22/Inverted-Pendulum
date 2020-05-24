#include <PID.h>// Include the PID alogirthm

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
 * 
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
int Max_Sensor_Value = 784;// Max sensor value at system limits
int Min_Sensor_Value = 0;// Min sensor value at system limts
int Sample_Period = 100;// Time between samples
unsigned long Current_Time = 0;
unsigned long Last_Time = 0;
//END SYSTEM PARAMETERS//

//CONTROLLER PARAMETERS//
double Control_Signal = 0;
Pid Controller(100, 'm', (Max_Sensor_Value+Min_Sensor_Value)/2, 1);// Create the controller
//END CONTROLLER PARAMETERS//

//FUNCTION PROTOTYPES//
void SetSpeed(double cs);
void SetDirection(double cs);
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
}

void loop() {

  Current_Time = millis();
  if(Current_Time - Last_Time >= Sample_Period)
  {
    Controller.SetInput(analogRead(Angle_Sensor_Pin));// Read the sensor and put it into the controller
    Control_Signal = Controller.ControlValue();// Calculate the control value of the system
    Serial.println(Control_Signal);
    SetSpeed(Control_Signal);// Set the speed of the controller
    SetDirection(Control_Signal);// Set the dirction of the controller
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
  if(cs > 0)// If the signal is greater than zero, set the direction to clockwise
  {
    digitalWrite(IN3_Pin, HIGH);
    digitalWrite(IN4_Pin, LOW);
  }
  else// Otherwise counter-clockwise
  {
    digitalWrite(IN3_Pin, LOW);
    digitalWrite(IN4_Pin, HIGH);
  }
}
//END FUNCTIONS//
