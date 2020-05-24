#include "PID.h"// Include the file named PID.h

Pid::Pid(int ts, char td, int r, double kp, double ki, double kd)//Constructor
{
	TimeStep_ = ts;
	TimeDivisor_ = (td == 'm') ? 1000 : 100000;
	Kp_ = kp;
	Ki_ = ki;
	Kd_ = kd;
	Derivative_ = 0;
	Integral_ = 0;
	Current_Error_ = 0;
	Previous_Error_ = 0;
	Reference_Value_ = r;
}
		
//SETFUNCTIONS//
void Pid::setTimeStep(int time)
{
	TimeStep_ = time;
}
void Pid::setTimeDivisor(char d)
{
	TimeDivisor_ = d;
}
void Pid::setKp(double p)
{
	Kp_ = p;
}
void Pid::setKi(double i)
{
	Ki_ = i;
}
void Pid::setKd(double d)
{
	Kd_ = d;
}
void Pid::setReference(int r)
{
	Reference_Value_ = r;
}
//ENDSETFUNCTIONS
		
//GETFUNCTIONS//
int Pid::getTimeStep()
{
	return TimeStep_;
}
char Pid::getTimeDivisor()
{
	if(TimeDivisor_ == 1000)
	{
		return 'm';
	}
	else
	{
		return 'u';
	}
}
double Pid::getKp()
{
	return Kp_;
}
double Pid::getKi()
{
	return Ki_;
}
double Pid::getKd()
{
	return Kd_;
}
int Pid::getReference()
{
	return Reference_Value_;
}
int Pid::getError()
{
	return Current_Error_;
}
//ENDGETFUNCTIONS//
		
void Pid::SetInput(int i)// i is between 0 and 1023
{
	Previous_Error_ = Current_Error_;// The old error is rhe current one since it is about to be changed
	Current_Error_ = Reference_Value_- i;// Calculate the error between the input and the reference value
}
void Pid::CalcDerivative()
{
	Derivative_ = (double)(Current_Error_-Previous_Error_)/(double)(TimeStep_/TimeDivisor_);
}
void Pid::CalcIntegral()
{
	Integral_ += (double)Current_Error_*(double)(TimeStep_/TimeDivisor_);
}
double Pid::ControlValue()
{
	CalcDerivative();
	CalcIntegral();
	return Kp_*(double)Current_Error_;
}