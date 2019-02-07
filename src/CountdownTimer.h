#ifndef CountdownTimer_h
#define CountdownTimer_h

class CountdownTimer 
{
  public:
    static void start();
    static void stop(); 
    static void reset();
    void countdown();
    bool hasUpdated();
    static const char* getClockText(); 
  private:
    //these are static as the countdown method is called from a separate timer instance
    static char clockText[6];
    static int decaminutes;
    static int minutes;
    static int decaseconds;
    static int seconds;
    static bool updated;
};
#endif