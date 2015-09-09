void updateDisplay() {
  display.clearDisplay();
  for (int i = 0; i < display.width(); i++) {
    int barOffset = int(map(phaseOffset, maxFreq, minFreq, 0, display.width() / 2));
    if (i % barOffset <= barOffset/2) {
      // fill in the column if it's not
      display.drawLine(i, 0, i, display.height()-14, WHITE);
    } 
  }

  display.setCursor(2, display.height() - 12);
  int freq = int(map(phaseOffset, minFreq, maxFreq, 37.5, 1200));
  display.print(analogRead(dutyPot));
  display.setCursor(42, display.height() - 12);
  display.println(dutyCycle);
  display.display();

}
