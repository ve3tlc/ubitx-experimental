
/**
 * This procedure is only for those who have a signal generator/transceiver tuned to exactly 7.150 and a dummy load 
 */

void btnWaitForClick(){
  while(!btnDown())
    delay(50);
  while(btnDown())
    delay(50);
 delay(50);
}

void factory_alignment(){
        
  factoryCalibration(1);

  if (calibration == 0){
    PRINTLINE1("Setup Aborted");
    return;
  }

  //move it away to 7.160 for an LSB signal
  setFrequency(7160000l);
  updateDisplay();
  PRINTLINE1("#2 BFO");
  delay(1000);

  usbCarrier = 11994999l;
  menuSetupCarrier(1);

  if (usbCarrier == 11994999l){
    PRINTLINE1("Setup Aborted");
    return;
  }

  
  PRINTLINE1("#3:Test 3.5MHz");
  isUSB = false;
  setFrequency(3500000l);
  updateDisplay();

  while (!btnDown()){
    checkPTT();
    delay(100);
  }

  btnWaitForClick();
  PRINTLINE1("#4:Test 7MHz");

  setFrequency(7150000l);
  updateDisplay();
  while (!btnDown()){
    checkPTT();
    delay(100);
  }

  btnWaitForClick();
  PRINTLINE1("#5:Test 14MHz");

  isUSB = true;
  setFrequency(14000000l);
  updateDisplay();
  while (!btnDown()){
    checkPTT();
    delay(100);
  }

  btnWaitForClick();
  PRINTLINE1("#6:Test 28MHz");

  setFrequency(28000000l);
  updateDisplay();
  while (!btnDown()){
    checkPTT();
    delay(100);
  }

  PRINTLINE1("Alignment done");
  delay(1000);

  isUSB = false;
  setFrequency(7150000l);
  updateDisplay();  
  
}

