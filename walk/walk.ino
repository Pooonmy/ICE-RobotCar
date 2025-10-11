#include <UTFT.h>
extern uint8_t SmallFont[];
//(Model,SDA,SCK,CS,RST,A0)
UTFT tft(ST7735, 6, 7, 3, 4, 5);
#define White VGA_WHITE
#define Black VGA_BLACK

#define enA 10
#define inA 8
#define inB 9
#define enB 11
#define inC 12
#define inD 13

#define walk_speed 100
#define tilt_speed 80
#define threshold 400

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

typedef struct s_ir {
  int front;
  int ll;
  int cl;
  int cr;
  int rr;
} ir_s;

ir_s ir;

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
  Serial.begin(9600);
  for (int i = 0; i < numIR; i++) {
    pinMode(irPins[i], INPUT);
  }

  while (digitalRead(btnPin) == HIGH) {
    // Do absolutely nothing
  }

  beep(0);
  tft.clrScr();
  tft.print("Running :3", CENTER, 56);
}

void loop() {

  if (digitalRead(btnPin) == LOW) {
    stop();
    beep(0);
    tft.clrScr();
    tft.print("Reset to continue", CENTER, 56);
    while (true)
      ;
  }

  readUltrasonic();
  readIR();
  // display_ir();

  while (isWhite(ir.rr) && isWhite(ir.ll)) {
    readIR();
    // display_ir();
    // Serial.println("Here");
    if (digitalRead(btnPin) == LOW) {
      beep(0);
      stop();
      tft.clrScr();
      tft.print("Reset to continue", CENTER, 56);
      while (true)
        ;
    }

    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_right();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_left();
  }
  // stop();
  // delay(500);
}
}

void log(void) {

  readIR();
  // Serial.print("ll : ");
  // Serial.println(ir.ll);
  // Serial.print("cl : ");
  // Serial.println(ir.cl);
  // Serial.print("cr : ");
  // Serial.println(ir.cr);
  // Serial.print("rr : ");
  // Serial.println(ir.rr);
  // Serial.print("front : ");
  // Serial.println(ir.front);
}

int isWhite(int read) {
  return read < threshold;
}

void tilt_left() {
  analogWrite(enA, tilt_speed);
  digitalWrite(inA, LOW);
  digitalWrite(inB, HIGH);

  analogWrite(enB, tilt_speed);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
}


void tilt_right() {
  analogWrite(enA, tilt_speed);
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);

  analogWrite(enB, tilt_speed);
  digitalWrite(inC, LOW);
  digitalWrite(inD, HIGH);
}

void walk_straight() {
  analogWrite(enA, walk_speed);
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);

  analogWrite(enB, walk_speed);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
}

void stop() {
  analogWrite(enA, 0);
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);

  analogWrite(enB, 0);
  digitalWrite(inC, LOW);
  digitalWrite(inD, LOW);
}

void turn_left() {

  readIR();
  while (isWhite(ir.rr) && isWhite(ir.ll)) {
    readIR();
    walk_straight();
  }

  while (!isWhite(ir.cl) && !isWhite(ir.cr)) {
    readIR();
    //tilt left untill all white
    analogWrite(enA, tilt_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, tilt_speed);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }
  while (isWhite(ir.cl) && isWhite(ir.cr)) {
    readIR();
    //tilt left untill all black
    analogWrite(enA, tilt_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, tilt_speed);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
    if (!isWhite(ir.cl) && !isWhite(ir.cr)) {
      readIR();
      return (0);
    }
  }
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
  ir.front = analogRead(irPins[4]);
  ir.ll = analogRead(irPins[3]);
  ir.cl = analogRead(irPins[2]);
  ir.cr = analogRead(irPins[1]);
  ir.rr = analogRead(irPins[0]);
}

void beep(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(buzzPin, LOW);
    delay(50);
    digitalWrite(buzzPin, HIGH);
    delay(50);
  }
}

void display_ir() {
  ir_s ir;

  readIR();
  tft.setColor(White);
  tft.print(String("Distance:"), 0, 0);
  tft.print(String(distance) + "  ", 80, 0);
  tft.print(String("IR rr"), 0, 12 + 12 * 0);
  tft.print(String(":"), 40, 12 + 12 * 0);
  tft.print(String(ir.rr) + "    ", 48, 12 + 12 * 0);

  tft.print(String("IR cr"), 0, 12 + 12 * 1);
  tft.print(String(":"), 40, 12 + 12 * 1);
  tft.print(String(ir.cr) + "    ", 48, 12 + 12 * 1);

  tft.print(String("IR cl"), 0, 12 + 12 * 2);
  tft.print(String(":"), 40, 12 + 12 * 2);
  tft.print(String(ir.cl) + "    ", 48, 12 + 12 * 2);

  tft.print(String("IR ll"), 0, 12 + 12 * 3);
  tft.print(String(":"), 40, 12 + 12 * 3);
  tft.print(String(ir.ll) + "    ", 48, 12 + 12 * 3);

  tft.print(String("IR F"), 0, 12 + 12 * 4);
  tft.print(String(":"), 40, 12 + 12 * 4);
  tft.print(String(ir.front) + "    ", 48, 12 + 12 * 4);
}
