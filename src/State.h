#ifndef State_h
#define State_h
#include "application.h"

struct SystemState{
  int boilPower; 
  double hltSetpoint; 
  double mashSetpoint; 
  int pump1Power;
  int pump2Power;

  double mashTemp;
  double boilTemp;
  double coilTemp;
  double hltTemp;
  bool boilElementOn;
  bool hltElementOn;
  char countdownTime[6];
  bool timerStarted;

  double boilSetpoint;
  int hltPower;
  int boilMode; // 0=power, 1=setpoint
  int hltMode; // 0=power, 1=hlt setpoint, 2=mash setpoint
  int cloudStatus; //0=disconnected, 1=connected
  int meshStatus;  //0=disconnected, 1=connected
  double batteryVoltage;
};

#endif