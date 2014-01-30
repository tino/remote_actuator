#define ID 'A'
// ^ Device ID
// Letters A-Z can be used. 0 is reserved for broadcast

#include <Afro.h>
#include <SimpleTimer.h>

// Constants and global vars
// ----------

// enable to show debugging information about parsing and operations
// not so good on broadcast channels. Not a constant so we can switch it live
// 0 = off
// 1 = lowest debug level
// the higher the number, the more verbose the logging becomes
// double digit debug levels can be used for specific types, so setting debug
// to 23 will show everything of level 1 + everything 21 - 23
int DEBUG = 2;  // FIXME: devices behave erronous when starting with DEBUG = 0...

#define REMOTE 'R'

// Pins
const int DIRECTION_PIN = 7;
const int SPEED_PIN = 5;
const int POSITION_PIN = 2;
const int END_STOP_IN_PIN = 0;
const int END_STOP_OUT_PIN = 1;
// const int HEARTBEAT_PIN = 12;
const int ON_PIN = 12;

// Times
const int HZ = 10;
const int HEARTBEAT = 2 * 1000;
const int PING_TIME = 2000;
const int STOP_TIMEOUT = 500; // millis

// Thresholds
const int TARGET_THRESHOLD = 5;
const int MAX_POSITION = 325;

// Messaging commands
#define MOVE_TO         'M' // Move the actuator to a designated position
#define STOP            'S' // Stop the actuator
#define AT_END          'E' // Signal actuator is at an end (0 = in, 1 = out)
#define PING            'P' // Signal we actuator is live
#define STATE           'N' // Signal wether actuator is moving


// States
#define REST                    0
#define STOPPED                 -1
#define MOVING                  1

// Global variables
int _position = 0;
int _lastPosition = 0;
int _targetPosition = 0;
int _lastTargetPosition = 0;
int _direction = 1; // 1 = out, 0 = in
int _lastDirection = 1;
unsigned long _stopTimeOut = 0;
unsigned long _lastStoppedAt = 0;
int _state = REST;
int _lastState = REST;

Afro afro;
SimpleTimer timer;

void setup(){
  // Start up Afro, we configured our XBEE devices for 9600 bps.
  afro.begin(ID, DEBUG, 9600);

  // Setup pins
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(POSITION_PIN, INPUT);
  // pinMode(END_STOP_OUT_PIN, INPUT);
  // pinMode(END_STOP_IN_PIN, INPUT);
  pinMode(ON_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(ON_PIN, HIGH);

  // put pin 5 low so it can be used with pull down for magnet of C
  // pinMode(5, OUTPUT);
  // digitalWrite(5, 0);

  afro.attach(MOVE_TO, moveTo);
  afro.attach(STOP, forceStop);

  afro.exerternalMonitor("online", ID);

  // allLedsOn();
  // delay(100);
  // allLedsOff();
  ping();
}


void loop(){
  static unsigned long _lastHeartBeat = 0;
  static unsigned long _lastLoop = 0;

  afro.processSerial();
  timer.run();

  // Main processing loop. Runs HZ times per second.
  if (millis() - _lastLoop > 1000/HZ)  {
    _lastLoop = millis();

    // Message position if it has changed
    int position = getPosition();
    if (position != _lastPosition) {
      afro.exerternalMonitor("position", position);
      _lastPosition = position;
    }

    // Message if targetPosition changes
    if (_targetPosition != _lastTargetPosition) {
      afro.exerternalMonitor("targetPosition", _targetPosition);
      _lastTargetPosition = _targetPosition;
    }

    // Message direction when it changes
    if (_direction != _lastDirection) {
      afro.exerternalMonitor("direction", _direction);
      _lastDirection = _direction;
    }


    // Update our readings
    int isIn = digitalRead(END_STOP_IN_PIN); // CHECK: higher than 3V? (1.98V)
    int isOut = digitalRead(END_STOP_OUT_PIN);

    // // // Safety
    // if (isIn && _state == MOVING && _direction == 0) {
    //   // stop();
    //   // position = 0;
    //   afro.exerternalMonitor("is IN", 1);
    // }
    // if (isOut && _state == MOVING && _direction == 1) {
    //   // stop();
    //   // position = 100;
    //   afro.exerternalMonitor("is OUT", 1);
    // }

    doMove();

    // If state changed in the processing above, let the external monitor know
    if (_state != _lastState) {
      afro.exerternalMonitor("state", _state);
      _lastState = _state;
    }

    // Heartbeats and DEBUG
    if (millis() - _lastHeartBeat > HEARTBEAT) {
      _lastHeartBeat = millis();
      afro.exerternalMonitor("alive", _lastPosition);

      // afro.sendDebug("isIn", isIn, 41);
      // afro.sendDebug("isOut", isOut, 41);
      // afro.sendDebug("state", _state, 21);

      // lightHeartBeat();

    }
  }
}

//////////////////////////////////////
// ACTION FUNCTIONS
//////////////////////////////////////


void moveTo(unsigned char from, unsigned char operand1, unsigned char target) {
  afro.exerternalMonitor("moveTo start", target);
  // discard really small updates
  if (abs(target - _targetPosition) < TARGET_THRESHOLD) return;
  if (target <= 5) target = 5;
  int direction;

  if (target > getPosition()) {
    direction = 1;
  } else {
    direction = 0;
  }
  if (_state == REST || _state == STOPPED) {
    afro.exerternalMonitor("moving to", target);
    _targetPosition = target;
    _direction = direction;
    start();
  } else {
    // We are moving, check if we need to revert.
    if (direction != _direction) {
      // We need to revert. First stop for STOP_TIMEOUT
      stop();
      _direction = direction;
    } else {
      // Just update target
      _targetPosition = target;
    }
  }
  afro.exerternalMonitor("moveTo target", _targetPosition);
  afro.exerternalMonitor("moveTo direction", _direction);
}

int getPosition() {
  int read = analogRead(POSITION_PIN);
  int position = ((float)read / (float)MAX_POSITION) * 100;
  if (position >= 100) return 100;
  if (position <= 5) return 5;
  return position;
}

void doMove() {
  if (_state == REST) return;

  if (_targetPosition == getPosition()) {
    afro.exerternalMonitor("target reached", _targetPosition);
    // We are at our target
    if (_state == MOVING) {
      stop();
    } else if (_state == STOPPED) {
      if (millis() > _lastStoppedAt + STOP_TIMEOUT) _state = REST;
    }
    return;
  }

  // We are not at target
  if (_state == MOVING) {
    // but we are already moving
    // Do check if direction is still oke for overshoot
    if ((getPosition() < _targetPosition) && _direction == 0) {
      afro.exerternalMonitor("undershoot", getPosition() - _targetPosition);
      stop();
    }
    if ((getPosition() > _targetPosition) && _direction == 1) {
      afro.exerternalMonitor("overshoot", getPosition() - _targetPosition);
      stop();
    }
  } else if (_state == STOPPED) {
    if (millis() > _lastStoppedAt + STOP_TIMEOUT) start();
  }
}

void start() {
  afro.exerternalMonitor("starting", 1);
  afro.exerternalMonitor("writing direction", _direction);
  digitalWrite(DIRECTION_PIN, _direction);
  digitalWrite(13, _direction);
  analogWrite(SPEED_PIN, 1023);
  _state = MOVING;
}

void stop() {
  afro.exerternalMonitor("stopping", 1);
  digitalWrite(13, 0);
  analogWrite(SPEED_PIN, 0);
  _targetPosition = getPosition();
  _state = STOPPED;
  _lastStoppedAt = millis();
}

void forceStop(unsigned char from, unsigned char operand1, unsigned char operand2) {
  afro.exerternalMonitor("forced stopping", 1);
  digitalWrite(13, 0);
  analogWrite(SPEED_PIN, 0);
  _targetPosition = getPosition();
  _state = REST;
}



//////////////////////////////////////
// OUTPUT FUNCTIONS
//////////////////////////////////////

void ping() {
  // afro.sendRequest(REMOTE, PING, 1, 1);
  timer.setTimeout(PING_TIME, ping);
}


//////////////////////////////////////
// HELPER FUNCTIONS
//////////////////////////////////////



//////////////////////////////////////
// DEBUG FUNCTIONS
//////////////////////////////////////

// void lightHeartBeat() {
//   digitalWrite(HEARTBEAT_PIN, HIGH);
//   timer.setTimeout(100, dimHeartBeat);
// }

// void dimHeartBeat() {
//   digitalWrite(HEARTBEAT_PIN, LOW);
// }
