 #include <LiquidCrystal.h>
 #include <Wire.h>

 #define I2C_ADDRESS 0x50
 //***** LCD Pin Variables *****
 uint8_t RS_LCD_pin=2;
 uint8_t E_LCD_pin=3;
 //*** Data Bus *****
 uint8_t D4_LCD_pin=4;
 uint8_t D5_LCD_pin=5;
 uint8_t D6_LCD_pin=6;
 uint8_t D7_LCD_pin=7;
//*****************************
uint8_t state = 0;
uint8_t displayed = 0;
uint8_t Digits_in_display = 0;
uint8_t header = 255;

unsigned int address = 1;
// Buffers Data variable
uint8_t input_pin[] = {0,0,0,0,0};
uint8_t input_EEPROM[] = {0,0,0,0,0};

LiquidCrystal lcd(RS_LCD_pin, E_LCD_pin, D4_LCD_pin, D5_LCD_pin, D6_LCD_pin, D7_LCD_pin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ///Serial.println("LCD Custom Controller!");
  lcd.begin(16, 2);
  lcd.home();

  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  
  Wire.begin();
  //Pin setted!
  byte byteVal_1 = 2;
  byte byteVal_2 = 0;
  byte byteVal_3 = 2;
  byte byteVal_4 = 3;
  byte byteVal_5 = 0;
  
  //eepromByteWrite(0x00,0xAA);    //writing 0x00 address key Value 0xAB = chip Used!! 
  
}

void loop() {
  // Initial State
  if(!state){
    lcd.print("  <|SeedGuard|>");
    delay(3000);  
    lcd.setCursor(0, 1);
    lcd.print("Checking chip...");
    delay(500);
    
    header = 0xFF;
    if(!ChipError())
      header = eepromByteRead(0x00);        // Reading header check value
    
    if(header == 0xAB){
      state = 2;
      displayed = 0;
    }else{
      state = 1;
      displayed = 0;
    }
  } //*****************

  //NO PIN SET on CHIP
  if(state == 1){
    if(!displayed){
      systemOut(0x00);
      lcd.print("Set new PIN");
      systemOut(0x01);
      lcd.print("Insert Pin:");
      lcd.setCursor(11, 1);
      lcd.blink();
      displayed = 1;
    }
    ReadInput();
    if(Digits_in_display >= 5){
        //write PIN on Memory
        eepromByteWrite(0x01,input_pin[0]);
        eepromByteWrite(0x02,input_pin[1]);
        eepromByteWrite(0x03,input_pin[2]);
        eepromByteWrite(0x04,input_pin[3]);
        eepromByteWrite(0x05,input_pin[4]);
        eepromByteWrite(0x00,0xAB); // Set Chip as Used
        //
        systemOut(0x01);
        systemOut(0x00);
        lcd.print("PIN set OK!");
        delay(3000);
        state = 2;
        Digits_in_display = 0;
        displayed = 0;
    }
  } //*****************

  //CHIP ALREADY USED
  if(state == 2){
    if(!displayed){
      systemOut(0x00);
      lcd.print("  <|SeedGuard|>");
      systemOut(0x01);
      lcd.print("Insert PIN:");
      lcd.setCursor(11, 1);
      lcd.blink();
      displayed = 1;
    }
    ReadInput();
    if(Digits_in_display >= 5){
      if(pin_ok()){
        systemOut(0x01);
        lcd.print("  PIN OK!!");
        delay(3000);
        state = 3;
      }else{
        systemOut(0x01);
        lcd.print("  WRONG PIN!!");
        delay(3000);
      }
      Digits_in_display = 0;
      displayed = 0;
    }
  } //*****************

  //Error state
  if(state == 3){
    if(!displayed){
      systemOut(0x01);
      systemOut(0x00);
      lcd.print("  Select Slot   ");
      systemOut(0x01);
      lcd.blink();
      displayed = 1;
    }

    //Debug Mode
    if(!digitalRead(A0) && !digitalRead(A1)){
      state = 1;
      displayed = 0;
    }
      
  }
  //********************

  //Checking chip status
  if(!CheckChip(0x00)){
    if(displayed < 2){
      systemOut(0x00);
      lcd.print("Checking chip...");
      systemOut(0x01);
      lcd.print("....CHIP OFF....");
      displayed = 2;
    }
    Digits_in_display = 0;
  }else if(displayed == 2){
      displayed = 0;
  }
  //*****************
}


//*********** FUNCTIONS *************
byte pin_ok(){
  input_EEPROM[0] = eepromByteRead(0x01);
  input_EEPROM[1] = eepromByteRead(0x02);
  input_EEPROM[2] = eepromByteRead(0x03);
  input_EEPROM[3] = eepromByteRead(0x04);
  input_EEPROM[4] = eepromByteRead(0x05);
  if(input_EEPROM[0] == input_pin[0] && input_EEPROM[1] == input_pin[1] && input_EEPROM[2] == input_pin[2] && input_EEPROM[3] == input_pin[3] && input_EEPROM[4] == input_pin[4]){
      return 1;
  }else{
      return 0;
  }
}

void ReadInput(){
  if(Digits_in_display < 5){
    if(!digitalRead(A0)){
      lcd.print("0");
      delay(250);
      input_pin[Digits_in_display] = 0x00;
      Digits_in_display++;
    }
    if(!digitalRead(A1)){
      lcd.print("1");
      delay(250);
      input_pin[Digits_in_display] = 0x01;
      Digits_in_display++;
    }
    if(!digitalRead(A2)){
      lcd.print("2");
      delay(250);
      input_pin[Digits_in_display] = 0x02;
      Digits_in_display++;
    }
    if(!digitalRead(A3)){
      lcd.print("3");
      delay(250);
      input_pin[Digits_in_display] = 0x03;
      Digits_in_display++;
    }
  }
}

byte CheckChip(byte print){
  if(!ChipError()){
      //Serial.println("EEPROM Connected!");
      if(print){
        systemOut(1);
        lcd.print("Chip OK!");
        delay(500);
        systemOut(1);
        lcd.print("Insert Pin:");
        lcd.setCursor(11, 1);
        lcd.blink();
      }
      return 1;
    }else{
      //Serial.println("EEPROM ERROR");
      if(print){
        systemOut(1);
        lcd.print(" Error on Chip! ");
      }
      return 0;
    }
}

byte ChipError(){
  Wire.beginTransmission(I2C_ADDRESS);
  return Wire.endTransmission();
}

void systemOut(uint8_t i){
  if(i){
      lcd.setCursor(0, 1);
  }else{
      lcd.setCursor(0, 0);
  }
  lcd.print("                ");
  delay(250);
  if(i){
      lcd.setCursor(0, 1);
  }else{
      lcd.setCursor(0, 0);
  }
}

void eepromByteWrite(unsigned int addr, byte byteToWrite){
  Wire.beginTransmission(I2C_ADDRESS);
  //Wire.write((byte)(addr>>8));
  Wire.write((byte)(addr&0xFF));
  Wire.write(byteToWrite);
  Wire.endTransmission();
  delay(5); // important!
}

int eepromByteRead(unsigned int addr){
  int byteToRead;
  Wire.beginTransmission(I2C_ADDRESS);
  //Wire.write((byte)(addr>>8));
  Wire.write((byte)(addr&0xFF));
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDRESS, 1);
  byteToRead = Wire.read();
  return byteToRead;
}
