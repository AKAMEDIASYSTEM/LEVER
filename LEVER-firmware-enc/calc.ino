
// native map func uses int math
float floatmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void calculateFeedback() {
//  int input = analogRead(feedbackPin);
  // do we integrate over time here?
  // the trick is that the signal here is a waveform and not a very static value...maybe an interrupt to accumulate?
  // i think summing over the whole waveform is possible, will still get me a net diff in current

//  in response, change the dutyCycle up or down...
  
  /* I think: we measure the current going across a precision resistor (at high voltage, so: how?! isolation amp?)
  * Then we see how far under/over target we are, and proportionally adjust duty cycle
  * (higher duty cycle, i think, means higher voltage but make sure it's not going into saturation)
  *
  */
}

