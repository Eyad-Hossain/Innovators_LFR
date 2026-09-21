#include <EEPROM.h>
#define L1 28   // Left Motor MB2
#define L2 22   // Left Motor MB1
#define R1 24   // Right Motor MA1
#define R2 26   // Right Motor MA2
#define enaL 2  // Left Motor Enable Pin EA
#define enaR 3  //Right Motor Enable Pin EB
#define sw 23
#define light 6
#define stop_timer 100
#define u_turn_delay 200
const int espResetPin = 31;  // Connected to ESP32 EN/RST

char turn = 'l', flag = 's', turn_detected = '0', v_turn = 'l';

// 1. Define the identical structure structure
struct __attribute__((__packed__)) DataPacket {
  uint16_t sensorData;     // 1 byte (Holds the 6 bits)
  uint16_t stableCounter;  // 2 bytes (Holds counter up to 65,535)
};

DataPacket packet;

uint16_t lastSensorData = 0xFF;
uint16_t stableCounter = 0;

bool brake_flag = 0, straight_flag = 0, pos_flag = 0, end_flag = 0, turn_SL = 0, turn_SR = 0, inverse_flag = 0, all_white_flag = 0, turn_flag = 1, break_flag = 1, counting_on = 0, left_turn_detected = 0, right_turn_detected = 0;

int s[14], maxi[14], mini[14], sum, W = 0;
int position[6] = { 1, 2, 4, 8, 16, 32 };
int threshold[14] = { 150, 255, 225, 150, 200, 250, 250, 250, 150, 150, 215, 150, 250 }, sensor;  // Right to Left
int lsp = 20, rsp = 20, tsp = 170;
float line_prop = 3;
int pos, i_mode = 0, inverse = 100, brake_threshold = 50, turn_count = 0;

uint32_t m1, m2, m3;
int lbounce = millis();
int dbounce = 100, pos_time, T = 0, R = 0, L = 0, SL = 0, SR = 0;
unsigned long brake_timer, straight_timer, pos_timer, end_time, turn_time = 0, inverse_time = 0, break_timer = 0, right_timer = 0, left_timer = 0, all_white_timer = 0, inverse_time_counter = 0, hold_break_time = 0, hold_break_time1 = 0, inverse_over = 0, break_count = 0, brake_timer1 = 0;

// Declare a function pointer to address 0 for software reset
void (*resetFunc)(void) = 0;

// Variables for millis()-based non-blocking debounce logic
int buttonState = HIGH;      // Current confirmed button state
int lastButtonState = HIGH;  // Previous raw reading from the pin
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 50 ms debounce threshold


void setup() {

  // Set ADC prescaler to 16 (Fast ADC Mode)
  // This modifies the ADCSRA register bits directly
  bitSet(ADCSRA, ADPS2);    // Set bit 2
  bitClear(ADCSRA, ADPS1);  // Clear bit 1
  bitClear(ADCSRA, ADPS0);  // Clear bit 0

  pinMode(espResetPin, OUTPUT);
  digitalWrite(espResetPin, HIGH);  // Normal state

  pinMode(sw, INPUT_PULLUP);
  pinMode(light, OUTPUT);

  Serial.begin(115200);
  // Serial3.begin(115200);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(30, OUTPUT);
  pinMode(34, OUTPUT);

  digitalWrite(30, LOW);

  //delay(1500);
  // Trigger reset
  digitalWrite(espResetPin, LOW);   // Hold reset active
  delay(200);                       // Wait 100 milliseconds
  digitalWrite(espResetPin, HIGH);  // Release reset
  //delay(1500);
  //analog_reading();
  //Serial.print("A");
}

void loop() {

  //motor(150, 150);
  //reading();
  line_follow();

  if (sum) {
    if (R) R--;
    if (L) L--;
    if (T) T--;
    if (SL) SL--;
    if (SR) SR--;
    if (W) W--;
    if (break_count) break_count--;
  }

  if (W > 6) {
    turn_SR = 0;
    turn_SL = 0;
    SR = 0;
    SL = 0;
  }




  //delay(1000);
  // if(millis()-lbounce>=dbounce){
  //    int r = button_read();
  //    //lbounce=millis();
  // //Serial.print(r);
  //   if (r != 0) {
  //     Serial.println(r);
  //     delay(1000);
  //      if (r == 2) cal();
  //      else if (r == 1) line_follow();
  //      //digital_reading();
  // }
  // }
}
