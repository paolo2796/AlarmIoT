/**
* ACCENSIONE LED CON RFID 
* Per maggiori info: www.progettiarduino.com
* Importante, collegare il lettore RFID ai pin di Arduino uno come segue:
* MOSI: Pin 11 / ICSP-4
* MISO: Pin 12 / ICSP-1
* SCK: Pin 13 / ISCP-3
* SDA: Pin 10
* RST: Pin 9
*/

    // FOR ARDUINO MEGA MOSI  MISO  SCK SS (slave)  SS (master) Level

//Mega1280 or Mega2560  51     50   52  53  - 5V
#include <SPI.h>
#include <RFID.h>
#include <EEPROM.h>


#define SS_PIN 53
#define RST_PIN 49

// Codice della chiave master.
#define masnum0 199 
#define masnum1 3
#define masnum2 151
#define masnum3 27
#define masnum4 72



#define resetkey 6

RFID rfid(SS_PIN, RST_PIN); 


boolean cardmas = 0; // Variabile chiave master
int slave=0; // Contatore delle chiavi salvate
    
int sernum0;
int sernum1;
int sernum2;
int sernum3;
int sernum4;




/* START TOUCHSCREEN PROPERTY */

#include <Adafruit_GFX.h> // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>



// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
// D0 connects to digital pin 8 (Notice these are
// D1 connects to digital pin 9 NOT in order!)
// D2 connects to digital pin 2
// D3 connects to digital pin 3
// D4 connects to digital pin 4
// D5 connects to digital pin 5
// D6 connects to digital pin 6
// D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
// Color definitions
#define ILI9341_BLACK 0x0000 /* 0, 0, 0 */
#define ILI9341_NAVY 0x000F /* 0, 0, 128 */
#define ILI9341_DARKGREEN 0x03E0 /* 0, 128, 0 */
#define ILI9341_DARKCYAN 0x03EF /* 0, 128, 128 */
#define ILI9341_MAROON 0x7800 /* 128, 0, 0 */
#define ILI9341_PURPLE 0x780F /* 128, 0, 128 */
#define ILI9341_OLIVE 0x7BE0 /* 128, 128, 0 */
#define ILI9341_LIGHTGREY 0xC618 /* 192, 192, 192 */
#define ILI9341_DARKGREY 0x7BEF /* 128, 128, 128 */
#define ILI9341_BLUE 0x001F /* 0, 0, 255 */
#define ILI9341_GREEN 0x07E0 /* 0, 255, 0 */
#define ILI9341_CYAN 0x07FF /* 0, 255, 255 */
#define ILI9341_RED 0xF800 /* 255, 0, 0 */
#define ILI9341_MAGENTA 0xF81F /* 255, 0, 255 */
#define ILI9341_YELLOW 0xFFE0 /* 255, 255, 0 */
#define ILI9341_WHITE 0xFFFF /* 255, 255, 255 */
#define ILI9341_ORANGE 0xFD20 /* 255, 165, 0 */
#define ILI9341_GREENYELLOW 0xAFE5 /* 173, 255, 47 */
#define ILI9341_PINK 0xF81F

/******************* UI details */
#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2
// text box where numbers go
#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 230
#define TEXT_H 50
#define TEXT_TSIZE 2
#define TEXT_TCOLOR ILI9341_MAGENTA
// the data (phone #) we store in the textfield
#define TEXT_LEN 30
char textfield[TEXT_LEN+1]="";
uint8_t textfield_i=0;
#define YP A3 // must be an analog pin, use "An" notation!
#define XM A2 // must be an analog pin, use "An" notation!
#define YM 9 // can be a digital pin
#define XP 8 // can be a digital pin



#define TS_MINX 100
#define TS_MAXX 920
#define TS_MINY 70
#define TS_MAXY 900
// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#define MINPRESSURE 10
#define MAXPRESSURE 1000

uint16_t identifier;




TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
//Adafruit_TFTLCD tft;
Adafruit_GFX_Button buttons[1];
/* create 15 buttons, in classic candybar phone style */
char buttonlabels[1][14] = {"SENSORI"};
uint16_t buttoncolors[2] = {ILI9341_DARKGREEN, ILI9341_RED};


/* END TOUCH SCREEN PROPERTY */

void setup()
{ 
  Serial.begin(9600);   //Apriamo la comunicazione con il monitor seriale
  SPI.begin(); 
  rfid.init();

  // For touch screen
  tft.reset();
  identifier = tft.readID();


 
  

}

void loop() {

   rfidRead();
   if (sernum0 == masnum0 && sernum1 == masnum1 && sernum2 == masnum2 && sernum3 == masnum3 && sernum4 == masnum4) {
      touchScreenAdminMode();
   }
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
                        //Start display admin mode
                        checkLCDDriver(identifier);

   
                  }  
                  else { 
                    cardmas = 0;
                    Serial.println("USCITA MODALITA ADMIN");
                    tft.reset();
                    delay(3000);
                  }
               }//end if
                
                // Se invece il seriale letto corrisponde con uno dei tre gruppi 
                // di chiavi memorizzate allora attiva o disattiva il Led
          else if ((sernum0 == EEPROM.read(1) && sernum1 == EEPROM.read(2) && sernum2 == EEPROM.read(3) && sernum3 == EEPROM.read(4) && sernum4 == EEPROM.read(5))
                   || (sernum0 == EEPROM.read(6) && sernum1 == EEPROM.read(7) && sernum2 == EEPROM.read(8) && sernum3 == EEPROM.read(9) && sernum4 == EEPROM.read(10))
                   || (sernum0 == EEPROM.read(11) && sernum1 == EEPROM.read(12) && sernum2 == EEPROM.read(13) && sernum3 == EEPROM.read(14) && sernum4 == EEPROM.read(15))) {
                              
                   Serial.println("CHIAVE SLAVE VALIDA");
                   printMessageField("CHIAVE SLAVE VALIDA");

                   
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
                    Serial.println("Slave 1 salvata!");
                    // update the current text field
                    printMessageField("SLAVE 1 salvato!");
                    delay(3000);
                                        
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
                    // update the current text field
                    printMessageField("SLAVE 2 salvato!");
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
                 cardmas = 0;
                 // update the current text field
                 printMessageField("SLAVE 3 salvato!");
                 delay(3000);
           }
         }
       }                 
       rfid.halt();
  
}


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









