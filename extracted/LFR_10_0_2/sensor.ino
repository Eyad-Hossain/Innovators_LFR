void reading() {
  sensor = 0;
  sum = 0;

  int rawValue;

  for (byte i = 0; i < 13; i++) {

    rawValue = (analogRead(i) > threshold[i]) ^ i_mode;
    // Pack Pin 0 into MSB (bit 12) down to Pin 12 into LSB (bit 0)
    sensor |= ((uint16_t)rawValue << (12 - i));
  }

  if (sensor == lastSensorData) {
    if (stableCounter < 65535) {
      stableCounter++;
    }
  } else {
    // Print bits 12 down to 0 (Pin 0 will now print first/leftmost)
    for (int i = 12; i >= 0; i--) {
      Serial.print(bitRead(sensor, i));
    }
    Serial.println();  // Moves to the next line

    // Populate and send the struct over UART
    packet.sensorData = sensor;
    packet.stableCounter = stableCounter;

    Serial3.write((uint8_t*)&packet, sizeof(packet));

    lastSensorData = sensor;
    stableCounter = 0;
  }

  sum = __builtin_popcount(sensor);
}

void show_value() {
  for (int i = 5; i >= 0; i--) {
    Serial.print(String(s[i], 10) + " ");
  }
  Serial.println(" ");
}

void analog_reading() {
  while (1) {

    for (int i = 0; i < 13; i++) {
      s[i] = analogRead(i);
      Serial.print(String(s[i]) + " ");
    }

    Serial.println();
    delay(500);
  }
}

void digital_reading() {
  while (1) {
    reading();
    for (int i = 5; i >= 0; i--)
      Serial.print(String(s[i]) + " ");
    Serial.println(sum);
  }
}

void break_fun() {
  motor(-10 * lsp, -10 * lsp);
  delay(50);
  motor(0, 0);
  delay(50);
}
