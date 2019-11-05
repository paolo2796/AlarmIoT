

void setup()
{ 
  Serial.begin(9600);   //Apriamo la comunicazione con il monitor seriale
  SPI.begin(); 
  rfid.init();

}

void loop() {

   rfidRead();
}

void rfidRead(){
   slave = (EEPROM.read(0) == 255)?0:EEPROM.read(0);
  // Rileva un tag...
  if (rfid.isCard()) {
      // Legge il seriale...
      if (rfid.readCardSerial()) {
                sernum0 = rfid.serNum[0];
                sernum1 = rfid.serNum[1];
                sernum2 = rfid.serNum[2];
                sernum3 = rfid.serNum[3];
                sernum4 = rfid.serNum[4];
                // Se il seriale letto corrisponde con il seriale Master
                // attiva o disattiva la modalita Memorizzazione chiavi
                // e in più visualizza l'elenco della chiavi salvate... 
                if (sernum0 == masnum0 && sernum1 == masnum1 && sernum2 == masnum2 && sernum3 == masnum3 && sernum4 == masnum4) {
                    if (cardmas==0) {
                        Serial.println("CARD MASTER");
                        Serial.println("MODALITA' ADMIN");
                        cardmas = 1;
                        Serial.println("Chiavi slave: ");
                        Serial.println(slave);
                  }  
                  else { 
                    cardmas = 0;
                    Serial.println("USCITA MODALITA ADMIN");
                    delay(3000);
                  }
               }//end if
                
                // Se invece il seriale letto corrisponde con uno dei tre gruppi 
                // di chiavi memorizzate allora attiva o disattiva il Led
          else if ((sernum0 == EEPROM.read(1) && sernum1 == EEPROM.read(2) && sernum2 == EEPROM.read(3) && sernum3 == EEPROM.read(4) && sernum4 == EEPROM.read(5))
                   || (sernum0 == EEPROM.read(6) && sernum1 == EEPROM.read(7) && sernum2 == EEPROM.read(8) && sernum3 == EEPROM.read(9) && sernum4 == EEPROM.read(10))
                   || (sernum0 == EEPROM.read(11) && sernum1 == EEPROM.read(12) && sernum2 == EEPROM.read(13) && sernum3 == EEPROM.read(14) && sernum4 == EEPROM.read(15))) {
                              
                   Serial.println("CHIAVE SLAVE VALIDA");

                   
           } 
                // Se il seriale letto è diverso dal master e non è presente in memoria,
                // e se è attiva la modalita Memorizzazione chiavi, salva il seriale in memoria
                // come slave1, slave2 o slave3.
           else if (cardmas == 1 && slave == 0) {
              
                    Serial.println("Chiave rilevata!");
                    EEPROM.write(0, 1);
                    EEPROM.write(1, sernum0);
                    EEPROM.write(2, sernum1);
                    EEPROM.write(3, sernum2);
                    EEPROM.write(4, sernum3);
                    EEPROM.write(5, sernum4);
                    cardmas = 0;
                    delay(1000);
                                        
           }
           else if (cardmas == 1 && slave == 1) {
                         
                   Serial.println("Chiave rilevata!");
                   EEPROM.write(0, 2);
                   EEPROM.write(6, sernum0);
                   EEPROM.write(7, sernum1);
                   EEPROM.write(8, sernum2);
                   EEPROM.write(9, sernum3);
                   EEPROM.write(10, sernum4);
                   cardmas = 0;
                   delay(1000);
                   Serial.println("Slave 2 salvata!");
                    delay(3000);
            }
            else if (cardmas == 1 && slave == 2) {
                 Serial.println("Chiave rilevata!");
                 EEPROM.write(0, 3);
                 EEPROM.write(11, sernum0);
                 EEPROM.write(12, sernum1);
                 EEPROM.write(13, sernum2);
                 EEPROM.write(14, sernum3);
                 EEPROM.write(15, sernum4);
                 Serial.println("Slave 3 salvata!");
                 cardmas = 0;
                 delay(3000);
           }
         }

         delay(1000);
       }
                        
       rfid.halt();
}

/*
void touchScreenAdminMode(){
        digitalWrite(13, HIGH);
           TSPoint p = ts.getPoint();
           digitalWrite(13, LOW);
           // if sharing pins, you'll need to fix the directions of the touchscreen pins
           //pinMode(XP, OUTPUT);
           pinMode(XM, OUTPUT);
           pinMode(YP, OUTPUT);
           //pinMode(YM, OUTPUT);
           // we have some minimum pressure we consider 'valid'
           // pressure of 0 means no pressing!
          

          
           // Scale from ~0->4000 to tft.width using the calibration #'s
           
           if (p.z != -1) {
            p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
            p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
           //Serial.print("("); Serial.print(p.x); Serial.print(", ");
           //Serial.print(p.y); Serial.print(", ");
           //Serial.print(p.z); Serial.println(") ");
           }
           if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
            // scale from 0->1023 to tft.width
            p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
            p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
           } 
           
            //go thru all the buttons, checking if they were pressed
           for (uint8_t b=0; b<1; b++) {
               if (buttons[b].contains(p.x, p.y)) {
                Serial.print("Pressing: "); Serial.println(b);
                buttons[b].press(true); // tell the button it is pressed
               } 
               else {
                  buttons[b].press(false); // tell the button it is NOT pressed
               }
           }
          
           // now we can ask the buttons if their state has changed
           for (uint8_t b=0; b<1; b++) {
             if (buttons[b].justReleased()) {
              Serial.print("Released: "); Serial.println(b);
              buttons[b].drawButton(); // draw normal
             }
          
            if (buttons[b].justPressed()) {
              Serial.println("JUST PRESSED");
              buttons[b].drawButton(true); // draw invert!
            

             // we dont really check that the text field makes sense
             // just try to call
             if (b == 0) {
  
              //status(F("Calling"));
              Serial.print("Calling "); Serial.print(textfield);
             //fona.callPhone(textfield);
             }

             delay(100); // UI debouncing
           }
         }//end for

      
  
}


void checkLCDDriver(uint16_t identifier){


 if(identifier == 0x9325) {
  Serial.println(F("Found ILI9325 LCD driver"));
 } else if(identifier == 0x9328) {
  Serial.println(F("Found ILI9328 LCD driver"));
 } else if(identifier == 0x4535) {
  Serial.println(F("Found LGDP4535 LCD driver"));
 }else if(identifier == 0x7575) {
  Serial.println(F("Found HX8347G LCD driver"));
 } else if(identifier == 0x9341) {

  
    Serial.println(F("Found ILI9341 LCD driver"));
  
    tft.begin(identifier);
    tft.setRotation(0);
    tft.fillScreen(ILI9341_WHITE);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(TEXT_TSIZE);

    // create 'text field'
    tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_BLUE);
        printMessageField("BENVENUTO ADMIN");
    createMenuButtons();
 }
 else if(identifier == 0x7783) {
  Serial.println(F("Found ST7781 LCD driver"));
 }else if(identifier == 0x8230) {
  Serial.println(F("Found UC8230 LCD driver"));
 }
 else if(identifier == 0x8357) {
  Serial.println(F("Found HX8357D LCD driver"));
 } 
 else if(identifier==0x0101)
 {
  identifier=0x9341;
  Serial.println(F("Found 0x9341 LCD driver"));
 }
 else {
  
   Serial.print(F("Unknown LCD driver chip: "));
   Serial.println(identifier, HEX);
   Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
   Serial.println(F(" #define USE_ADAFRUIT_SHIELD_PINOUT"));
   Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
   Serial.println(F("If using the breakout board, it should NOT be #defined!"));
   Serial.println(F("Also if using the breakout, double-check that all wiring"));
   Serial.println(F("matches the tutorial."));
   identifier=0x9341;

   tft.begin(identifier);
   tft.setRotation(0);
   tft.fillScreen(ILI9341_NAVY);

    // create 'text field'
    tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);
 }

}


//Crea menu principale
void createMenuButtons() {
    // create buttons

    for (uint8_t row = 0; row < 1; row++) {
        buttons[row].initButton( & tft, 120,BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y), 150, BUTTON_H, ILI9341_WHITE, buttoncolors[row],ILI9341_WHITE,buttonlabels[row], BUTTON_TEXTSIZE);
        buttons[row].drawButton();
    }
}

void printMessageField(String message){
                    tft.setCursor(TEXT_X + 2, TEXT_Y+10);
                    tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
                    tft.setTextSize(TEXT_TSIZE);
                    tft.print(message);
}


*/








