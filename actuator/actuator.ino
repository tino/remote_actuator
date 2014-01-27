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
int DEBUG = 1;  // FIXME: devices behave erronous when starting with DEBUG = 0...

#define REMOTE 'R'

// Pins
const int DIRECTION_PIN = 1;
const int SPEED_PIN = 2;
const int POSITION_PIN = 3;
const int END_STOP_IN_PIN = 4;
const int END_STOP_OUT_PIN = 5;

// Times
const int HZ = 20;
const int HEARTBEAT = 2 * 1000;
const int STOP_TIMEOUT = 500; // millis

// Thresholds
const int TARGET_THRESHOLD = 5;
const int MAX_POSITION = 1023;

// Messaging commands
#define MOVE_TO         'M' // Move the actuator to a designated position
#define STOP            'S' // Stop the actuator
#define AT_END          'E' // Signal actuator is at an end (0 = in, 1 = out)
#define PING            'P' // Signal we actuator is live
#define MOVING          'N' // Signal wether actuator is moving


// States
#define REST                    0
#define STOPPED                 -1
#define MOVING                  1

// Global variables
int _position = 0;
int _targetPosition = 0;
int _moving = false;
int _direction = 1; // 1 = out, 0 = in
unsigned long _stopTimeOut = 0;
unsigned long _lastStoppedAt = 0;
int _state = REST;
int _lastState = REST;

Afro afro;

void setup(){
  // Start up Afro, we configured our XBEE devices for 9600 bps.
  afro.begin(ID, 9600);

  // Setup pins
  pinMode(POSITION_PIN, INPUT);
  pinMode(END_STOP_OUT_PIN, INPUT);
  pinMode(END_STOP_IN_PIN, INPUT);

  // put pin 5 low so it can be used with pull down for magnet of C
  // pinMode(5, OUTPUT);
  // digitalWrite(5, 0);

  afro.attach(MOVE_TO, moveTo);
  afro.attach(STOP, stop);

  afro.exerternalMonitor("online", ID);

  // allLedsOn();
  // delay(100);
  // allLedsOff();
}


void loop(){
  static unsigned long _lastHeartBeat = 0;
  static unsigned long _lastLoop = 0;

  afro.processSerial();
  // timer.run();

  // Main processing loop. Runs HZ times per second.
  if (millis() - _lastLoop > 1000/HZ)  {
    _lastLoop = millis();

    // Ping the remote
    afro.sendRequest(REMOTE, PING, 1, 1);

    // Update our readings
    int position = getPosition();
    int isIn = digitalRead(END_STOP_IN_PIN); // CHECK: higher than 3V? (1.98V)
    int isOut = digitalRead(END_STOP_OUT_PIN);

    // Safety
    if (isIn) {
      position = 0;
      stop();
    }
    if (isOut) {
      position = 100;
      stop();
    }

    doMove();

    // If state changed in the processing above, let the external monitor know
    if (_state != _lastState) {
      afro.exerternalMonitor("statechange", _state);
      _lastState = _state;
    }

    // Heartbeats and DEBUG
    if (millis() - _lastHeartBeat > HEARTBEAT) {
      _lastHeartBeat = millis();
      afro.exerternalMonitor("alive", ID);

      afro.sendDebug("position", position, 31);
      afro.sendDebug("isIn", isIn, 41);
      afro.sendDebug("isOut", isOut, 41);
      afro.sendDebug("state", _state, 21);

      // lightHeartBeat();

    }
  }
}

//////////////////////////////////////
// ACTION FUNCTIONS
//////////////////////////////////////


void moveTo(unsigned char from, unsigned char operand1, unsigned char target) {
  // discard really small updates
  if (abs(target - _targetPosition) < TARGET_THRESHOLD) return;
  int direction;

  if (target > getPosition()) {
    direction = 1;
  } else {
    direction = 0;
  }
  if (_state == REST || _state == STOPPED) {
    _targetPosition = target;
    _direction = direction;
    doMove();
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
}

int getPosition() {
  int position = analogRead(POSITION_PIN);
  return ((float)position / (float)MAX_POSITION) * 100;
}

void doMove() {
  if (_targetPosition == getPosition()) {
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
    if (getPosition() > _targetPosition && _direction == 0) stop();
    if (getPosition() < _targetPosition && _direction == 1) stop();
    return;
  } else if (_state == STOPPED) {
    if (millis() > _lastStoppedAt + STOP_TIMEOUT) start();
  } else if (_state == REST) {
    start();
  }
}

void start() {
  digitalWrite(DIRECTION_PIN, _direction);
  analogWrite(SPEED_PIN, 1023);
  _state = MOVING;
}

void stop() {
  analogWrite(SPEED_PIN, 0);
  _targetPosition = getPosition();
  _state = STOPPED;
  _lastStoppedAt = millis();
}
// as Afro callback
void stop(unsigned char from, unsigned char operand1, unsigned char operand2) {
  stop();
}


//////////////////////////////////////
// OUTPUT FUNCTIONS
//////////////////////////////////////




//////////////////////////////////////
// HELPER FUNCTIONS
//////////////////////////////////////



//////////////////////////////////////
// DEBUG FUNCTIONS
//////////////////////////////////////

// void lightHeartBeat() {
//   // Only show heartbeat led if output is not blocked
//   if (!_outputBlocked) {
//     updateLeds(B00011000, 0, 0.5);
//     // timer.setTimeout(100, dimHeartBeat);
//   }
// }

// void dimHeartBeat() {
//   if (!_outputBlocked) {
//     updateLeds(B00000000);
//   }
// }
