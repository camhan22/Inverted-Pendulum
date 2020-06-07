/*
Class to run a PID controlled system

Author: Cameron Hanson
Creation Date: 05/13/2020
Last Updated: 05/13/2020
Version Number: 1

//REVISIONS//
//END REVISIONS//
*/
#ifndef PID// If this header file is not defined
#define PID// Declare it
#endif

class Pid
{
	private:
		int TimeStep_;
		double TimeDivisor_;
		double Kp_;
		double Ki_;
		double Kd_;
		double Derivative_;
		double Integral_;
		int Current_Error_;
		int Previous_Error_;
		int Reference_Value_;
	
	public:
		//Constructor
		Pid(int ts, char td, int r = 0 , double kp = 0, double ki = 0, double kd = 0);
		
		//SETFUNCTIONS//
		void setTimeStep(int time);
		void setTimeDivisor(char d);
		void setKp(double p);
		void setKi(double i);
		void setKd(double d);
		void setReference(int r);
		//ENDSETFUNCTIONS
		
		//GETFUNCTIONS//
		int getTimeStep();
		char getTimeDivisor();
		double getKp();
		double getKi();
		double getKd();
		int getReference();
		int getError();
		//ENDGETFUNCTIONS//
		
		void SetInput(int i);
		void CalcDerivative();
		void CalcIntegral();
		double ControlValue();	
};
