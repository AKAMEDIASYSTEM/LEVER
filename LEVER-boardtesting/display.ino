void updateDisplay() {
  display.clearDisplay();
  for (int i = 0; i < display.width(); i++) {
    int barOffset = int(floatmap(phaseOffset, maxFreq, minFreq, 0, display.width() / 2));
    if (i % barOffset <= barOffset / 2) {
      // fill in the column if it's not
      display.drawLine(i, 0, i, display.height() - 14, WHITE);
    }
  }

  display.setCursor(2, display.height() - 12);
  int freq = int(floatmap(phaseOffset, minFreq, maxFreq, 55, 855)); // measured for minFreq 0.0125 * 1000.0 and maxFreq 0.2 * 1000.0
  display.print(waveLabels[waveType]);
  display.setCursor(42, display.height() - 12);
//  display.print(dutyCycle*100);
//  display.print("%");
  display.setCursor(72, display.height() - 12);
  display.print(freq);
//  display.setCursor(102, display.height() - 12);
//  display.print(int(DACamplitude/maxAmpl));
  display.display();

}
