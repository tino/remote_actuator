#define ID 'R'
// ^ Device ID
// Letters A-Z can be used. 0 is reserved for broadcast

#include <Afro.h>

// Constants and global vars
// ----------

// enable to show debugging information about parsing and operations
// not so good on broadcast channels. Not a constant so we can switch it live
// 0 = off
// 1 = lowest debug level
// the higher the number, the more verbose the logging becomes
// double digit debug levels can be used for specific types, so setting debug
// to 23 will show everything of level 1 + everything 21 - 23
int DEBUG = 1;  // FIXME: devices behave erronous when starting with DEBUG = 0...

#define REMOTE 'A'

// Pins
const int POSITION_PIN = 1;
const int CONNECTION_PIN = 2;
const int MOVING_PIN = 3;
const int END_STOP_IN_PIN = 4;
const int END_STOP_OUT_PIN = 5;

// Messaging commands
#define MOVE_TO         'M' // Move the actuator to a designated position
#define STOP            'S' // Stop the actuator
#define AT_END          'E' // Signal actuator is at an end (0 = in, 1 = out)
#define PING            'P' // Signal we actuator is live
#define MOVING          'N' // Signal wether actuator is moving

// Times
const int HZ = 20;
const int PING_TIME = 2 * 1000;

// Measurement values
const float MAX_POSITION = 3.3;

// Global variables
int _position = 0;
int _lastPosition = 0;
int _moving = 0;
int _direction = 1; // 1 = out, -1 = in
unsigned long _lastPing = 0;
int _atEnd = 0; // 0 = false, 1 = out end, -1 = in end

Afro afro;

void setup() {
  afro.begin(ID, DEBUG);

  // Setup pins
  pinMode(POSITION_PIN, INPUT);

  afro.attach(AT_END, atEnd);
  afro.attach(PING, ping);
  afro.attach(MOVING, moving);
}

void ping(unsigned char from, unsigned char operand1, unsigned char operand2) {
  _lastPing = millis();
}

void moving(unsigned char from, unsigned char operand1, unsigned char operand2) {
  int value = operand1;
  if (value) {
    _moving = 1;
  } else {
    _moving = 0;
  }
}

void atEnd(unsigned char from, unsigned char operand1, unsigned char operand2) {
  _atEnd = operand1 - 65;
}

void updatePosition() {
  float infloat = (float)analogRead(POSITION_PIN) / 1024.0 * 100;
  _position = (int)infloat;

  if (abs(_position - _lastPosition) > 5) {
    afro.sendRequest(REMOTE, MOVE_TO, 'x', _position);
    _lastPosition = _position;
  }
}

void loop() {
  afro.processSerial();
  updatePosition();

  if (millis() - _lastPing > PING_TIME) {
    digitalWrite(CONNECTION_PIN, LOW);
  } else {
    digitalWrite(CONNECTION_PIN, HIGH);
  }

  digitalWrite(MOVING_PIN, _moving);

  if (_atEnd == -1) {
    digitalWrite(END_STOP_IN_PIN, HIGH);
    digitalWrite(END_STOP_OUT_PIN, LOW);
  } else if (_atEnd == 1) {
    digitalWrite(END_STOP_IN_PIN, LOW);
    digitalWrite(END_STOP_OUT_PIN, HIGH);
  } else {
    digitalWrite(END_STOP_IN_PIN, LOW);
    digitalWrite(END_STOP_OUT_PIN, LOW);
  }
}
