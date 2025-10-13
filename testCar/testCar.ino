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

#define walk_speed 130
#define tilt_speed 120
#define turn_speed 120
#define walk_speed_enB 205
#define tilt_speed_enB 185
#define turn_speed_enB 150
#define threshold 800
#define block 25

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
int distance = 0;

unsigned long timestamp;

int count = 1;

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
  // tft.print("Running :3", CENTER, 56);

  
  readUltrasonic();
  readIR();
  delay(1000);
  beep(1);

  home_to_check();
  delay(3000);
  beep(2);
  check_to_home();
  
  beep(3);
  readIR();

}

void loop() {


  // readUltrasonic();
  // readIR();
  // display_ir();


  // follow_line_full();
  // turn_right();
  // follow_line_full();
  // turn_left();

  // readIR();
}


void home_to_check() {
  delay(200);
  readUltrasonic();
  if (detect_box_front()) {
    turn_right();
    follow_line_full();
    turn_left();
    follow_line_full();
    turn_right();
  } else {
    follow_line_full();
    turn_right();
    follow_line_full();
  }
  follow_line_full();
  follow_line_full();
  turn_left();
  follow_line_full();
  follow_line_full();
  follow_line_full();
  turn_right();
  follow_line_full();
  delay(100);
  readUltrasonic();
  if (detect_box_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_left();
    straight(1500);
    readIR();
    
  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
    straight(1500);
  }
}

void check_to_home() {
  uturn();
  back_ward();
  delay(100);
  readUltrasonic();
  if (detect_box_front()) {
    turn_right();
    follow_line_full();
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_left();
  } else {
    follow_line_full();
    turn_right();
    follow_line_full();
    follow_line_full();
    turn_left();
  }
  follow_line_full();
  follow_line_full();
  follow_line_full();
  turn_right();
  follow_line_full();
  follow_line_full();
  delay(100);
  readUltrasonic();
  if (detect_box_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_right();
  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
    uturn();
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

bool detect_box_front() {
  readUltrasonic();
  return (distance > 0 && distance < block);
}

void safe_turn_right() {
  turn_right();
  stop(500);
  delay(2000);
  readUltrasonic();
  if (detect_box_front()) {
    beep(1);
    turn_left();
    follow_line_full();
    turn_right();
  } else return;
}

void safe_turn_left() {
  turn_left();
  stop(500);
  readUltrasonic();
  if (detect_box_front()) {
    beep(1);
    turn_right();
    follow_line_full();
    turn_left();
  } else return;
}

void safe_move_forward() {
  stop(500);
  readUltrasonic();
  if (detect_box_front()) {
    beep(1);
    while (millis() - timestamp < 200) {
      readIR();
      if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
        walk_straight();
      } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
        tilt_left();
      } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
        tilt_right();
      }
    }
    turn_right();
    follow_line_full();
    turn_left();
    follow_line_full();
  } else follow_line_full();
}

void straight(unsigned long time) {
  timestamp = millis();
  while (millis() - timestamp < time) {
      readIR();
      if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
        walk_straight();
      } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
        tilt_left();
      } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
        tilt_right();
      }
    }
    stop(0);
}


void follow_line() {
  while (isWhite(ir.rr) && isWhite(ir.ll)) {
    delay(20);
    readIR();

    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_left();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_right();
    }
    readIR();
  }
  readIR();
  timestamp = millis();

  while (millis() - timestamp < 200) {
    readIR();
    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_left();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_right();
    }
  }
  stop(0);
  readIR();
  stop(500);
  readIR();
}

void back_ward() {
  while (millis() - timestamp < 200) {
    readIR();
    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      analogWrite(enA, walk_speed);
      digitalWrite(inA, LOW);
      digitalWrite(inB, HIGH);

      analogWrite(enB, walk_speed_enB);
      digitalWrite(inC, LOW);
      digitalWrite(inD, HIGH);
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      analogWrite(enA, 0);
      digitalWrite(inA, LOW);
      digitalWrite(inB, LOW);

      analogWrite(enB, tilt_speed_enB);
      digitalWrite(inC, LOW);
      digitalWrite(inD, HIGH);
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      analogWrite(enA, tilt_speed);
      digitalWrite(inA, LOW);
      digitalWrite(inB, HIGH);

      analogWrite(enB, 0);
      digitalWrite(inC, LOW);
      digitalWrite(inD, LOW);
    }
  }
}

void follow_line_full() {
  follow_line();
  readIR();
  beep(0);
  stop(700);
  readIR();
}

void tilt_left() {
  analogWrite(enA, 0);
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);

  analogWrite(enB, tilt_speed_enB);
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

  analogWrite(enB, walk_speed_enB);
  digitalWrite(inC, HIGH);
  digitalWrite(inD, LOW);
}

void stop(int time) {
  readIR();
  analogWrite(enA, 0);
  digitalWrite(inA, LOW);
  digitalWrite(inB, LOW);

  analogWrite(enB, 0);
  digitalWrite(inC, LOW);
  digitalWrite(inD, LOW);
  readIR();
  delay(time);
  readIR();
}

void turn_left() {

  readIR();
  timestamp = millis();

  while (millis() - timestamp < 300) {
    readIR();
    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_left();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_right();
    }
  }
  stop(0);
  readIR();
  stop(500);

  while (!isWhite(ir.cl) || !isWhite(ir.cr)) {
    readIR();
    //tilt left untill all white
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }
  while (isWhite(ir.cr) || isWhite(ir.cl)) {
    readIR();
    //tilt left untill all black
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }

  readIR();
}

void turn_right() {

  timestamp = millis();

  readIR();
  while (millis() - timestamp < 300) {
    readIR();
    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_left();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_right();
    }
  }
  stop(0);

  stop(500);

  readIR();

  while (!isWhite(ir.cl) || !isWhite(ir.cr)) {
    readIR();
    //tilt left untill all white
    analogWrite(enA, turn_speed);
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);

    analogWrite(enB, turn_speed);
    digitalWrite(inC, LOW);
    digitalWrite(inD, HIGH);
  }
  while (isWhite(ir.cl) || isWhite(ir.cr)) {
    readIR();
    //tilt left untill all black
    analogWrite(enA, turn_speed);
    digitalWrite(inA, HIGH);
    digitalWrite(inB, LOW);

    analogWrite(enB, turn_speed);
    digitalWrite(inC, LOW);
    digitalWrite(inD, HIGH);
  }

  readIR();
}

void uturn() {

  readIR();
  while (millis() - timestamp < 200) {
    readIR();
    if (!isWhite(ir.cr) && !isWhite(ir.cl)) {
      walk_straight();
    } else if (!isWhite(ir.cl) && isWhite(ir.cr)) {
      tilt_left();
    } else if (!isWhite(ir.cr) && isWhite(ir.cl)) {
      tilt_right();
    }
  }
  stop(0);
  readIR();
  stop(500);

  while (!isWhite(ir.cl) || !isWhite(ir.cr)) {
    readIR();
    //tilt left untill all white
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }
  while (isWhite(ir.cr) || isWhite(ir.cl)) {
    readIR();
    //tilt left untill all black
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }

  stop(500);

  while (!isWhite(ir.cl) || !isWhite(ir.cr)) {
    readIR();
    //tilt left untill all white
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }
  while (isWhite(ir.cr) || isWhite(ir.cl)) {
    readIR();
    //tilt left untill all black
    analogWrite(enA, turn_speed);
    digitalWrite(inA, LOW);
    digitalWrite(inB, HIGH);

    analogWrite(enB, turn_speed_enB);
    digitalWrite(inC, HIGH);
    digitalWrite(inD, LOW);
  }
  stop(500);

  readIR();
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
    delay(200);
    digitalWrite(buzzPin, HIGH);
    delay(200);
  }
}

void display_ir() {

  readUltrasonic();
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
