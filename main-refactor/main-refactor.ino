// =========================
// Libraries and Pinouts
// =========================
#include <UTFT.h>
extern uint8_t SmallFont[];
     // (Model,SDA,SCK,CS,RST,A0)
UTFT tft(ST7735, 6, 7, 3, 4, 5);

// Color aliases
#define WHITE VGA_BLACK
#define BLACK VGA_WHITE


// Left motor: EN_A, IN_A, IN_B
// Right motor: EN_B, IN_C, IN_D
#define EN_A 10
#define IN_A 8
#define IN_B 9
#define EN_B 11
#define IN_C 12
#define IN_D 13

// Sensors and IO Pins
#define BTN_PIN 0
#define TRIG_PIN 1         
#define ECHO_PIN 2
#define BUZZER_PIN 19

// =========================
// Constants and Tunables
// =========================
const unsigned long PULSE_TIMEOUT_US = 30000UL;
const float SOUND_US_PER_CM = 29.1f;
const int MIN_CM = 2;
const int MAX_CM = 400;

// IR line sensor config
const int NUM_IR = 4; // rr, cr, cl, ll
const uint8_t IR_PINS[NUM_IR] = {A0, A1, A2, A3};
const int LINE_THRESHOLD = 800; // < threshold = white, >= threshold = black

// Box detection threshold
const int BOX_DETECT_CM = 25;

// Speed calibration offsets
const int SPEED_OFFSET_LEFT = -20;
const int SPEED_OFFSET_RIGHT = -10;

// Left Motor Base speeds
const int WALK_SPEED_LEFT = 150 + SPEED_OFFSET_LEFT;
const int TILT_SPEED_LEFT = 140 + SPEED_OFFSET_LEFT;
const int TURN_SPEED_LEFT = 130 + SPEED_OFFSET_LEFT;

// Right Motor Base speeds
const int WALK_SPEED_RIGHT = 235 + SPEED_OFFSET_RIGHT;
const int TILT_SPEED_RIGHT = 215 + SPEED_OFFSET_RIGHT;
const int TURN_SPEED_RIGHT = 210 + SPEED_OFFSET_RIGHT;

// Backward delta speeds
const int DEL_SPEED_LEFT = 20;
const int DEL_SPEED_RIGHT = 120;

// =========================
// Globals Variables
// =========================
struct IrReadings {
  int ll; // edge left
  int cl; // center left
  int cr; // center right
  int rr; // edge right
};

IrReadings ir;               // Latest IR readings
int distance_cm = -1;        // Latest distance in cm
unsigned long time_mark_ms;  // General-purpose time marker

// =========================
// Function Declarations
// =========================
void beep_times(int count);
void read_ultrasonic();
void read_ir();
bool is_white(int value);
bool detect_box_front();
void drive_forward_ms(unsigned long duration_ms);
void follow_line();
void follow_line_full();
void drive_backward();
void tilt_left();
void tilt_right();
void walk_straight();
void stop_motors(int hold_ms);
void turn_left();
void turn_right();
void u_turn();
void display_ir();
void home_check_mission();
void check_mission_home();

// =========================
// Setup and Loop
// =========================
void setup() {
  // Initialize display
  tft.InitLCD();
  tft.setFont(SmallFont);
  tft.clrScr();
  tft.fillScr(BLACK);
  tft.print("Press to start", CENTER, 56);

  // IO setup
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(TRIG_PIN, LOW);

  // IR sensors
  for (int i = 0; i < NUM_IR; i++) {
    pinMode(IR_PINS[i], INPUT);
  }

  // Wait untill start button is pressed
  while (digitalRead(BTN_PIN) == HIGH) {
    // Idle wait
  }

  beep_times(0);
  tft.clrScr();

  // Startup
  read_ultrasonic();
  read_ir();
  delay(1000);
  beep_times(1);

  // Mission: home -> checkpoint
  home_check_mission();
  delay(3000);
  beep_times(2);
  delay(1000);

  // Mission: checkpoint -> home
  check_mission_home();
  delay(1000);
  beep_times(3);
}

void loop() {
  // Nothing here
}

// =========================
// Missions
// =========================
void home_check_mission() {
  delay(200);
  read_ultrasonic();
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
  turn_left();
  follow_line_full();

  turn_left();
  drive_forward_ms(200);
  pushblock();
  turn_left();
  follow_line_full();

  follow_line_full();
  turn_left();
  follow_line_full();
  turn_right();

  if (detect_box_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
    turn_left();
    stop_motors(0);
  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
  }
}

void check_mission_home() {
  u_turn();
  stop_motors(100);
  drive_forward_ms(400);
  stop_motors(500);
  delay(100);
  read_ultrasonic();
  read_ultrasonic();
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
  turn_left();
  follow_line_full();
  follow_line_full();
  follow_line_full();

  turn_right();
  follow_line_full();
  follow_line_full();
  turn_right();

  follow_line_full();
  turn_right();
  drive_forward_ms(200);
  pushblock();

  turn_right();
  follow_line_full();
  turn_left();
  follow_line_full();

  turn_left();
  drive_forward_ms(200);
  pushblock();
  drive_forward_ms(500);
  pushblock();

  turn_left();
  follow_line_full();
  turn_right();
  follow_line_full();

  turn_right();
  drive_forward_ms(200);
  pushblock();

  turn_right();
  follow_line_full();
  follow_line_full();
  follow_line_full();

  turn_right();
  follow_line_full();
  follow_line_full();
  if (detect_box_front()) {
    turn_left();
    follow_line_full();
    turn_right();
    follow_line_full();
  } else {
    follow_line_full();
    turn_left();
    follow_line_full();
  }
}

// =========================
// Line Following and Movement
// =========================
bool is_white(int value) {
  return value < LINE_THRESHOLD;
}

bool detect_box_front() {
  read_ultrasonic();
  return (distance_cm > 0 && distance_cm < BOX_DETECT_CM);
}

void drive_forward_ms(unsigned long duration_ms) {
  time_mark_ms = millis();
  while (millis() - time_mark_ms < duration_ms) {
    read_ir();
    if (!is_white(ir.cr) && !is_white(ir.cl)) {
      walk_straight();
    } else if (!is_white(ir.cl) && is_white(ir.cr)) {
      tilt_left();
    } else if (!is_white(ir.cr) && is_white(ir.cl)) {
      tilt_right();
    }
  }
  stop_motors(0);
}

void follow_line() {
  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(50);
  while (is_white(ir.rr) && is_white(ir.ll)) {
    delay(20);
    read_ir();

    if (!is_white(ir.cr) && !is_white(ir.cl)) {
      walk_straight();
    } else if (!is_white(ir.cl) && is_white(ir.cr)) {
      tilt_left();
    } else if (!is_white(ir.cr) && is_white(ir.cl)) {
      tilt_right();
    }
    read_ir();
  }
  read_ir();
  drive_forward_ms(300);
  stop_motors(0);
  read_ir();
  stop_motors(300);
  read_ir();
}

void drive_backward() {
  analogWrite(EN_A, 200);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, HIGH);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, HIGH);
  delay(50);
  while (is_white(ir.rr) && is_white(ir.ll)) {
    read_ir();

    analogWrite(EN_A, WALK_SPEED_LEFT - DEL_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, WALK_SPEED_RIGHT - DEL_SPEED_RIGHT);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }
  stop_motors(500);
  drive_forward_ms(500);
}

void follow_line_full() {
  follow_line();
  read_ir();
  beep_times(0);
  stop_motors(400);
  read_ir();
}

// =========================
// Push Block Sequence
// =========================
void pushblock() {
  drive_forward_ms(500);

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

  stop_motors(200);

  analogWrite(EN_A, 200);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 255);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
  delay(0);

  drive_forward_ms(1000);
  stop_motors(200);
  drive_backward();
}

// =========================
// Low-level Motor Helpers
// =========================
void tilt_left() {
  // Stop left wheel, drive right forward
  analogWrite(EN_A, 0);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, TILT_SPEED_RIGHT);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
}

void tilt_right() {
  // Stop right wheel, drive left forward
  analogWrite(EN_A, TILT_SPEED_LEFT);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 0);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, LOW);
}

void walk_straight() {
  // Drive both forward
  analogWrite(EN_A, WALK_SPEED_LEFT);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, WALK_SPEED_RIGHT);
  digitalWrite(IN_C, HIGH);
  digitalWrite(IN_D, LOW);
}

void stop_motors(int hold_ms) {
  // Stop both motors with delay
  read_ir();
  analogWrite(EN_A, 0);
  digitalWrite(IN_A, LOW);
  digitalWrite(IN_B, LOW);

  analogWrite(EN_B, 0);
  digitalWrite(IN_C, LOW);
  digitalWrite(IN_D, LOW);
  read_ir();
  delay(hold_ms);
  read_ir();
}

void turn_left() {
  read_ir();
  drive_forward_ms(0);
  stop_motors(0);
  read_ir();
  stop_motors(500);

  while (!is_white(ir.cl) || !is_white(ir.cr)) {
    read_ir();
    // Tilt left until both center sensors see white
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.cr) || is_white(ir.cl)) {
    read_ir();
    // Continue until both center sensors see black
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }

  read_ir();
  drive_forward_ms(150);
  stop_motors(200);
}

void turn_right() {
  drive_forward_ms(0);
  stop_motors(0);

  stop_motors(500);

  read_ir();

  while (!is_white(ir.cl) || !is_white(ir.cr)) {
    read_ir();
    // Tilt right until both center sensors see white
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, HIGH);
    digitalWrite(IN_B, LOW);

    analogWrite(EN_B, TURN_SPEED_LEFT);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }
  while (is_white(ir.cl) || is_white(ir.cr)) {
    read_ir();
    // Continue until both center sensors see black
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, HIGH);
    digitalWrite(IN_B, LOW);

    analogWrite(EN_B, TURN_SPEED_LEFT);
    digitalWrite(IN_C, LOW);
    digitalWrite(IN_D, HIGH);
  }

  read_ir();
  drive_forward_ms(150);
  stop_motors(200);
}

void u_turn() {
  read_ir();
  drive_forward_ms(300);
  stop_motors(0);
  read_ir();
  stop_motors(500);

  while (!is_white(ir.cl) || !is_white(ir.cr)) {
    read_ir();
    // Tilt left until both center sensors see white
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.cr) || is_white(ir.cl)) {
    read_ir();
    // Continue until both center sensors see black
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }

  stop_motors(500);
  drive_forward_ms(400);
  stop_motors(300);

  while (!is_white(ir.cl) || !is_white(ir.cr)) {
    read_ir();
    // Tilt left until both center sensors see white
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  while (is_white(ir.cr) || is_white(ir.cl)) {
    read_ir();
    // Continue until both center sensors see black
    analogWrite(EN_A, TURN_SPEED_LEFT);
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);

    analogWrite(EN_B, TURN_SPEED_RIGHT);
    digitalWrite(IN_C, HIGH);
    digitalWrite(IN_D, LOW);
  }
  stop_motors(500);

  read_ir();
}

// =========================
// Sensors
// =========================
void read_ultrasonic() {
  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dur = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (dur == 0) {
    distance_cm = -1;
    return;
  }

  int cm = (int)((dur / 2.0f) / SOUND_US_PER_CM);
  if (cm < MIN_CM || cm > MAX_CM) {
    distance_cm = -1;
    return;
  }
  distance_cm = cm;
}

void read_ir() {
  // Read values from each IR sensors
  ir.rr = analogRead(IR_PINS[0]);
  ir.cr = analogRead(IR_PINS[1]);
  ir.cl = analogRead(IR_PINS[2]);
  ir.ll = analogRead(IR_PINS[3]);
}

// =========================
// Buzzer and Display
// =========================
void beep_times(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
  }
}

void display_ir() {
  read_ultrasonic();
  read_ir();
  tft.setColor(WHITE);
  tft.print(String("Distance:"), 0, 0);
  tft.print(String(distance_cm) + "  ", 80, 0);

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
}