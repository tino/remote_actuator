#ifndef Afro_h
#define Afro_h

#include "Arduino.h"


extern "C" {
// callback function types
    typedef void (*callbackFunction)(unsigned char command, unsigned char operand1, unsigned char operand2);
}


class Afro
{
public:
    Afro(void);
    void begin(unsigned char id, int debug);
    void begin(unsigned char id, int debug, long speed);
    void attach(unsigned char command, callbackFunction newFunction);
    void detach(unsigned char command);
    void processSerial(void);
    void sendRequest(unsigned char to, unsigned char command, unsigned char operand1, unsigned char operand2);
    void sendResponse(unsigned char to, unsigned char command, unsigned char res1, unsigned char res2);
    void sendDebug(char key[], int value);
    void sendDebug(char key[], int value, int debugLevel);
    void exerternalMonitor(char key[], int value);

private:
    int serialState;
    unsigned char _id;
    int _debug;
    callbackFunction cbA;
    callbackFunction cbB;
    callbackFunction cbC;
    callbackFunction cbD;
    callbackFunction cbE;
    callbackFunction cbF;
    callbackFunction cbG;
    callbackFunction cbH;
    callbackFunction cbI;
    callbackFunction cbJ;
    callbackFunction cbK;
    callbackFunction cbL;
    callbackFunction cbM;
    callbackFunction cbN;
    callbackFunction cbO;
    callbackFunction cbQ;
    callbackFunction cbR;
    callbackFunction cbS;
    callbackFunction cbT;
    callbackFunction cbU;
    callbackFunction cbV;
    callbackFunction cbW;
    callbackFunction cbX;
    callbackFunction cbY;
    callbackFunction cbZ;

};

#endif
