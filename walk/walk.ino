#include <UTFT.h>
extern uint8_t SmallFont[];
//(Model,SDA,SCK,CS,RST,A0)
UTFT tft(ST7735, 6, 7, 3, 4, 5);
#define White VGA_WHITE
#define Black VGA_BLACK

#define enA 11
#define inA 12
#define inB 13
#define enB 10
#define inC 8
#define inD 9

#define walk_speed 100
#define tilt_speed 100

#define btnPin 0
#define buzzPin 19
#define trigPin 1
#define echoPin 2
const unsigned long PULSE_TIMEOUT_US = 30000UL;
const float SOUND_US_PER_CM = 29.1f;
const int MIN_CM = 2;
const int MAX_CM = 400;

const int numIR = 5;
const int irPins[numIR] = { A0, A1, A2, A3, A4 };
int irVals[numIR];

long duration;
int distance;

void beep(int count);

void setup() {
  tft.InitLCD();
  tft.setFont(SmallFont);
  tft.clrScr();
  tft.fillScr(Black);
  tft.print("Press to start", CENTER, 56);

  pinMode(btnPin, INPUT_PULLUP);
  pinMode(buzzPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(buzzPin, HIGH);
  digitalWrite(trigPin, LOW);
  for (int i = 0; i < numIR; i++) {
    pinMode(irPins[i], INPUT);
  }

  while (digitalRead(btnPin) == HIGH) {
    // Do absolutely nothing
  }

  beep(1);
  tft.clrScr();
}

void loop() {
  readUltrasonic();
  readIR();
  tft.setColor(White);
  tft.print(String("Distance:"), 0, 0);
  tft.print(String(distance) + "  ", 80, 0);
  for (int i = 0; i < numIR; i++) {
    tft.print(String("IR"), 0, 12 + 12 * i);
    tft.print(String(i), 24, 12 + 12 * i);
    tft.print(String(":"), 32, 12 + 12 * i);
    tft.print(String(irVals[i]) + "    ", 48, 12 + 12 * i);
  }
  while (1) {
    readIR();
    if (irVals[1] > 400 && irVals[2] > 400) {
      walk_straight();
    } else if (irVals[2] > 400 && irVals[1] < 400) {
      tilt_right();
    } else if (irVals[1] > 400 && irVals[2] < 400) {
      tilt_left();
    }
  }
}

void tilt_left() {
  analogWrite(enA, 0);
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);

  analogWrite(enB, tilt_speed);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
}


void tilt_right() {
  analogWrite(enA, tilt_speed);
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);

  analogWrite(enB, 0);
  digitalWrite(inC, LOW);
  digitalWrite(inD, LOW);
}

void walk_straight() {
  analogWrite(enA, walk_speed);
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);

  analogWrite(enB, walk_speed);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
}

void turn_left() {
  analogWrite(enA, 0);
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);

  analogWrite(enB, 100);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
  delay(100);
}

void turn_right() {
  analogWrite(enA, 100);
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);

  analogWrite(enB, 0);
  digitalWrite(inC, LOW);
  digitalWrite(inD, LOW);
  delay(100);
}

void readUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) return -1;

  int cm = (int)((duration / 2.0f) / SOUND_US_PER_CM);

  if (cm < MIN_CM || cm > MAX_CM) return -1;

  distance = cm;
}

void readIR() {
  for (int i = 0; i < numIR; i++) {
    irVals[i] = analogRead(irPins[i]);
  }
}

void beep(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(buzzPin, LOW);
    delay(50);
    digitalWrite(buzzPin, HIGH);
    delay(50);
  }
}
