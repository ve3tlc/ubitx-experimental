/** Menus
 *  The Radio menus are accessed by tapping on the function button. 
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press. 
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter. 
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */



int menuBand(int btn){

  int knob = 0;
    
  if (!btn){
   PRINTLINE1("Band Select?");
   return 0;
  }

  PRINTLINE1("Press to confirm");
  //wait for the button menu select button to be lifted)
  while (btnDown())
    delay(50);
  delay(50);    
  ritDisable();

  while(!btnDown()){

    knob = enc_read();
    if (knob != 0){
      if (knob < 0 && frequency > 3000000l)
        setFrequency(frequency - 200000l);
      if (knob > 0 && frequency < 30000000l)
        setFrequency(frequency + 200000l);
      if (frequency > 10000000l)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
    }
    delay(20);
  }
  
  while(btnDown())
    delay(50);
  delay(50);
  
  PRINTLINE1("");
  updateDisplay();
  menuOn = 0;

  return 1;
}

void menuVfoToggle(int btn){
  
  if (!btn){
    if (vfoActive == VFO_A)
      PRINTLINE1("Select VFO B?   ");
    else
      PRINTLINE1("Select VFO A?   ");    
  }
  else {
      if (vfoActive == VFO_B){
        vfoB = frequency;
        EEPROM.put(VFO_B, frequency);
        vfoActive = VFO_A;
        PRINTLINE1("Selected VFO A  ");
        frequency = vfoA;
      }
      else {
        vfoA = frequency;
        EEPROM.put(VFO_A, frequency);
        vfoActive = VFO_B;
        PRINTLINE1("Selected VFO B  ");      
        frequency = vfoB;
      }
      
      ritDisable();
      setFrequency(frequency);
      if (frequency >= 10000000l)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
      PRINTLINE1("");
      delay(1000);
      //exit the menu
      menuOn = 0;
  }
}

void menuRitToggle(int btn){
  if (!btn){
    if (ritOn == 1)
      PRINTLINE1("RIT:On, Off?   ");
    else
      PRINTLINE1("RIT:Off, On?   ");
  }
  else {
      if (ritOn == 0){
        PRINTLINE1("RIT is ON");
        //enable RIT so the current frequency is used at transmit
        ritEnable(frequency);
      }
      else{
        PRINTLINE1("RIT is OFF");
        ritDisable();
      }
      menuOn = 0;
      delay(500);
      PRINTLINE1("");
    updateDisplay();
  }
}

void menuSidebandToggle(int btn){
  if (!btn){
    if (isUSB == true)
      PRINTLINE1("Select LSB?");
    else
      PRINTLINE1("Select USB?");
  }
  else {
      if (isUSB == true){
        isUSB = false;
        PRINTLINE1("LSB Selected");
        delay(500);
        PRINTLINE1("");
      }
      else {
        isUSB = true;
        PRINTLINE1("USB Selected");
        delay(500);
        PRINTLINE1("");
      }
    
    updateDisplay();
    menuOn = 0;
  }
}

/**
 * The calibration routines are not normally shown in the menu as they are rarely used
 * They can be enabled by choosing this menu option
 */
void menuSetup(int btn){
  if (!btn){
    if (!modeCalibrate)
      PRINTLINE1("Setup On?");
    else
      PRINTLINE1("Setup Off?");
  }else {
    if (!modeCalibrate){
      modeCalibrate = true;
      PRINTLINE1("Setup:On   ");
    }
    else {
      modeCalibrate = false;
      PRINTLINE1("Setup:Off   ");      
    }
   delay(2000);
   PRINTLINE1("");
   menuOn = 0;
  }
}

void menuExit(int btn){

  if (!btn){
      PRINTLINE1("Exit Menu?      ");
  }
  else{
      PRINTLINE1("Exiting menu");
      delay(300);
      PRINTLINE1("");
      updateDisplay();
      menuOn = 0;
  }
}

int menuCWSpeed(int btn){
    int knob = 0;
    int wpm;

    wpm = 1200/cwSpeed;
     
    if (!btn){
     strcpy(b, "CW:");
     itoa(wpm,c, 10);
     strcat(b, c);
     strcat(b, "WPM Change?");
     PRINTLINE1(b);
     return 0;
    }

    PRINTLINE2("Press PTT to set");
    strcpy(b, "WPM:");
    itoa(wpm,c, 10);
    strcat(b, c);
    PRINTLINE1(b);
    delay(300);

    while(!btnDown() && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (wpm > 3 && knob < 0)
          wpm--;
        if (wpm < 50 && knob > 0)
          wpm++;

        strcpy(b, "WPM:");
        itoa(wpm,c, 10);
        strcat(b, c);
        PRINTLINE1(b);
      }
      //abort if this button is down
      if (btnDown())
        //re-enable the clock1 and clock 2
        break;
    }
    
    //save the setting
    if (digitalRead(PTT) == LOW){
      PRINTLINE1("CW Speed set!");
      cwSpeed = 1200/wpm;
      EEPROM.put(CW_SPEED, cwSpeed);
      delay(2000);
    }
    PRINTLINE1("");
    menuOn = 0;
    return cwSpeed;
}



/**
 * Take a deep breath, math(ematics) ahead
 * The 25 mhz oscillator is multiplied by 35 to run the vco at 875 mhz
 * This is divided by a number to generate different frequencies.
 * If we divide it by 875, we will get 1 mhz signal
 * So, if the vco is shifted up by 875 hz, the generated frequency of 1 mhz is shifted by 1 hz (875/875)
 * At 12 Mhz, the carrier will needed to be shifted down by 12 hz for every 875 hz of shift up of the vco
 * 
 */

 //this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;

int factoryCalibration(int btn){
  int knob = 0;

  //keep clear of any previous button press
  while (btnDown())
    delay(100);
  delay(100);
   
  if (!btn){
    PRINTLINE1("Set Calibration?");
    return 0;
  }

  calibration = 0;

  isUSB = true;

  //turn off the second local oscillator and the bfo
  si5351_set_calibration(calibration);
  startTx(TX_CW);
  si5351bx_setfreq(2, 10000000l); 
  
  strcpy(b, "#1 10 MHz cal:");
  ltoa(calibration/8750, c, 10);
  strcat(b, c);
  PRINTLINE1(b);     

  while (!btnDown())
  {
    if (digitalRead(PTT) == LOW && !keyDown)
      cwKeydown();
    if (digitalRead(PTT)  == HIGH && keyDown)
      cwKeyUp();
      
    knob = enc_read();

    if (knob > 0)
      calibration += 875;
    else if (knob < 0)
      calibration -= 875;
    else 
      continue; //don't update the frequency or the display
      
    si5351_set_calibration(calibration);
    si5351bx_setfreq(2, 10000000l);
    strcpy(b, "#1 10 MHz cal:");
    ltoa(calibration/8750, c, 10);
    strcat(b, c);
    PRINTLINE1(b);     
  }

  cwTimeout = 0;
  keyDown = 0;
  stopTx();

  PRINTLINE1("Calibration set!");
  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  setFrequency(frequency);    
  updateDisplay();

  while(btnDown())
    delay(50);
  delay(100);

  return 1;
}

int menuSetupCalibration(int btn){
  int knob = 0;
  int32_t prev_calibration;
   
  if (!btn){
    PRINTLINE1("Set Calibration?");
    return 0;
  }

  PRINTLINE2("Set to Zero-beat,");
  PRINTLINE1("press PTT to save");
  delay(1000);
  
  prev_calibration = calibration;
  calibration = 0;
  si5351_set_calibration(calibration);
  setFrequency(frequency);    
  
  strcpy(b, "cal:");
  ltoa(calibration/8750, c, 10);
  strcat(b, c);
  PRINTLINE1(b);     

  while (digitalRead(PTT) == HIGH && !btnDown())
  {
    knob = enc_read();

    if (knob > 0){
      calibration += 8750;
      usbCarrier += 120;
    }
    else if (knob < 0){
      calibration -= 8750;
      usbCarrier -= 120;
    }
    else
      continue; //don't update the frequency or the display

    si5351_set_calibration(calibration);
    si5351bx_setfreq(0, usbCarrier);
    setFrequency(frequency);    

    strcpy(b, "cal:");
    ltoa(calibration/8750, c, 10);
    strcat(b, c);
    PRINTLINE1(b);     
  }
  
  //save the setting
  if (digitalRead(PTT) == LOW){
    PRINTLINE2("Calibration set!");
    PRINTLINE1("Set Carrier now");
    EEPROM.put(MASTER_CAL, calibration);
    delay(2000);
  }
  else
    calibration = prev_calibration;

  PRINTLINE1("");
  initOscillators();
  //si5351_set_calibration(calibration);
  setFrequency(frequency);    
  updateDisplay();
  menuOn = 0;

  return calibration;
}


void printCarrierFreq(unsigned long freq){

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(freq, b, DEC);
  
  strncat(c, b, 2);
  strcat(c, ".");
  strncat(c, &b[2], 3);
  strcat(c, ".");
  strncat(c, &b[5], 1);
  PRINTLINE1(c);    
}

void menuSetupCarrier(int btn){
  int knob = 0;
  unsigned long prevCarrier;
   
  if (!btn){
      PRINTLINE1("Set the BFO");
    return;
  }

  prevCarrier = usbCarrier;
  PRINTLINE2("Tune to best Signal");  
  PRINTLINE1("PTT to confirm. ");
  delay(1000);

  usbCarrier = 11995000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  //disable all clock 1 and clock 2 
  while (digitalRead(PTT) == HIGH && !btnDown())
  {
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; //don't update the frequency or the display
      
    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);
    
    delay(100);
  }

  //save the setting
  if (digitalRead(PTT) == LOW){
    PRINTLINE1("Carrier set!    ");
    EEPROM.put(USB_CAL, usbCarrier);
    delay(1000);
  }
  else 
    usbCarrier = prevCarrier;

  si5351bx_setfreq(0, usbCarrier);          
  setFrequency(frequency);    
  updateDisplay();
  PRINTLINE1("");
  menuOn = 0; 
}

void menuSetupCwTone(int btn){
    int knob = 0;
    int prev_sideTone;
     
    if (!btn){
        PRINTLINE1("Change CW Tone");
      return;
    }

    prev_sideTone = sideTone;
    PRINTLINE2("Tune CW tone");  
    PRINTLINE1("PTT to confirm. ");
    delay(1000);
    tone(CW_TONE, sideTone);

    //disable all clock 1 and clock 2 
    while (digitalRead(PTT) == LOW || !btnDown())
    {
      knob = enc_read();

      if (knob > 0 && sideTone < 2000)
        sideTone += 10;
      else if (knob < 0 && sideTone > 100 )
        sideTone -= 10;
      else
        continue; //don't update the frequency or the display
        
      tone(CW_TONE, sideTone);
      itoa(sideTone, b, 10);
      PRINTLINE1(b);

      delay(100);
    }
    noTone(CW_TONE);
    //save the setting
    if (digitalRead(PTT) == LOW){
      PRINTLINE1("Sidetone set!    ");
      EEPROM.put(CW_SIDETONE, usbCarrier);
      delay(2000);
    }
    else
      sideTone = prev_sideTone;
    
    PRINTLINE1("");  
    updateDisplay(); 
    menuOn = 0; 
 }

void doMenu(){
  int select=0, i,btnState;

  //wait for the button to be raised up
  while(btnDown())
    delay(50);
  delay(50);  //debounce
  
  menuOn = 2;
  
  while (menuOn){
    i = enc_read();
    btnState = btnDown();

    if (i > 0){
      if (modeCalibrate && select + i < 110)
        select += i;
      if (!modeCalibrate && select + i < 70)
        select += i;
    }
    if (i < 0 && select - i >= 0)
      select += i;      //caught ya, i is already -ve here, so you add it

    if (select < 10)
      menuBand(btnState);
    else if (select < 20)
      menuRitToggle(btnState);
    else if (select < 30)
      menuVfoToggle(btnState);
    else if (select < 40)
      menuSidebandToggle(btnState);
    else if (select < 50)
      menuCWSpeed(btnState);
    else if (select < 60)
      menuSetup(btnState);
    else if (select < 70 && !modeCalibrate)
      menuExit(btnState);
    else if (select < 80 && modeCalibrate)
      menuSetupCalibration(btnState);   //crystal
    else if (select < 90 && modeCalibrate)
      menuSetupCarrier(btnState);       //lsb
    else if (select < 100 && modeCalibrate)
      menuSetupCwTone(btnState);
    else if (select < 110 && modeCalibrate)
      menuExit(btnState);  
  }

  //debounce the button
  while(btnDown())
    delay(50);
  delay(50);
}

