#include <UTFT.h>
extern uint8_t SmallFont[];
     //(Model,SDA,SCK,CS,RST,A0)
UTFT tft(ST7735, 6, 7, 3, 4, 5);
#define White VGA_WHITE
#define Black VGA_BLACK

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
int distanceCm;

int readUltrasonicCM() {
  // Trigger 10 µs pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure echo pulse
  unsigned long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) return -1;  // timeout / no echo

  // Convert time->distance (round trip: divide by 2)
  int cm = (int)((duration / 2.0f) / SOUND_US_PER_CM);

  if (cm < MIN_CM || cm > MAX_CM) return -1;
  return cm;
}

// --- Median of n (odd) samples to reduce noise ---
int readUltrasonicCM_Median(uint8_t n = 5) {
  if (n < 3 || (n % 2) == 0) n = 5;  // enforce odd >=3
  int vals[9];                       // supports up to 9; increase if you need larger n
  if (n > 9) n = 9;

  uint8_t count = 0;
  for (uint8_t i = 0; i < n; i++) {
    int v = readUltrasonicCM();
    if (v != -1) vals[count++] = v;
    delay(10);  // brief settle between pings
  }
  if (count == 0) return -1;

  // simple insertion sort
  for (uint8_t i = 1; i < count; i++) {
    int key = vals[i], j = i;
    while (j > 0 && vals[j - 1] > key) {
      vals[j] = vals[j - 1];
      j--;
    }
    vals[j] = key;
  }
  return vals[count / 2];
}

void readIR() {
  for (int i = 0; i < numIR; i++) {
    irVals[i] = analogRead(irPins[i]);
  }
}

void setup() {
  tft.InitLCD();
  tft.setFont(SmallFont);
  tft.clrScr();
  tft.fillScr(Black);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  for (int i = 0; i < numIR; i++) {
    pinMode(irPins[i], INPUT);
  }
}

void loop() {
  int distance = readUltrasonicCM();
  // int distance = readUltrasonicCM_Median(5);
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
}