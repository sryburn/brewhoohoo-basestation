#include "JsonParserGeneratorRK.h"
#include "OneWire.h"
#include "spark-dallas-temperature.h"
#include "CountdownTimer.h"
#include "State.h"

#define testing //uncomment for test board

SYSTEM_THREAD(ENABLED);

Timer cloudPublish(6000, timeToPublishCloud);

unsigned long lastTempRequest = 0;
const int sensorDelay = 750;

DallasTemperature sensors(new OneWire(A0)); //alternative version
CountdownTimer countdownTimer;
SystemState state;

bool cloudPublishFlag = false;
bool clockChanged = false;

const int led = D7;
const int pump1 = D12;
const int pump2 = D0;
const int boilElement = A2;
const int hltElement = A1;

volatile double hltDriveSet = 0; //has mashing offset added if necessesary

void setup() {
  sensors.begin();
  sensors.setWaitForConversion(false);
  waitUntil(Mesh.ready);
  Mesh.subscribe("set", setValue);
  setPinModes();
  countdownTimer.reset();
  driveBoil();
  sensors.requestTemperatures();
  lastTempRequest = millis(); 
  state.hltMode = 1;
}

void loop() {
  getTemperatures();
  drivePumps();
  driveBoil();
  addHltOffset();
  driveHLT();

  if (countdownTimer.hasUpdated()==true){
    strcpy(state.countdownTime, countdownTimer.getClockText());
    clockChanged = true;
  }

  publishMesh();
}

void setPinModes(){
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(boilElement, OUTPUT);
  pinMode(hltElement, OUTPUT);
  pinMode(led, OUTPUT);
}

void getTemperatures(){
  #ifdef testing
    static DeviceAddress mashSensor = {0x28, 0xFF, 0x5E, 0xB6, 0x0, 0x17, 0x4, 0x4};
    static DeviceAddress coilSensor = {0x28, 0xFF, 0x63, 0xA8, 0x0, 0x17, 0x5, 0x1F};
    static DeviceAddress hltSensor = {0x28, 0x5B, 0xC4, 0xAC, 0x9, 0x0, 0x0, 0xC0};
    static DeviceAddress boilSensor = {0x28, 0xFF, 0xA8, 0xAF, 0x0, 0x17, 0x4, 0x95};
  #else
    static DeviceAddress mashSensor = {0x28, 0xB7, 0x3A, 0x74, 0x6, 0x0, 0x0, 0xD2};
    static DeviceAddress coilSensor = {0x28, 0xB8, 0x4E, 0x74, 0x6, 0x0, 0x0, 0x84};
    static DeviceAddress hltSensor = {0x28, 0xE7, 0xC, 0x74, 0x6, 0x0, 0x0, 0xE4};
    static DeviceAddress boilSensor = {0x28, 0xB3, 0xB5, 0x73, 0x6, 0x0, 0x0, 0x2C};
  #endif
  
  if (millis() - lastTempRequest >= sensorDelay){
    saveTemperature(boilSensor, state.boilTemp);
    saveTemperature(mashSensor, state.mashTemp);
    saveTemperature(hltSensor, state.hltTemp);
    saveTemperature(coilSensor, state.coilTemp);
    sensors.requestTemperatures(); 
    lastTempRequest = millis(); 
  }
}

void saveTemperature(uint8_t* device, double &value){
    double temp = sensors.getTempC(device);
    if ( temp > 0 && temp < 120){
        value = round(temp*10)/10;
    }
}

void drivePumps(){
  analogWrite(pump1, state.pump1Power*2.55, 65000);
  analogWrite(pump2, state.pump2Power*2.55, 65000);
}


void driveBoil(){
// 10 second Time Proportional Output window
  static unsigned long WindowSize = 10000;
  static unsigned long windowStartTime;
  static volatile unsigned long boilOnTime;
  boilOnTime = state.boilPower * 100;
  unsigned long now = millis();
  if(now - windowStartTime>WindowSize){ //time to shift the Relay Window
    windowStartTime += WindowSize;
  }

  if((boilOnTime > 100) && (boilOnTime > (now - windowStartTime))){
    if ((digitalRead(boilElement) == LOW) && (digitalRead(hltElement) == LOW)) {
      digitalWrite(boilElement,HIGH);
      state.boilElementOn=true;
    }
  }
  else if (digitalRead(boilElement) == HIGH){
    digitalWrite(boilElement,LOW);
    state.boilElementOn=false;
  }
}

void addHltOffset(){
    static int minOffset = 2;
    static int maxOffset = 6;
    static int offset;

    if ((state.hltMode == 2) && (state.mashTemp<=state.hltSetpoint)){
        int difference = (state.hltSetpoint-state.mashTemp);
        if (difference>=minOffset){
          offset = min(maxOffset, difference);
        } else {
          offset = max(minOffset, difference);
        }
        hltDriveSet = (state.hltSetpoint + offset);
    } else {
      hltDriveSet = state.hltSetpoint;
    }
}

void driveHLT(){
    if (state.hltTemp < hltDriveSet - 0.2){
        digitalWrite(boilElement,LOW);
        state.boilElementOn = false;
        // display.renderElementIndicator(false, 1);
        digitalWrite(hltElement, HIGH);
        state.hltElementOn = true;
        // display.renderElementIndicator(true, 2);
    } else if (state.hltTemp > hltDriveSet + 0.2){
        digitalWrite(hltElement, LOW);
        state.hltElementOn = false;
        // display.renderElementIndicator(false, 2);
    }
}


void timeToPublishCloud(){
  cloudPublishFlag = true;
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
}

void publishMesh(){

  static SystemState prevState;

  if (state.boilPower != prevState.boilPower ||
  state.hltSetpoint != prevState.hltSetpoint ||
  state.mashSetpoint != prevState.mashSetpoint ||
  state.pump1Power != prevState.pump1Power ||
  state.pump2Power != prevState.pump2Power ||
  state.mashTemp != prevState.mashTemp ||
  state.boilTemp != prevState.boilTemp ||
  state.coilTemp != prevState.coilTemp ||
  state.hltTemp != prevState.hltTemp ||
  state.boilElementOn != prevState.boilElementOn ||
  state.hltElementOn != prevState.hltElementOn ||
  clockChanged ||
  state.timerStarted != prevState.timerStarted ||
  state.boilSetpoint != prevState.boilSetpoint ||
  state.hltPower != prevState.hltPower ||
  state.boilMode != prevState.boilMode ||
  state.hltMode != prevState.hltMode ||
  state.cloudStatus != prevState.cloudStatus ||
  state.meshStatus != prevState.meshStatus ||
  state.batteryVoltage != prevState.batteryVoltage){

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
    prevState = state;
    clockChanged = false;
  }
}