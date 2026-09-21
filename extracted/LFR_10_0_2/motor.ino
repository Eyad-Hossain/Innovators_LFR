void motor(int a, int b) {
  if (a > 0) {
    digitalWrite(L1, 1);
    digitalWrite(L2, 0);
  }

  else {
    a = -(a);
    digitalWrite(L1, 0);
    digitalWrite(L2, 1);
  }

  if (b > 0) {
    digitalWrite(R1, 1);
    digitalWrite(R2, 0);
  }

  else {
    b = -b;
    digitalWrite(R1, 0);
    digitalWrite(R2, 1);
  }

  if (a > 255) a = 255;
  if (b > 255) b = 255;

  analogWrite(enaL, a);
  analogWrite(enaR, b);

  // analogWrite(enaL, 255);
  // analogWrite(enaR, 255);
  //  Serial.print(a);
  //  Serial.print(" ");
  //  Serial.println(b);
}
