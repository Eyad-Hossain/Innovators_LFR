int button_read() {
int currentReading = digitalRead(sw);

  // Check if the pin state changed (button press, release, or bounce)
  if (currentReading != lastButtonState) {
    lastDebounceTime = millis(); // Reset timer on state shift
  }

  // If the reading has remained stable longer than the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    // If the button state has genuinely changed
    if (currentReading != buttonState) {
      buttonState = currentReading;

      // Trigger reset only when the confirmed state becomes LOW (pressed)
      if (buttonState == LOW) {
        Serial.println("Resetting system...");
        resetFunc(); // Jump to program start address
      }
    }
  }

  // Save the reading for comparison in the next iteration
  lastButtonState = currentReading;
}