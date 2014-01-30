#include <Afro.h>
#include <SimpleTimer.h>

Afro afro;
SimpleTimer timer;

void setup() {
    Serial.begin(9600);
    pinMode(2, INPUT);
    pinMode(2, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(13, OUTPUT);

}

// void loop() {
//     Serial.print(analogRead(2));
//     Serial.println(", ");
//     // Serial.printl(analogRead(2));
//     test();
//     delay(2000);
//     off();
// }

// void test() {
//     digitalWrite(2, HIGH);
//     digitalWrite(7, HIGH);
//     digitalWrite(13, HIGH);
//     // timer.setTimeout(2000, off);
// }

// void off() {
//     digitalWrite(2, LOW);
//     digitalWrite(7, LOW);
//     digitalWrite(13, LOW);
// }

void loop() {
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(2, 0);   // turn the LED on (0 is the voltage level)
  digitalWrite(7, 0);   // turn the LED on (0 is the voltage level)
  digitalWrite(13, 0);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
}
