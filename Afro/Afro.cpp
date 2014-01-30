

#include "Afro.h"

Afro::Afro(void) {
  unsigned char _id;
  int _debug;
}

void Afro::begin(unsigned char id, int debug)
{
  begin(id, debug, 9600);
}

/* begin method for overriding default serial bitrate */
void Afro::begin(unsigned char id, int debug, long speed)
{
  _id = id;
  _debug = debug;
  Serial.begin(speed);
}

void Afro::attach(unsigned char command, callbackFunction newFunction)
{
  switch(command) {
    case 'A': cbA = newFunction; break;
    case 'B': cbB = newFunction; break;
    case 'C': cbC = newFunction; break;
    case 'D': cbD = newFunction; break;
    case 'E': cbE = newFunction; break;
    case 'F': cbF = newFunction; break;
    case 'G': cbG = newFunction; break;
    case 'H': cbH = newFunction; break;
    case 'I': cbI = newFunction; break;
    case 'J': cbJ = newFunction; break;
    case 'K': cbK = newFunction; break;
    case 'L': cbL = newFunction; break;
    case 'M': cbM = newFunction; break;
    case 'N': cbN = newFunction; break;
    case 'O': cbO = newFunction; break;
    case 'Q': cbQ = newFunction; break;
    case 'R': cbR = newFunction; break;
    case 'S': cbS = newFunction; break;
    case 'T': cbT = newFunction; break;
    case 'U': cbU = newFunction; break;
    case 'V': cbV = newFunction; break;
    case 'W': cbW = newFunction; break;
    case 'X': cbX = newFunction; break;
    case 'Y': cbY = newFunction; break;
    case 'Z': cbZ = newFunction; break;
  }
}

void Afro::detach(unsigned char command)
{
  attach(command, (callbackFunction)NULL);
}

void Afro::processSerial(void)
{
  if (!Serial.available()) return;

  static short int serialState = 0;
  static unsigned char from = 255;
  static unsigned char command = 255;
  static unsigned char operand1 = 255;
  static int operand2 = -1;

  char c = Serial.read();

  switch(serialState) {

    case 0:
      // A
      if (c == 'A')
        serialState = 1;
      else
        serialState = 0; // reset serialState
    break;

    case 1:
      // F
      if (c == 'F') {
        serialState = 2;
      }
      else if (c =='A') {
        serialState = 1; // stay right were you are
      }
      else {
        serialState = 0;
      }
    break;

    case 2:
      // Target/to
      // Is the message for us or a broadcast? If not ignore the rest
      if (c == _id || c == '0') {
        serialState = 3;
      }
      else {
        Serial.print("c");
        Serial.println(c);
        Serial.print("id:");
        Serial.println(_id);
        serialState = 0;
      }
    break;

    case 3:
      // From
      from = c;
      serialState = 4;
    break;

    case 4:
      command = c;
      serialState = 5;
    break;

    case 5: //read first operand in
      operand1 = c;
      serialState = 6;
    break;

    case 6: //read first byte of second operand
      operand2 = c;
      serialState = 7;
    break;

    case 7: //read second byte of second operand
      operand2 = word(c, operand2);
      serialState = 8;
    break;

    case 8:
      if (c == 'F')
        serialState = 9;
      else
        serialState = 0;
    break;

    case 9:
      if (c != 'A') {
        serialState = 0;
        return;
      }
      switch(command) {
        case 'A': (*cbA)(from, operand1, operand2); break;
        case 'B': (*cbB)(from, operand1, operand2); break;
        case 'C': (*cbC)(from, operand1, operand2); break;
        case 'D': (*cbD)(from, operand1, operand2); break;
        case 'E': (*cbE)(from, operand1, operand2); break;
        case 'F': (*cbF)(from, operand1, operand2); break;
        case 'G': (*cbG)(from, operand1, operand2); break;
        case 'H': (*cbH)(from, operand1, operand2); break;
        case 'I': (*cbI)(from, operand1, operand2); break;
        case 'J': (*cbJ)(from, operand1, operand2); break;
        case 'K': (*cbK)(from, operand1, operand2); break;
        case 'L': (*cbL)(from, operand1, operand2); break;
        case 'M': (*cbM)(from, operand1, operand2); break;
        case 'N': (*cbN)(from, operand1, operand2); break;
        case 'O': (*cbO)(from, operand1, operand2); break;
        case 'Q': (*cbQ)(from, operand1, operand2); break;
        case 'R': (*cbR)(from, operand1, operand2); break;
        case 'S': (*cbS)(from, operand1, operand2); break;
        case 'T': (*cbT)(from, operand1, operand2); break;
        case 'U': (*cbU)(from, operand1, operand2); break;
        case 'V': (*cbV)(from, operand1, operand2); break;
        case 'W': (*cbW)(from, operand1, operand2); break;
        case 'X': (*cbX)(from, operand1, operand2); break;
        case 'Y': (*cbY)(from, operand1, operand2); break;
        case 'Z': (*cbZ)(from, operand1, operand2); break;
      }
      serialState = 0;
    break;

    default:
    break;

  }
}

void Afro::sendRequest(unsigned char to, unsigned char command, unsigned char operand1, unsigned char operand2)
{
  // Send out a message. to is the ID of the targeted machine
  // operand1 cannot be greater than 255
  // operand2 will be send as a MSB and LSB
  Serial.print("AF");
  Serial.write(to);
  Serial.write(_id);
  Serial.write(command);
  Serial.write(operand1);
  Serial.write(highByte(operand2));
  Serial.write(lowByte(operand2));
  Serial.print("FA");
}

void Afro::sendResponse(unsigned char to, unsigned char command, unsigned char res1, unsigned char res2)
{
  // Reply to a message. to is the ID of the targeted machine
  Serial.print("SR");
  Serial.write(to);
  Serial.write(_id);
  Serial.write(command);
  Serial.write(res1);           // primary, one byte
  Serial.write(highByte(res2)); // secondary result, two bytes
  Serial.write(lowByte(res2));
  Serial.print("RS");
}

void Afro::sendDebug(char key[], int value) {
  // Debug with level 1
  sendDebug(key, value, 1);
}
void Afro::sendDebug(char key[], int value, int debugLevel) {
  int DEBUG_STREAM = 0;
  int DEBUG_LEVEL = 0;
  int debugStream = 0;

  if (_debug > 10) {
    DEBUG_STREAM = _debug / 10;
    DEBUG_LEVEL = _debug % 10;
  } else {
    DEBUG_LEVEL = _debug;
  }

  if (debugLevel > 10) {
    debugStream = debugLevel / 10;
    debugLevel = debugLevel % 10;
  }

  if (debugLevel > DEBUG_LEVEL) return;
  if (debugStream == 0 || debugStream == DEBUG_STREAM) {
    Serial.print("<");
    Serial.print(_id);
    Serial.print(":");
    Serial.print(key);
    Serial.print(":");
    Serial.print(value);
    Serial.print(":");
    Serial.print(millis());
    Serial.println(">");
  }
}

void Afro::exerternalMonitor(char key[], int value) {
  Serial.print("<<");
  Serial.print(_id);
  Serial.print(":");
  Serial.print(key);
  Serial.print(":");
  Serial.print(value);
  Serial.print(":");
  Serial.print(millis());
  Serial.println(">>");
}
