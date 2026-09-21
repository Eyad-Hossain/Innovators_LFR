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
const int sensorPins[NUM_SENSORS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

int threshold[NUM_SENSORS] = {
  150, 255, 225, 150, 200, 250, 250, 250, 150, 150, 215, 150, 250
};

float Kp = 0.15;
float Ki = 0.0002;
float Kd = 0.8;

float Kp_sharp = 0.25;
float Kd_sharp = 1.2;

int baseSpeed = 180;
int maxSpeed = 255;

const int sensorWeights[NUM_SENSORS] = {
  -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6
};

struct PIDController {
  float Kp, Ki, Kd;
  float integral = 0;
  float lastError = 0;
  unsigned long lastTime = 0;
  float integralLimit = 10000;

  PIDController(float p, float i, float d) : Kp(p), Ki(i), Kd(d) {}

  void reset() {
    integral = 0;
    lastError = 0;
    lastTime = millis();
  }

  void setGains(float p, float i, float d) {
    Kp = p; Ki = i; Kd = d;
  }

  float compute(float error) {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    if (dt <= 0) dt = 0.001f;

    integral += error * dt;
    integral = constrain(integral, -integralLimit, integralLimit);

    float derivative = (error - lastError) / dt;
    lastError = error;

    return Kp * error + Ki * integral + Kd * derivative;
  }
};

PIDController pid(Kp, Ki, Kd);

enum State { NORMAL, ZIGZAG, JUNCTION, GAP };
State currentState = NORMAL;

uint16_t sensorBits = 0;
int activeCount = 0;

bool zigzagMode = false;
unsigned long lastSignChange = 0;

bool wasLineLost = false;
unsigned long lineLostTime = 0;
const unsigned long GAP_TIMEOUT = 150;

int turnCount = 0;

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
  
  pid.lastTime = millis();
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

int computeLinePosition() {
  long weightedSum = 0;
  int activeSensors = 0;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorBits & (1 << (NUM_SENSORS - 1 - i))) {
      weightedSum += sensorWeights[i];
      activeSensors++;
    }
  }

  if (activeSensors == 0) return 0;

  float position = (float)weightedSum / activeSensors;
  return (int)(position * 100.0f / 6.0f);
}

void updateState() {
  int position = computeLinePosition();
  long error = (activeCount > 0) ? position : pid.lastError;

  if (activeCount == 0) {
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

  if ((error > 80 && pid.lastError < -80) || (error < -80 && pid.lastError > 80)) {
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

void runPID() {
  int position = computeLinePosition();
  long error = (activeCount > 0) ? position : pid.lastError;

  float Kp_use = zigzagMode ? Kp_sharp : Kp;
  float Kd_use = zigzagMode ? Kd_sharp : Kd;
  int speed_use = zigzagMode ? baseSpeed * 0.7 : baseSpeed;

  pid.setGains(Kp_use, Ki, Kd_use);

  float correction = pid.compute(error);

  int leftSpeed = constrain(speed_use + correction, -maxSpeed, maxSpeed);
  int rightSpeed = constrain(speed_use - correction, -maxSpeed, maxSpeed);

  setMotors(leftSpeed, rightSpeed);

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
    if (activeCount < 5 && computeLinePosition() != 0) break;
    setMotors(turnDir * 200, -turnDir * 200);
    delay(10);
  }

  pid.reset();
}

void setMotors(int left, int right) {
  if (left >= 0) {
    digitalWrite(L1, HIGH);
    digitalWrite(L2, LOW);
  } else {
    digitalWrite(L1, LOW);
    digitalWrite(L2, HIGH);
    left = -left;
  }

  if (right >= 0) {
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

void calibrateSensors() {
  int maxi[NUM_SENSORS];
  int mini[NUM_SENSORS];

  for (int i = 0; i < NUM_SENSORS; i++) {
    maxi[i] = 0;
    mini[i] = 1023;
  }

  setMotors(80, -80);
  for (int j = 0; j < 5000; j++) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      int val = analogRead(sensorPins[i]);
      maxi[i] = max(maxi[i], val);
      mini[i] = min(mini[i], val);
    }
  }
  setMotors(0, 0);

  for (int i = 0; i < NUM_SENSORS; i++) {
    threshold[i] = (maxi[i] - mini[i]) * 0.3 + mini[i];
    EEPROM.write(i, threshold[i] / 4);
    delay(10);
  }
}