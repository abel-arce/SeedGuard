#include "ssd1306.h"
#include "nano_gfx.h"
#include <Wire.h>

#define I2C_ADDRESS 0x50
//*****************************
uint8_t state = 0;
uint8_t displayed = 0;
uint8_t Digits_in_display = 0;
uint8_t header = 255;

unsigned int address = 1;
// Buffers Data variable
uint8_t input_pin[] = {0,0,0,0,0};
uint8_t input_EEPROM[] = {0,0,0,0,0};

uint8_t Display_Slots[] = {0,8,16,24,32,40,48,56};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //Set Up Display
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_128x64_i2c_init();
  ssd1306_clearScreen();
  //
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  
  Wire.begin();  
  //eepromByteWrite(0x00,0xAA);    //writing 0x00 address key Value 0xAB = chip Used!! 
  //LockScreen();
  //DrawBounds();
  //ShowMenu();
  //ClearMenu();
}

void ClearSlot(uint8_t i){
    ssd1306_printFixed(8, Display_Slots[i],"                    ", STYLE_BOLD);
}

void ClearMenu(){
    ssd1306_printFixed(8, Display_Slots[2], "                  ", STYLE_BOLD);
    ssd1306_printFixed(8, Display_Slots[3], "                  ", STYLE_BOLD);
    ssd1306_printFixed(8, Display_Slots[4], "                  ", STYLE_BOLD);
    ssd1306_printFixed(8, Display_Slots[5], "                  ", STYLE_BOLD);
    ssd1306_printFixed(8, Display_Slots[6], "                  ", STYLE_BOLD);
}

void ShowMenu(){
    ssd1306_printFixed(36, Display_Slots[0], "SeedGuard", STYLE_BOLD);
    ssd1306_printFixed(10, Display_Slots[2], "Slot 1", STYLE_NORMAL);
    ssd1306_printFixed(10, Display_Slots[3], "Slot 2", STYLE_NORMAL);
    ssd1306_printFixed(10, Display_Slots[4], "Slot 3", STYLE_NORMAL);
    ssd1306_printFixed(10, Display_Slots[5], "Slot 4", STYLE_NORMAL);
    ssd1306_printFixed(10, Display_Slots[6], "Slot 5", STYLE_NORMAL);
}

void ShowChipError(){
    ssd1306_clearScreen();
    ssd1306_printFixed(28, Display_Slots[3], "NO CHIP FOUND!", STYLE_BOLD);
    ssd1306_printFixed(24, Display_Slots[7], "Checking chip...", STYLE_NORMAL);
}

void ShowCheckChip(){
  ClearSlot(7);
  ssd1306_printFixed(16, Display_Slots[7], "Checking chip...", STYLE_NORMAL);
}

void DrawBounds(){
    uint8_t slot_under_title = 10; 
    //Horizontal Bounds
    ssd1306_drawLine(3,slot_under_title, ssd1306_displayWidth() -3, slot_under_title);
    ssd1306_drawLine(3,ssd1306_displayHeight() - 3, ssd1306_displayWidth() -3, ssd1306_displayHeight() -3);

    //Vertical Bounds
    ssd1306_drawLine(3,slot_under_title, 3, ssd1306_displayHeight() -3);
    ssd1306_drawLine(ssd1306_displayWidth() - 3,slot_under_title, ssd1306_displayWidth() - 3, ssd1306_displayHeight() - 3);
}

void LockScreen(){
      ssd1306_clearScreen();
      ssd1306_printFixed(46, Display_Slots[2], "Locked", STYLE_BOLD);
      ssd1306_printFixed(32, Display_Slots[3], "Insert PIN", STYLE_NORMAL);

      //ssd1306_printFixed(46, Display_Slots[5], "*", STYLE_NORMAL);
      //ssd1306_printFixed(54, Display_Slots[5], "*", STYLE_NORMAL);
      //ssd1306_printFixed(62, Display_Slots[5], "*", STYLE_NORMAL);
      //ssd1306_printFixed(70, Display_Slots[5], "*", STYLE_NORMAL);
      //ssd1306_printFixed(78, Display_Slots[5], "*", STYLE_NORMAL);
}

void loop() {
  

  // Initial State
  if(!state){
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
        state = 2;
        Digits_in_display = 0;
        displayed = 0;
    }
  } //*****************

  //CHIP ALREADY USED
  if(state == 2){
    if(!displayed){
      LockScreen();
      displayed = 1;
    }
    ReadInput();
    if(Digits_in_display >= 5){
      if(pin_ok()){
        state = 3;
      }else{
        ClearSlot(5);
        ssd1306_printFixed(32, Display_Slots[5], "WRONG PING!", STYLE_NORMAL);
        delay(2000);
      }
      Digits_in_display = 0;
      displayed = 0;
    }
  } //*****************

  //OK state
  if(state == 3){
    if(!displayed){
      displayed = 1;
    }  
  }
  //********************

  //Checking chip status
  if(!CheckChip(0x00)){
    if(displayed < 2){
      ShowChipError();
      displayed = 2;
    }
    state = 99;
    Digits_in_display = 0;
  }else if(displayed == 2){
      ClearSlot(3);
      ClearSlot(7);
      ssd1306_printFixed(40, Display_Slots[3], "CHIP OK!", STYLE_BOLD);
      delay(3000);
      state = 2;
      displayed = 0;
  }
  //*****************
  // Debug Mode Reset State
    if(!digitalRead(A0) && !digitalRead(A1)){
      header = 0xFF;
      //displayed = 0;
    }
    if(!digitalRead(A2) && !digitalRead(A3)){
      header = 0x00;
      //displayed = 0;
    }
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
      input_pin[Digits_in_display] = 0x00;
      ssd1306_printFixed(46 + (Digits_in_display*8), Display_Slots[5], "*", STYLE_NORMAL);
      Digits_in_display++;
      delay(300);
    }
    if(!digitalRead(A1)){
      input_pin[Digits_in_display] = 0x01;
      ssd1306_printFixed(46 + (Digits_in_display*8), Display_Slots[5], "*", STYLE_NORMAL);
      Digits_in_display++;
      delay(300);
    }
    if(!digitalRead(A2)){
      input_pin[Digits_in_display] = 0x02;
      ssd1306_printFixed(46 + (Digits_in_display*8), Display_Slots[5], "*", STYLE_NORMAL);
      Digits_in_display++;
      delay(300);
    }
    if(!digitalRead(A3)){
      input_pin[Digits_in_display] = 0x03;
      ssd1306_printFixed(46 + (Digits_in_display*8), Display_Slots[5], "*", STYLE_NORMAL);
      Digits_in_display++;
      delay(300);
    }
  }
}

byte CheckChip(byte print){
  if(!ChipError()){
      if(print){
        //Chip OK!
      }
      return 1;
    }else{
      if(print){
        //Error on CHIP return 0
      }
      return 0;
    }
}

byte ChipError(){
  Wire.beginTransmission(I2C_ADDRESS);
  //return Wire.endTransmission();
  return header;
}
/*
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
}*/

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
