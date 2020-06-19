# Inverted-Pendulum
PID Controlled Inverted Pendulum

This is a inverted pendulum system that uses a digital PID controller

The PID.cpp, PID.h, and Keywords.txt files are a stand alone digital PID controller that can be put into the libraries folder and has variable time stepping meaning both micros() and millis() can be used with it. The input to the system can be any number but inputs from the standard analog read pins have been validated. Remember that the arduino can only do PWM with a max value of 255; Meaning the control signal should be checked to see if it is greater than the maximum output.

The STL files will be uploaded shortly as well as pictures of the completed project
