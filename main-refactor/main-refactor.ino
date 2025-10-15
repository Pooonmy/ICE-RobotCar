#include <UTFT.h>
extern uint8_t SmallFont[];
// (model, sda, sck, cs, rst, a0)
UTFT tft(ST7735, 6, 7, 3, 4, 5);
#define WHITE VGA_WHITE
#define BLACK VGA_BLACK

// Motor driver pins (connected to motor driver inputs)
#define EN_A 10
#define IN_A 8
#define IN_B 9
#define EN_B 11
#define IN_C 12
#define IN_D 13

// Base speed adjustments
#define SPEED_OFFSET -20
#define SPEED_B_OFFSET 0
#define WALK_SPEED (150 + SPEED_OFFSET)
#define TILT_SPEED (140 + SPEED_OFFSET)
#define TURN_SPEED (130 + SPEED_OFFSET)
#define WALK_SPEED_B (235 + SPEED_B_OFFSET)
#define TILT_SPEED_B (215 + SPEED_B_OFFSET)
#define TURN_SPEED_B (200 + SPEED_B_OFFSET)

#define IR_THRESHOLD 800
#define BLOCK_DISTANCE_CM 25

#define BTN_PIN 0
#define BUZZ_PIN 19
#define TRIG_PIN 1
#define ECHO_PIN 2
const unsigned long PULSE_TIMEOUT_US = 30000UL;
const float SOUND_US_PER_CM = 29.1f;
const int MIN_CM = 2;
const int MAX_CM = 400;

// IR sensors: indexes correspond to physical placement across the front of the robot
const int NUM_IR = 4;
const int IR_PINS[NUM_IR] = { A0, A1, A2, A3 }; // right_far, right_center, left_center, left_far
int ir_values[NUM_IR];

// Structured IR readings
typedef struct sensor_ir {
  int left_far;     // far-left sensor
  int left_center;  // left-center sensor
  int right_center; // right-center sensor
  int right_far;    // far-right sensor
} ir_sensors_t;

ir_sensors_t ir;

long pulse_duration;
int distance_cm = 0;

unsigned long millis_timestamp;

int beep_count_global = 1;

void beep(int times);

// setup: initialize display, pins and wait for start button
void setup() {
  tft.InitLCD();
  tft.setFont(SmallFont);
  tft.clrScr();
  tft.fillScr(BLACK);
  tft.print("Press to start", CENTER, 56);

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(BUZZ_PIN, HIGH);
  digitalWrite(TRIG_PIN, LOW);
  for (int i = 0; i < NUM_IR; i++) {
    pinMode(IR_PINS[i], INPUT);
  }

  // Wait for user to press the start button (active LOW)
  while (digitalRead(BTN_PIN) == HIGH) {
    // idle
  }

  tft.clrScr();
  tft.print("Running :3", CENTER, 56);

  // small initial sensor reads to populate values
  read_ultrasonic();
  read_ir_sensors();
  delay(1000);
  beep(1);

  // navigate: home -> checkpoint
  home_to_checkpoint();
  delay(3000);
  beep(2);
  delay(1000);

  // navigate: checkpoint -> home
  checkpoint_to_home();
  delay(1000);
  beep(3);

  // end here

  // push_one_block();
  // beep(1);
  // delay(500);
  // turn_left();
  // follow_line_full();
  // turn_right();
  // follow_line_full();
  // turn_right();
  // push_one_block();
}

void loop() {

  // main loop currently idle; diagnostics can be enabled here
  // read_ultrasonic();
  // read_ir_sensors();
  // display_ir_data();

}


void home_to_checkpoint() {
  delay(200);
  read_ultrasonic();
  if (detect_box_in_front()) {
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
  turn_left();
  follow_line_full();
  push_one_block();
  turn_right();
  follow_line_full();
  follow_line_full();
  turn_left();
  follow_line_full();
  turn_right();
  stop(50);
  delay(50);
  read_ultrasonic();
  if (detect_box_in_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_left();
    stop(0);

  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
  }
}
void checkpoint_to_home() {
  u_turn();
  stop(100);
  drive_straight(300);
  stop(500);
  delay(100);
  read_ultrasonic();
  read_ultrasonic();
  if (detect_box_in_front()) {
    turn_right();
    stop(0);
    delay(200);
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
  read_ultrasonic();
  if (detect_box_in_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_right();
    stop(0);
  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
  }
}

int is_white(int reading) {
  return reading < IR_THRESHOLD;
}

bool detect_box_in_front() {
  read_ultrasonic();
  return (distance_cm > 0 && distance_cm < BLOCK_DISTANCE_CM);
}


void drive_straight(unsigned long time_ms) {
  millis_timestamp = millis();
  while (millis() - millis_timestamp < time_ms) {
    read_ir_sensors();
    if (!is_white(ir.right_center) && !is_white(ir.left_center)) {
      drive_forward();
    } else if (!is_white(ir.left_center) && is_white(ir.right_center)) {
      tilt_left();
    } else if (!is_white(ir.right_center) && is_white(ir.left_center)) {
      tilt_right();
    }
  }
  stop(0);
}


void follow_line() {
  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(50);
  while (is_white(ir.right_far) && is_white(ir.left_far)) {
    delay(20);
    read_ir_sensors();

    if (!is_white(ir.right_center) && !is_white(ir.left_center)) {
      drive_forward();
    } else if (!is_white(ir.left_center) && is_white(ir.right_center)) {
      tilt_left();
    } else if (!is_white(ir.right_center) && is_white(ir.left_center)) {
      tilt_right();
    }
    read_ir_sensors();
  }
  read_ir_sensors();
  drive_straight(300);
  stop(0);
  read_ir_sensors();
  stop(300);
  read_ir_sensors();
}

#define DELAY_SPEED 20
#define DELAY_SPEED_B 120

void drive_backward() {
  analogWrite(EN_A, 200);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, HIGH);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, HIGH);
  delay(50);
  while (is_white(ir.right_far) && is_white(ir.left_far)) {
    read_ir_sensors();

    analogWrite(EN_A, WALK_SPEED - DELAY_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, WALK_SPEED_B - DELAY_SPEED_B);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }
  stop(500);
  drive_straight(500);
}

void follow_line_full() {
  follow_line();
  read_ir_sensors();
  beep(0);
  stop(400);
  read_ir_sensors();
}

void push_one_block() {

  drive_straight(300);
  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(50);

  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(150);

  follow_line();

  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(50);

  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(0);

  stop(200);

  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(200);

  drive_straight(500);
  stop(0);
  drive_backward();
}

void tilt_left() {
  analogWrite(EN_A, 0);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, TILT_SPEED_B);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
}


void tilt_right() {
  analogWrite(EN_A, TILT_SPEED);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 0);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, LOW);
}

void drive_forward() {
  analogWrite(EN_A, WALK_SPEED);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, WALK_SPEED_B);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
}

void stop(int time_ms) {
  read_ir_sensors();
  analogWrite(EN_A, 0);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 0);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, LOW);
  read_ir_sensors();
  delay(time_ms);
  read_ir_sensors();
}

void turn_left() {

  read_ir_sensors();
  drive_straight(0);
  stop(0);
  read_ir_sensors();
  stop(500);

  while (!is_white(ir.left_center) || !is_white(ir.right_center)) {
    read_ir_sensors();
    // rotate left until sensors detect white
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.right_center) || is_white(ir.left_center)) {
    read_ir_sensors();
    // continue rotating until sensors detect black
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }

  read_ir_sensors();
}

void turn_right() {

  drive_straight(0);
  stop(0);

  stop(500);

  read_ir_sensors();

  while (!is_white(ir.left_center) || !is_white(ir.right_center)) {
    read_ir_sensors();
    // rotate right until sensors detect white
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, HIGH);
    digitalWrite(IN_B, LOW);

    analogWrite(EN_B, TURN_SPEED);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }
  while (is_white(ir.left_center) || is_white(ir.right_center)) {
    read_ir_sensors();
    // continue rotating until sensors detect black
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, HIGH);
    digitalWrite(IN_B, LOW);

    analogWrite(EN_B, TURN_SPEED);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }

  read_ir_sensors();
}

void u_turn() {

  read_ir_sensors();
  drive_straight(300);
  stop(0);
  read_ir_sensors();
  stop(500);

  while (!is_white(ir.left_center) || !is_white(ir.right_center)) {
    read_ir_sensors();
    // rotate left until sensors detect white
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.right_center) || is_white(ir.left_center)) {
    read_ir_sensors();
    // continue rotating until sensors detect black
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }

  stop(500);
  drive_straight(400);
  stop(300);

  while (!is_white(ir.left_center) || !is_white(ir.right_center)) {
    read_ir_sensors();
    // rotate left until sensors detect white
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.right_center) || is_white(ir.left_center)) {
    read_ir_sensors();
    // continue rotating until sensors detect black
    analogWrite(EN_A, TURN_SPEED);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_B);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  stop(500);

  read_ir_sensors();
}

void read_ultrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration_local = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (duration_local == 0) return;

  int cm = (int)((duration_local / 2.0f) / SOUND_US_PER_CM);

  if (cm < MIN_CM || cm > MAX_CM) return;

  distance_cm = cm;
}

void read_ir_sensors() {
  for (int i = 0; i < NUM_IR; i++) {
    ir_values[i] = analogRead(IR_PINS[i]);
  }
  // [0]=right_far, [1]=right_center, [2]=left_center, [3]=left_far
  ir.right_far = analogRead(IR_PINS[0]);
  ir.right_center = analogRead(IR_PINS[1]);
  ir.left_center = analogRead(IR_PINS[2]);
  ir.left_far = analogRead(IR_PINS[3]);
}

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZ_PIN, LOW);
    delay(200);
    digitalWrite(BUZZ_PIN, HIGH);
    delay(200);
  }
}

void display_ir_data() {

  read_ultrasonic();
  read_ir_sensors();
  tft.setColor(WHITE);
  tft.print(String("Distance:"), 0, 0);
  tft.print(String(distance_cm) + "  ", 80, 0);
  tft.print(String("IR rf"), 0, 12 + 12 * 0);
  tft.print(String(":"), 40, 12 + 12 * 0);
  tft.print(String(ir.right_far) + "    ", 48, 12 + 12 * 0);

  tft.print(String("IR rc"), 0, 12 + 12 * 1);
  tft.print(String(":"), 40, 12 + 12 * 1);
  tft.print(String(ir.right_center) + "    ", 48, 12 + 12 * 1);

  tft.print(String("IR lc"), 0, 12 + 12 * 2);
  tft.print(String(":"), 40, 12 + 12 * 2);
  tft.print(String(ir.left_center) + "    ", 48, 12 + 12 * 2);

  tft.print(String("IR lf"), 0, 12 + 12 * 3);
  tft.print(String(":"), 40, 12 + 12 * 3);
  tft.print(String(ir.left_far) + "    ", 48, 12 + 12 * 3);
}
