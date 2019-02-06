#include "application.h"
#include "CountdownTimer.h"

CountdownTimer callback;
Timer countdownClock(1000, &CountdownTimer::countdown, callback);

bool CountdownTimer::updated = false;
char CountdownTimer::clockText[20] = "";
int CountdownTimer::decaminutes = 6;
int CountdownTimer::minutes = 0;
int CountdownTimer::decaseconds = 0;
int CountdownTimer::seconds = 0;


//Timer Functions
void CountdownTimer::countdown(){
    // if (decaminutes==0&&minutes==0&&decaseconds==0&&seconds==0){
    //     countdown.setText("ALARM");
    // } else{
        if (seconds>0){
            seconds--;
        } else if (decaseconds>0){
            decaseconds--;
            seconds=9;
        } else if (minutes>0){
            minutes--;
            decaseconds=5;
            seconds=9;
        } else if (decaminutes>0){
            decaminutes--;
            minutes=9;
            decaseconds=5;
            seconds=9;
        }
    snprintf(clockText, sizeof(clockText), "%d%d:%d%d", decaminutes, minutes, decaseconds, seconds);
    updated = true;
    // }
}

void CountdownTimer::start(){
  countdownClock.start();
}

void CountdownTimer::stop(){
  countdownClock.stop();
}

void CountdownTimer::reset(){
  decaminutes = 6;
  minutes = 0;
  decaseconds = 0;
  seconds = 0;
  snprintf(clockText, sizeof(clockText), "%d%d:%d%d", decaminutes, minutes, decaseconds, seconds);
  updated = true;
}

bool CountdownTimer::hasUpdated(){
  if (updated){
    updated = false;
    return true;
  } else {
    return false;
  }
}

const char* CountdownTimer::getClockText()
{
  return clockText;
}