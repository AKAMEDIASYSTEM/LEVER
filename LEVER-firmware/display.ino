void updateDisplay() {
  for (int i = 0; i < display.width(); i++) {
    int barOffset = int(map(phaseOffset, maxFreq, minFreq, 0, display.width() / 2));
    if (i % barOffset <= barOffset/2) {
      // fill in the column if it's not
      display.drawLine(i, 0, i, display.height()-14, WHITE);
    } 
  }
//  display.setCursor(0, 0);
//  display.println(tempread);
//  display.drawRect(0, display.height(), display.width(), display.height() - 12, BLACK);
//  display.setCursor(2, display.height() - 12);
  int freq = int(map(phaseOffset, minFreq, maxFreq, 37.5, 1200));
//  display.println(freq);
  display.display();

}
