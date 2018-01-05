/**
 * The CAT protocol is used by many radios to provide remote control to comptuers through
 * the serial port.
 * 
 * This is a modified version of the original code that liberally
 * borrowed from other GPLicensed works like hamlib.
 * 
 * WARNING : This should be considered an unstable version as it has not been tested.   
 */

// The next 4 functions are needed to implement the CAT protocol, which
// uses 4-bit BCD formatting.
//
byte setHighNibble(byte b,byte v) {
  // Clear the high nibble
  b &= 0x0f;
  // Set the high nibble
  return b | ((v & 0x0f) << 4);
}

byte setLowNibble(byte b,byte v) {
  // Clear the low nibble
  b &= 0xf0;
  // Set the low nibble
  return b | (v & 0x0f);
}

byte getHighNibble(byte b) {
  return (b >> 4) & 0x0f;
}

byte getLowNibble(byte b) {
  return b & 0x0f;
}

// Takes a number and produces the requested number of decimal digits, staring
// from the least significant digit.  
//
void getDecimalDigits(unsigned long number,byte* result,int digits) {
  for (int i = 0; i < digits; i++) {
    // "Mask off" (in a decimal sense) the LSD and return it
    result[i] = number % 10;
    // "Shift right" (in a decimal sense)
    number /= 10;
  }
}

// Takes a frequency and writes it into the CAT command buffer in BCD form.
//
void writeFreq(unsigned long freq,byte* cmd) {
  // Convert the frequency to a set of decimal digits. We are taking 9 digits
  // so that we can get up to 999 MHz. But the protocol doesn't care about the
  // LSD (1's place), so we ignore that digit.
  byte digits[9];
  getDecimalDigits(freq,digits,9);
  // Start from the LSB and get each nibble 
  cmd[3] = setLowNibble(cmd[3],digits[1]);
  cmd[3] = setHighNibble(cmd[3],digits[2]);
  cmd[2] = setLowNibble(cmd[2],digits[3]);
  cmd[2] = setHighNibble(cmd[2],digits[4]);
  cmd[1] = setLowNibble(cmd[1],digits[5]);
  cmd[1] = setHighNibble(cmd[1],digits[6]);
  cmd[0] = setLowNibble(cmd[0],digits[7]);
  cmd[0] = setHighNibble(cmd[0],digits[8]);  
}

// This function takes a frequency that is encoded using 4 bytes of BCD
// representation and turns it into a long measured in Hz.
//
// [12][34][56][78] = 123.45678? Mhz
//
unsigned long readFreq(byte* cmd) {
    // Pull off each of the digits
    byte d7 = getHighNibble(cmd[0]);
    byte d6 = getLowNibble(cmd[0]);
    byte d5 = getHighNibble(cmd[1]);
    byte d4 = getLowNibble(cmd[1]); 
    byte d3 = getHighNibble(cmd[2]);
    byte d2 = getLowNibble(cmd[2]); 
    byte d1 = getHighNibble(cmd[3]);
    byte d0 = getLowNibble(cmd[3]); 
    return  
      (unsigned long)d7 * 100000000L +
      (unsigned long)d6 * 10000000L +
      (unsigned long)d5 * 1000000L + 
      (unsigned long)d4 * 100000L + 
      (unsigned long)d3 * 10000L + 
      (unsigned long)d2 * 1000L + 
      (unsigned long)d1 * 100L + 
      (unsigned long)d0 * 10L; 
}

/**
 * Responds to all the cat commands, emulates FT-817
 */
  
void processCATCommand(byte* cmd) {

  byte response[5];
  unsigned long f;
  
  // Debugging code, enable it to fix the cat implementation  
  count++;

  /* command response protocol based on information from KA7OEI website http://www.ka7oei.com/ft817_meow.html */
  switch (cmd[4])
  {
    /*
    Lock On -- This command is equivalent to turning the dial/panel lock on.
    Byte1..Byte4 = don't care, Byte5= 0x00      
    Returns 0x00 if it was unlocked or 0xF0 if it was already locked

    -- still need to implement the global variable panelLock to lock the front panel tuning and menus.
    */
    case 0x00: // CMD = LOCK ON
      if (panelLock == 0)
      {
        panelLock = 1;
        response[0]=0;
      }
      else
        response[0]=0xF0;
      Serial.write(response, 1);
      break;

    /*
    Set Frequency -- This commands sets the current frequency. The frequency is set
    using 4 BCD bytes. To set a frequency such as 435.12345 MHz, 
    data bytes 1-4 would be [43][51][23][45] followed by the the set frequency command [01]
    Byte1  = 100/10 MHz
    Byte2  = 1 MHz/100 kHz
    Byte3  = 10/1 kHz
    Byte4  = 100/10 Hz
    Byte5  = 0x01
    There is no value returned in response to this command.
    */
    case 0x01: // CMD = SET FREQUENCY
      f = readFreq(cmd);
      setFrequency(f);   
      updateDisplay();
      break;

    /*
    Read Frequency and mode -- This command returns five bytes. The first 4 bytes contain
    the current frequency in the same format as the Set frequency command. The last byte
    contains the current mode.
    Byte1..Byte4 = don't care, Byte5= 0x03 
    Bytes 1 thru 5 are returned in response to this command.
    Byte1  = 100/10 MHz
    Byte2  = 1 MHz/100 kHz
    Byte3  = 10/1 kHz
    Byte4  = 100/10 Hz
    Byte5  = 00 - LSB, 01 - USB, 02 - CW, 03 - CWR, 04, (AM, 08 - FM, 0A - DIG, 0C - PKT)
    */
    case 0x03:  // CMD = READ FREQ AND MODE STATUS
      writeFreq(frequency,response); // Put the frequency into the buffer
      if (isUSB)
        response[4] = 0x01; //USB
      else
        response[4] = 0x00; //LSB
      Serial.write(response,5);
      PRINTLINE1("cat:getfreq");
      break;
      
    /*
    Clarifier On -- This command turns RIT on.
    Byte1..Byte4 = don't care, Byte5= 0x05      
    Returns 0x00 if the RIT was off or 0xF0 if it was already on when the command was received.
    */
    case 0x05: // SET CLARIFIER (RIT) ON
      if (ritOn == 0)
      {
        response[0] = 0;
        if (catClarifierFreq > 0)
           ritEnable(catClarifierFreq);
      } 
      else
        response[0] = 0xf0;
     
      Serial.write(response,1);
      break;

    /*
    Set Operating Mode -- This command sets the operating mode. Only LSB,USB,CW are supported.
    Byte1 = 00 - LSB, 01 - USB, 02 - CW, 03 - CWR, 04, (AM, 08 - FM, 0A - DIG, 0C - PKT)  
    Byte2..Byte4 = don't care, Byte5 = 0x07      
    There is no value returned in response to this command.
    */
    case 0x07: // SET OPERATING MODE
      if (cmd[0] == 0x00 || cmd[0] == 0x03)
        isUSB = 0;
      else
        isUSB = 1;
      response[0] = 0x00;
      Serial.write(response, 1);
      setFrequency(frequency);
      break;

    /*
    Set PTT ON -- This "keys" the UBITX. In CW, this sets the radio to transmit mode, and
    keys the transmitter.
    Byte1..Byte4 = don't care, Byte5= 0x08      
    Returns 0x00 if the PTT was off or 0xF0 if it was already on when the command was received.
    */
    case 0x08: // // CMD = PTT ON
      if (!inTx) {
        response[0] = 0;
        txCAT = true;
        startTx(TX_SSB);
        updateDisplay();
      } else {
        response[0] = 0xf0;
      } 
      Serial.write(response,1);
      PRINTLINE1("rx > tx");
      break;

    /*
    Read TX keyed state -- This command returns
    Byte1..Byte4 = don't care, Byte5= 0x10 
    Returns 0x00 if not in transmit mode or 0xF0 if it is in transmit mode.
    */
    case 0x10: // CMD = Read TX keyed state (undocumented in manual)
      if (!inTx)
        response[0] = 0;
      else
        response[0] = 0xf0; 
      Serial.write(response,1);
      updateDisplay();
      break;

    /*
    Lock Off -- This command is equivalent to turning the dial/panel lock off.
    Byte1..Byte4 =  don't care, Byte5= 0x80      
    Returns 0x00 if it was locked or 0xF0 if it was already unlocked.
    */
    case 0x80: // CMD = LOCK OFF
      if (panelLock == 1)
      {
        panelLock = 0;
        response[0]=0X00;       
      }
      else
        response[0]=0X80;
      Serial.write(response, 1);
      break;

    /*
    Toggle VFO -- This command toggles between VFO-A and VFO-B.
    Byte1..Byte4 = don't care, Byte5= 0x81      
    There is no value returned in response to this command.
    */
    case 0x81: // CMD = VFO - A/B
     if (vfoActive == VFO_B){
        vfoActive = VFO_A;
        frequency = vfoA;
      }
      else
      {
        vfoActive = VFO_B;   
        frequency = vfoB;
      }    
      ritDisable();
      setFrequency(frequency);
      if (frequency >= 10000000l)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
      break;

    /*
    Clarifier Off -- This command turns RIT off.
    Byte1..Byte4 = don't care, Byte5 = 0x85      
    Returns 0x00 if the RIT was on or 0xF0 if it was already off when the command was received.
    */
    case 0x85: // SET CLARIFIER (RIT) OFF
      if (ritOn == 1)
      {
         response[0] = 0x00;
         ritDisable();
      }
      else
         response[0] = 0xf0;
  
      Serial.write(response,1);     
      break;

    /*
    Set PTT OFF -- This command puts the UBITX into receive mode..
    Byte1..Byte4 = don't care, Byte5 = 0x88      
    Returns 0x00 if the PTT was on or 0xF0 if it was already off when the command was received.       
    */
    case 0x88: // CMD = PTT OFF
      if (inTx){
        stopTx();
        txCAT = false;
        response[0] = 0x00;
      }
      else
        response[0] = 0xf0;
      PRINTLINE1("tx > rx");
      Serial.write(response,1);
      break;

    /*
    Read Receiver Status -- This command returns a byte containing the receiver status.
    Byte1..Byte4 = don't care, Byte5= 0xE7 
    This command returns one byte containing the receiver status. Its contents are valid only when the UBITX is in receive mode and it should be ignored when transmitting.
    Bits (0-3) of the returned byte indicate the current S-meter reading. 00 refers to an S-Zero reading, 04 = S4, 09 = S9, 0A = "10 over," 0B = "20 over" and so on up to 0F.
    Bit 4 contains no useful information.
    Bit 5 is 0 in non-FM modes (which is always the case for the UBITX).
    Bit 6 is 0 if the CTCSS or DCS is turned off  (which is always the case for the UBITX).
    Bit 7 is 0 if there is a signal present, or 1 if the receiver is squelched.
    */      
    case 0xe7: // CMD = READ RX STATUS
      response[0] = 0x00;
      Serial.write(response,1);
      break;

    /*
    Set Clarifier Frequency -- This command sets the RIT direction and offset amount.
    Byte1 = Clarifier Direction, 00 = Positive (+) offset. Any other value sets a negative (-) offset
    Byte2 = don't care
    Byte3 = BCD byte containing the 10 kHz and 1 kHz digits
    Byte4 = BCD byte containing the 100 Hz and 10 Hz digits
    Byte5 = 0xF5      
    There is no value returned in response to this command.
    */      
    case 0xf5: // CMD = CLARIFIER (RIT) frequency
      if (cmd[0] == 0)
      {
        cmd[0] = 0;
        cmd[1] = 0;
        catClarifierFreq = frequency + readFreq(cmd);
      }
      else
      {
        cmd[0] = 0;
        cmd[1] = 0;
        catClarifierFreq = frequency - readFreq(cmd);
      }
      break;

    /*
    Read Transmitter Status -- This command returns a byte containing the Transmitter status.
    Byte1  .. Byte4 = don't care, Byte5= 0xF7 
    This command returns one byte containing the Transmitter status. Its contents are valid only when the UBITX is in transmit mode and it should be ignored when receiving.
    Bits (0-3) of the returned byte indicate Power Output.
    Bit 4 contains no useful information.
    Bit 5 is 1 if the SPLIT mode is off (which is always the case for the UBITX).
    Bit 6 is 0 if the SWR is acceptably low, and 1 if it is too high.
    Bit 7 indicates the current PTT status: 0 = Unkeyed, 1 = Keyed.
    */
    case 0xf7: // CMD = read TX status
      if (inTx)
        response[0] = 0xA1;
      else
        response[0] = 0x21;     
      Serial.write(response,1);
      break;
      
    default:
      //somehow, get this to print the four bytes
      ultoa(*((unsigned long *)cmd), c, 16);
      itoa(cmd[4], b, 16);
      strcat(b, ":");
      strcat(b, c);
      PRINTLINE1(b);
      response[0] = 0x00;
      Serial.write(response[0]);
      break;
  } // end switch
}

void checkCAT(){
  static byte cat[5]; 
  byte i;

  /* since all command sequences are 5 bytes in length
  * just wait until we receive at least 5 bytes over the serial connection.
  */
  if (Serial.available() < 5)
    return;

  /*
  * read the next 5 bytes received into the cmd array
  */
  for (i = 0; i < 5; i++)
    cat[i] = Serial.read();

  /*
  * Process the command received
  */
  processCATCommand(cat);
}


