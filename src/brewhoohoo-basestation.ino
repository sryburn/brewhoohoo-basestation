#include "JsonParserGeneratorRK.h"

  int boilPower = 0;
  double boilSetpoint = 0;
  double mashSetpoint = 0;
  int hltPower = 0;
  double hltSetpoint = 0;
  int pump1Power = 0;
  int pump2Power = 0;
  double mashTemp = 0;
  double boilTemp = 0;
  double coilTemp = 0;
  double hltTemp = 0;
  bool boilElementOn = false;
  bool hltElementOn = false;
  int boilMode = 0; // 0=power, 1=setpoint
  int hltMode = 1; // 0=power, 1=hlt setpoint, 2=mash setpoint
  char countdownTime[5];
  int timerState = 0; // 0=paused, 1=started
  int cloudStatus = 1; //0=disconnected, 1=connected
  int meshStatus = 1; //0=disconnected, 1=connected
  int batteryVoltage = 0;

int led = D7;
int pump1 = WKP;
int pump2 = D0;
int boilElement = A2;
int hltElement = A1;

void setup() {
  Mesh.subscribe("set", setValue);
  setPinModes();
}

void loop() {

}

void setPinModes(){
    pinMode(pump1, OUTPUT);
    pinMode(pump2, OUTPUT);
    pinMode(boilElement, OUTPUT);
    pinMode(hltElement, OUTPUT);
    pinMode(led, OUTPUT);
}

void setValue(const char *event, const char *data){
  if (strcmp(event, "setBoilPower") == 0){
    boilPower = atoi(data);
  }
  if (strcmp(event, "setBoilSetpoint") == 0){
    boilSetpoint = atoi(data);
  }
  if (strcmp(event, "setMashSetpoint") == 0){
    mashSetpoint = atof(data);
    hltSetpoint = -1;
  }
  if (strcmp(event, "setHltPower") == 0){
    hltPower = atoi(data);
    mashSetpoint = -1;
  }
  if (strcmp(event, "setHltSetpoint") == 0){
    hltSetpoint = atof(data);
    mashSetpoint = -1;
  }
  if (strcmp(event, "setPump1Power") == 0){
    pump1Power = atoi(data);
  }
  if (strcmp(event, "setPump2Power") == 0){
    pump2Power = atoi(data);
  }

  publishJson();
}

void publishJson(){
  JsonWriterStatic<622> jsonBuffer;
  {
  JsonWriterAutoObject obj(&jsonBuffer);
  jsonBuffer.insertKeyValue("boilPower", boilPower);
  jsonBuffer.insertKeyValue("boilSetpoint", boilSetpoint);
  jsonBuffer.insertKeyValue("mashSetpoint", mashSetpoint);
  jsonBuffer.insertKeyValue("hltPower", hltPower);
  jsonBuffer.insertKeyValue("hltSetpoint", hltSetpoint);
  jsonBuffer.insertKeyValue("pump1Power", pump1Power);
  jsonBuffer.insertKeyValue("pump2Power", pump2Power);
  jsonBuffer.insertKeyValue("mashTemp", mashTemp);
  jsonBuffer.insertKeyValue("boilTemp", boilTemp);
  jsonBuffer.insertKeyValue("coilTemp", coilTemp);
  jsonBuffer.insertKeyValue("hltTemp", hltTemp);
  jsonBuffer.insertKeyValue("boilElementOn", boilElementOn);
  jsonBuffer.insertKeyValue("hltElementOn", hltElementOn);
  jsonBuffer.insertKeyValue("boilMode", boilMode);
  jsonBuffer.insertKeyValue("hltMode", hltMode);
  jsonBuffer.insertKeyValue("countdownTime", countdownTime);
  jsonBuffer.insertKeyValue("timerState", timerState);
  jsonBuffer.insertKeyValue("cloudStatus", cloudStatus);
  jsonBuffer.insertKeyValue("meshStatus", meshStatus);
  jsonBuffer.insertKeyValue("batteryVoltage", batteryVoltage);
  }

  Mesh.publish("status", jsonBuffer.getBuffer());
  Particle.publish("status", jsonBuffer.getBuffer(), PRIVATE);
}