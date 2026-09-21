void cal() {
  for (int i = 0; i < 6; i++) {
    maxi[i] = 0;
    mini[i] = 1023;
  }
  motor(80, -80);
  for (int j = 0; j < 5000; j++) {
    for (int i = 0; i < 6; i++) {
      s[i] = analogRead(i);
      maxi[i] = max(maxi[i], s[i]);
      mini[i] = min(mini[i], s[i]);
    }
  }
  motor(0, 0);
  for (int i = 0; i < 6; i++) {
    threshold[i] = (maxi[i] - mini[i]) * 0.3 + mini[i];
    EEPROM.write(i, threshold[i] / 4);  delay(10);
    EEPROM.write(i + 6, maxi[i] / 4);   delay(10);
    EEPROM.write(i + 12, mini[i] / 4);  delay(10);
  }
}