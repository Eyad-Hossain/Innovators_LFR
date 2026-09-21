#include <EEPROM.h>

#define L1 28
#define L2 22
#define R1 24
#define R2 26
#define enaL 2
#define enaR 3
#define sw 23
#define light 6
#define espResetPin 31

const int NUM_SENSORS = 13;
int sensorPins[NUM_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

int threshold[NUM_SENSORS] = {150, 255, 225, 150, 200, 250, 250, 250, 150, 150, 215, 150, 250};

float Kp = 0.15;
float Ki = 0.0002;
float Kd = 0.8;

float Kp_sharp = 0.25;
float Kd_sharp = 1.2;

int baseSpeed = 180;
int maxSpeed = 255;

int sensorWeights[NUM_SENSORS] = {-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6};

long lastError = 0;
long integral = 0;
unsigned long lastTime = 0;

bool zigzagMode = false;
unsigned long lastSignChange = 0;

bool wasLineLost = false;
unsigned long lineLostTime = 0;
const unsigned long GAP_TIMEOUT = 150;

enum State { NORMAL, ZIGZAG, JUNCTION, GAP };
State currentState = NORMAL;

uint16_t sensorBits = 0;
int activeCount = 0;

void setup() {
  bitSet(ADCSRA, ADPS2);
  bitClear(ADCSRA, ADPS1);
  bitClear(ADCSRA, ADPS0);

  pinMode(espResetPin, OUTPUT);
  digitalWrite(espResetPin, HIGH);

  pinMode(sw, INPUT_PULLUP);
  pinMode(light, OUTPUT);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);

  digitalWrite(espResetPin, LOW);
  delay(200);
  digitalWrite(espResetPin, HIGH);

  Serial.begin(115200);
  lastTime = millis();
}

void loop() {
  readSensors();
  updateState();
  runPID();
}

void readSensors() {
  sensorBits = 0;
  activeCount = 0;
  
  for (int i = 0; i < NUM_SENSORS; i++) {
    int raw = analogRead(sensorPins[i]);
    bool onLine = (raw > threshold[i]);
    if (onLine) {
      sensorBits |= (1 << (NUM_SENSORS - 1 - i));
      activeCount++;
    }
  }
}

void updateState() {
  int pos = getLinePosition();
  long error = (pos >= 0) ? (pos - 5000) : lastError;

  if (pos == -1) {
    if (!wasLineLost) {
      lineLostTime = millis();
      wasLineLost = true;
    }
    currentState = GAP;
    return;
  }
  wasLineLost = false;

  if (activeCount >= 9) {
    currentState = JUNCTION;
    return;
  }

  if ((error > 800 && lastError < -800) || (error < -800 && lastError > 800)) {
    if (millis() - lastSignChange < 400) {
      zigzagMode = true;
      currentState = ZIGZAG;
    }
    lastSignChange = millis();
  }
  if (millis() - lastSignChange > 600) {
    zigzagMode = false;
  }

  if (currentState != ZIGZAG) {
    currentState = NORMAL;
  }
}

int getLinePosition() {
  long weightedSum = 0;
  int sum = 0;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorBits & (1 << (NUM_SENSORS - 1 - i))) {
      weightedSum += sensorWeights[i] * 1000;
      sum++;
    }
  }

  if (sum == 0) return -1;

  return (int)(weightedSum / sum) + 5000;
}

void runPID() {
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  int pos = getLinePosition();
  long error;

  if (pos == -1) {
    error = lastError;
  } else {
    error = pos - 5000;
    if (currentState != GAP) {
      integral += error * dt;
      integral = constrain(integral, -10000, 10000);
    }
  }

  long derivative = (error - lastError) / dt;

  float Kp_use = zigzagMode ? Kp_sharp : Kp;
  float Kd_use = zigzagMode ? Kd_sharp : Kd;
  int speed_use = zigzagMode ? baseSpeed * 0.7 : baseSpeed;

  float output = Kp_use * error + Ki * integral + Kd_use * derivative;

  int leftSpeed = constrain(speed_use + output, -maxSpeed, maxSpeed);
  int rightSpeed = constrain(speed_use - output, -maxSpeed, maxSpeed);

  setMotors(leftSpeed, rightSpeed);

  lastError = error;

  if (currentState == JUNCTION) {
    handleJunction();
  }
}

void handleJunction() {
  setMotors(0, 0);
  delay(100);
  
  int turnDir = (turnCount % 2 == 0) ? 1 : -1;
  turnCount++;
  
  unsigned long start = millis();
  while (millis() - start < 500) {
    readSensors();
    if (activeCount < 5 && getLinePosition() != -1) break;
    setMotors(turnDir * 200, -turnDir * 200);
    delay(10);
  }
  
  integral = 0;
  lastError = 0;
}

int turnCount = 0;

void setMotors(int left, int right) {
  if (left > 0) {
    digitalWrite(L1, HIGH);
    digitalWrite(L2, LOW);
  } else {
    digitalWrite(L1, LOW);
    digitalWrite(L2, HIGH);
    left = -left;
  }

  if (right > 0) {
    digitalWrite(R1, HIGH);
    digitalWrite(R2, LOW);
  } else {
    digitalWrite(R1, LOW);
    digitalWrite(R2, HIGH);
    right = -right;
  }

  left = constrain(left, 0, 255);
  right = constrain(right, 0, 255);

  analogWrite(enaL, left);
  analogWrite(enaR, right);
}