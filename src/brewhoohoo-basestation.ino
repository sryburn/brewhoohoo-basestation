#include "JsonParserGeneratorRK.h"
#include "CountdownTimer.h"
#include "State.h"

CountdownTimer countdownTimer;
SystemState state;

void setup() {
  Mesh.subscribe("set", setValue);
  setPinModes();
  countdownTimer.reset();
}

void loop() {
  //TODO: read temperature probes

  if (countdownTimer.hasUpdated()==true){
    strcpy(state.countdownTime, countdownTimer.getClockText());
    // state.countdownTime = countdownTimer.getClockText();
    publishJson();
  }
}

void setPinModes(){
  int led = D7;
  int pump1 = WKP;
  int pump2 = D0;
  int boilElement = A2;
  int hltElement = A1;
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(boilElement, OUTPUT);
  pinMode(hltElement, OUTPUT);
  pinMode(led, OUTPUT);
}

void setValue(const char *event, const char *data){
  if (strcmp(event, "setBoilPower") == 0){
    state.boilPower = atoi(data);
  }
  if (strcmp(event, "setBoilSetpoint") == 0){
    state.boilSetpoint = atoi(data);
  }
  if (strcmp(event, "setMashSetpoint") == 0){
    state.mashSetpoint = atof(data);
    state.hltSetpoint = -1;
  }
  if (strcmp(event, "setHltPower") == 0){
    state.hltPower = atoi(data);
    state.mashSetpoint = -1;
  }
  if (strcmp(event, "setHltSetpoint") == 0){
    state.hltSetpoint = atof(data);
    state.mashSetpoint = -1;
  }
  if (strcmp(event, "setPump1Power") == 0){
    state.pump1Power = atoi(data);
  }
  if (strcmp(event, "setPump2Power") == 0){
    state.pump2Power = atoi(data);
  }
  if (strcmp(event, "setTimerStart") == 0){
    countdownTimer.start();
    state.timerStarted = true;
  }
  if (strcmp(event, "setTimerStop") == 0){
    countdownTimer.stop();
    state.timerStarted = false;
  }
  if (strcmp(event, "setTimerReset") == 0){
    countdownTimer.reset();
  }

  publishJson();
}

void publishJson(){
  JsonWriterStatic<622> jsonBuffer;
  {
  JsonWriterAutoObject obj(&jsonBuffer);
  jsonBuffer.insertKeyValue("countdownTime", state.countdownTime);
  jsonBuffer.insertKeyValue("boilPower", state.boilPower);
  jsonBuffer.insertKeyValue("boilSetpoint", state.boilSetpoint);
  jsonBuffer.insertKeyValue("mashSetpoint", state.mashSetpoint);
  jsonBuffer.insertKeyValue("hltPower", state.hltPower);
  jsonBuffer.insertKeyValue("hltSetpoint", state.hltSetpoint);
  jsonBuffer.insertKeyValue("pump1Power", state.pump1Power);
  jsonBuffer.insertKeyValue("pump2Power", state.pump2Power);
  jsonBuffer.insertKeyValue("mashTemp", state.mashTemp);
  jsonBuffer.insertKeyValue("boilTemp", state.boilTemp);
  jsonBuffer.insertKeyValue("coilTemp", state.coilTemp);
  jsonBuffer.insertKeyValue("hltTemp", state.hltTemp);
  jsonBuffer.insertKeyValue("boilElementOn", state.boilElementOn);
  jsonBuffer.insertKeyValue("hltElementOn", state.hltElementOn);
  jsonBuffer.insertKeyValue("timerStarted", state.timerStarted);
  jsonBuffer.insertKeyValue("boilMode", state.boilMode);
  jsonBuffer.insertKeyValue("hltMode", state.hltMode);
  jsonBuffer.insertKeyValue("cloudStatus", state.cloudStatus);
  jsonBuffer.insertKeyValue("meshStatus", state.meshStatus);
  jsonBuffer.insertKeyValue("batteryVoltage", state.batteryVoltage);
  }

  Mesh.publish("status", jsonBuffer.getBuffer());
  Particle.publish("status", jsonBuffer.getBuffer(), PRIVATE);
}