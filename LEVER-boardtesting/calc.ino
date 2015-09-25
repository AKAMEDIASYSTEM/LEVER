
// native map func uses int math
float floatmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void calculateFeedback(){
  
}


/*
 * prototype for TTY communcation protocol
 * 
 * Right now, there would be a second microcontroller like rfduino
 * handling BLE communication and OLED updating, making teensy DAC
 * signal generatino more stable
 * 
 * rf-to-teensy will want to send:
 * - turn on (ie, hand detected or on-phone stack updated)
 * - change freq
 * - change waveform
 * - create new waveform (wavetable?)(do this last, it's not critical)
 * - change amplitude
 * - change duty cycle (or leave this to the feedback onboard the teensy?)
 * 
 * teensy-to-rf messages:
 * - not sure yet, maybe "i am done executing last command?"
 * 
 * phone-to-rf messages:
 * - stack change
 * - keepalive/periodic update
 * 
 * rf-to-phone messages:
 * - battery status
 * - user just interacted, clear/dismiss stack (necessary? user-settable?)
 * 
 * 
 * 
 */
