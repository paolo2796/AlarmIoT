#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RFID.h>
#include <EEPROM.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <DS1302.h>




/**
  ACCENSIONE LED CON RFID
  Importante, collegare il lettore RFID ai pin di Arduino uno come segue:
  MOSI: Pin 11 / ICSP-4
  MISO: Pin 12 / ICSP-1
  SCK: Pin 13 / ISCP-3
  SDA: Pin 10
  RST: Pin 9
*/

// FOR ARDUINO MEGA MOSI  MISO  SCK SS (slave)  SS (master) Level
#define SS_PIN 10
#define RST_PIN 9

// Codice della chiave master.
#define masnum0 199
#define masnum1 3
#define masnum2 151
#define masnum3 27
#define masnum4 72



#define ARDUINO_CLIENT_ID "centralina_mc"
#define CONNECTOR "mqtt"
#define  SUB_ALLARME_KEYNFC_REMOVE "casa/allarme/keynfc/remove"
#define  SUB_ALLARME_STATO "casa/allarme/stato"
#define  SUB_ALLARME_CLOCK_SET "casa/allarme/clock/set"
#define  SUB_ALLARME_CLOCK_GET "casa/allarme/clock/request"

/* DISPlAY LCD */
//#define pins:
#define I2C_ADDR    0x27 // LCD address (make sure you run the I2C scanner to verify our LCD address)
#define Rs_pin  0        // Assign pins between I2C and LCD
#define Rw_pin  1
#define En_pin  2
#define BACKLIGHT_PIN 3
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

const int ledBlue = 6;
const int ledWhite = 4;

IPAddress server(192, 168, 43, 175);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// DS1302:  CE pin    -> Arduino Digital 2
//          I/O pin   -> Arduino Digital 3
//          SCLK pin  -> Arduino Digital 7

// Init the DS1302
DS1302 rtc(2, 3, 7);
// Init a Time-data structure
Time t;
String clockProgramming="none";


RFID rfid(SS_PIN, RST_PIN);
boolean cardmas = 0; // Variabile chiave master
int slave = 0; // Contatore delle chiavi salvate


int sernum0;
int sernum1;
int sernum2;
int sernum3;
int sernum4;


enum AlarmStatus {
  ACTIVE,
  DISABLED,
  ALARMED
};

AlarmStatus statusAlarm = DISABLED;


char ssid[] = "afg";        // your network SSID (name)
char pass[] = "paoletto";    // your network password (use for WPA, or use as key for WEP)






void initClock(){
  
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);

  rtc.setTime(11, 30, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(2, 12, 2019);   // Set the date to August 6th, 2010
   

}


void updateTime(){
  
  // Get data from the DS1302
  t = rtc.getTime();

  lcd.setCursor(0,3);
  lcd.print(String(t.date, DEC));
  lcd.print(" ");
  lcd.print(rtc.getMonthStr());
  lcd.setCursor(11,3);

  lcd.print(t.hour, DEC);
  lcd.print(":");
  lcd.print(t.min);

}

void checkClockProgramming(){
    
    // Get data from the DS1302
    t = rtc.getTime();
  
    String timeCurrent = String(t.hour);
    timeCurrent.concat(":");
    timeCurrent.concat(t.min);

    if(timeCurrent.equals(clockProgramming)){
      if(statusAlarm==ACTIVE || statusAlarm == ALARMED)
        return;
        
        statusAlarm=ACTIVE;
        lcd.setCursor(0,0);
        lcd.print("                                                  ");
        lcd.setCursor(0,0);
        lcd.print("ALLARME ATTIVO");  
        client.publish("casa/allarme/stato","{\"client_id\":\"" ARDUINO_CLIENT_ID  "\",\"data\":\"ACTIVE\"}",2);

    }
}

void initPins() {

  pinMode(ledWhite, OUTPUT);
  digitalWrite(ledWhite, HIGH);
  pinMode(ledBlue, OUTPUT);

}


/* END RFID  PROPERTY */





void setup() {

  //Serial.begin(9600);
  initDisplayLCD();
  lcd.print("Inizializ. WiFi");
  initWifi();  
  initPins();
  initClock();
  lcd.clear();
  SPI.begin();
  rfid.init();
  initClientMQTT();
  lcd.clear();
  lcd.print("BENVENUTO");




 EEPROM.write(0,0);
 EEPROM.write(1,0);
 EEPROM.write(2,0);
 EEPROM.write(3,0);
 EEPROM.write(4,0);
 EEPROM.write(5,0);  


}



void initDisplayLCD(){
  // set up backlight and turn on module:
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4); //My LCD is 20x4
  lcd.setCursor(5,1);
  
}




void loop() {

  
  checkConnectWifi();
  checkConnectMqtt();
  
  blinkLed();
  updateTime();
  checkClockProgramming();
  
  //verifico lettura rfid
  rfidRead();

  //maintain connection mqtt
  client.loop();

  delay(500);

}




void initClientMQTT(){
  
  
    client.setServer(server, 1883);
    client.setCallback(callback);


     while (!client.connected()) {
          //Serial.println("Connecting to MQTT...");
       
          if (client.connect(ARDUINO_CLIENT_ID,"paolo", "paoletto")) {
       
            //Serial.println("connected");  
           // Subscribe
           client.subscribe(SUB_ALLARME_STATO);
           client.subscribe(SUB_ALLARME_KEYNFC_REMOVE);
           client.subscribe(SUB_ALLARME_CLOCK_GET);
           client.subscribe(SUB_ALLARME_CLOCK_SET);

          } else {
       
            //Serial.print("failed with state ");
            //Serial.print(client.state());
            delay(500);
       
          }
      }

}

/* Event's MQTT client */

 void callback(char* topic, byte* payload, unsigned int length) {
 
    //Serial.print("Message arrived in topic: ");
    //Serial.println(topic);
   
    //Serial.println("Message:");
    String message = "";
    for (int i = 0; i < length; i++) {
      message = message + (char)payload[i];
    }
  
   
    //Serial.println(message);
    //Serial.println("-----------------------");
    if(strcmp(topic,SUB_ALLARME_STATO)==0){
          StaticJsonDocument<200> doc;
         DeserializationError error = deserializeJson(doc, message);
          // Test if parsing succeeds.
          if (error) {
            //Serial.print(F("deserializeJson() failed: "));
            //Serial.println(error.c_str());
            return;
         }

           // Fetch values.
          //
          // Most of the time, you can rely on the implicit casts.
          // In other case, you can do doc["time"].as<long>();
          const char* client_id = doc["client_id"];
          String  data = doc["data"];

        if(data.equals("StatusAlarm.ACTIVE")){
          //Serial.println("ACTIVE");
           statusAlarm =ACTIVE;
           lcd.clear();
           lcd.print("ALLARME ATTIVO");
        }
        else if(data.equals("StatusAlarm.ALARMED")){
          //Serial.println("ALARMED");
              if(statusAlarm==DISABLED)
                return;
              statusAlarm =ALARMED;

              lcd.clear();
              lcd.print("INTRUSIONE");
         }

        else if(data.equals("StatusAlarm.DISABLED")){
          //Serial.println("DISABLED");
              statusAlarm=DISABLED;
              lcd.clear();
              lcd.print("ALLARMED DISATTIVATO");
             
         
        }
    }
  
    else if(strcmp(topic,SUB_ALLARME_KEYNFC_REMOVE)==0)
          return;
        //removeNFCKeyLocal(message);
    else if(strcmp(topic,SUB_ALLARME_CLOCK_SET)==0){
      lcd.setCursor(0,1);
      clockProgramming = message;
    }

    else if(strcmp(topic,SUB_ALLARME_CLOCK_GET)==0){
      lcd.setCursor(0,1);
      char stringachar[6];
      clockProgramming.toCharArray(stringachar,6);
      client.publish("casa/allarme/clock/response",stringachar,2);

    }
    
 }


 void removeNFCKeyLocal(String keynfc){


         int j=1;
        for(int i=1;i<=EEPROM.read(0);i++){

          String EEPROMRead0 = String(EEPROM.read(j));
          String EEPROMRead1=String(EEPROM.read(j+1));
          String EEPROMRead2=String(EEPROM.read(j+2));
          String EEPROMRead3=String(EEPROM.read(j+3));
          String EEPROMRead4=String(EEPROM.read(j+4));



          String EEPROMReadTemp0 = EEPROMRead0 + EEPROMRead1;
          String EEPROMReadTemp1 = EEPROMReadTemp0 + EEPROMRead2;
          String EEPROMReadTemp2 = EEPROMReadTemp1 + EEPROMRead3;
          String EEPROMRead = EEPROMReadTemp2 + EEPROMRead4;

          //Serial.println(keynfc);
          //Serial.println("-");
          //Serial.println(EEPROMRead);

            if(keynfc.equals(EEPROMRead)){
                //Serial.println("CHIAVE SLAVE VALIDA");
                EEPROM.write(j,0);
                EEPROM.write(j+1,0);
                EEPROM.write(j+2,0);
                EEPROM.write(j+3,0);
                EEPROM.write(j+4,0);

                shiftKeysToLeft(j+4, i);
                return;
            }
            j=j+4;
        }
 }

 void shiftKeysToLeft(int j, int i){


    if(i==EEPROM.read(0)){
          EEPROM.write(0,EEPROM.read(0)-1);
          return;
    }

    for(int y=i;y<=EEPROM.read(0);y++){
                EEPROM.write(j-4,EEPROM.read(j+1));
                EEPROM.write(j-3,EEPROM.read(j+2));
                EEPROM.write(j-2,EEPROM.read(j+3));
                EEPROM.write(j-1,EEPROM.read(j+4));
                EEPROM.write(j,EEPROM.read(j+5));

       j=j+4;
          
    }
 }



void blinkLed(){

      if(statusAlarm==ACTIVE)
          digitalWrite(ledWhite,HIGH);
      else if(statusAlarm==ALARMED){
          digitalWrite(ledBlue,HIGH);
          digitalWrite(ledWhite,LOW);
          delay(500);
          digitalWrite(ledWhite,HIGH);
          digitalWrite(ledBlue,LOW);
         
      }

      else{
        
        digitalWrite(ledBlue,LOW);
        digitalWrite(ledWhite,LOW);   
     }
  
}



/* RFID */
void rfidRead() {

  slave = (EEPROM.read(0) == 255) ? 0 : EEPROM.read(0);
  // Rileva un tag...
  if (rfid.isCard()) {
    // Legge il seriale...
    if (rfid.readCardSerial()) {
      sernum0 = rfid.serNum[0];
      sernum1 = rfid.serNum[1];
      sernum2 = rfid.serNum[2];
      sernum3 = rfid.serNum[3];
      sernum4 = rfid.serNum[4];

      //Serial.println("LETTURA SERIAL NFC");
      // Se il seriale letto corrisponde con il seriale Master
      // attiva o disattiva la modalita Memorizzazione chiavi
      // e in più visualizza l'elenco della chiavi salvate...
      if (sernum0 == masnum0 && sernum1 == masnum1 && sernum2 == masnum2 && sernum3 == masnum3 && sernum4 == masnum4) {
        if (cardmas == 0) {
          cardmas = 1;
          lcd.clear();
          lcd.print("BENVENUTO ADMIN");
          lcd.setCursor(0,1);
          lcd.print("KEY TAG LOCALI: ");
          lcd.print(slave);  
        }
        else {
          cardmas = 0;
          lcd.clear();
          lcd.print("EXIT ADMIN");
          delay(1000);
          lcd.setCursor(0,0);
          lcd.print("                     ");
        }
      }//end if


      // Se invece il seriale letto corrisponde con uno dei tre gruppi
      // di chiavi memorizzate allora attiva o disattiva il Led
      else if(slave>0) {
        int j=1;
        for(int i=1;i<=EEPROM.read(0);i++){
            if(sernum0==EEPROM.read(j) && sernum1==EEPROM.read(j+1) && sernum2==EEPROM.read(j+2) && sernum3==EEPROM.read(j+3) && sernum4==EEPROM.read(j+4)){
                lcd.clear();
                lcd.print("CHIAVE VALIDA");
                switchStatusAlarm();
            }

            j=j+4;
        }
      }

      // Se il seriale letto è diverso dal master e non è presente in memoria,
      // e se è attiva la modalita Memorizzazione chiavi, salva il seriale in memoria come slave
      else if (cardmas == 1 ){
        int nextSlavePosition = (EEPROM.read(0)*5) +1;
        //Serial.println(nextSlavePosition +1);
        EEPROM.write(0, EEPROM.read(0) + 1);
        EEPROM.write(nextSlavePosition, sernum0);
        EEPROM.write(nextSlavePosition + 1, sernum1);
        EEPROM.write(nextSlavePosition + 2, sernum2);
        EEPROM.write(nextSlavePosition + 3, sernum3);
        EEPROM.write(nextSlavePosition + 4, sernum4);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("REGISTRATO CON SUCCESSO");
        delay(2000);
        //Serial.println(nextSlavePosition);
        //storeNfcKey(String(sernum0,DEC) + String(sernum1,DEC) + String(sernum2,DEC) + String(sernum3,DEC) + String(sernum4,DEC));
        lcd.setCursor(0,0);
        lcd.print("                                ");
       
      }
    }

  }

  rfid.halt();
}


void switchStatusAlarm() {

  if (statusAlarm == DISABLED) {
    statusAlarm = ACTIVE;
    lcd.clear();
    lcd.print("ALLARME ATTIVO");
    client.publish("casa/allarme/stato","{\"client_id\":\"" ARDUINO_CLIENT_ID  "\",\"data\":\"ACTIVE\"}",2);

  }

  else if (statusAlarm == ALARMED || statusAlarm == ACTIVE) {
    statusAlarm = DISABLED;
    lcd.clear();
    lcd.print("ALLARME DISATTIVATO");
    client.publish("casa/allarme/stato","{\"client_id\":\"" ARDUINO_CLIENT_ID  "\",\"data\":\"DISABLED\"}",2);
  }

}





void initWifi() {

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    lcd.clear();
    lcd.print("Comunicazione wifi fallita");
    //Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    //Serial.println("Please upgrade the firmware");
  }

  checkConnectWifi();

  // you're connected now, so print out the data:
  //Serial.println("You're connected to the network");
  //printCurrentNet();
  //printWifiData();


}

void checkConnectMqtt(){
  

     while (!client.connected()) {
          //Serial.println("Connecting to MQTT...");
       
          if (client.connect(ARDUINO_CLIENT_ID,"paolo", "paoletto")) {
       
            //Serial.println("connected");  
           // Subscribe
           client.subscribe(SUB_ALLARME_STATO);
           client.subscribe(SUB_ALLARME_KEYNFC_REMOVE);
           client.subscribe(SUB_ALLARME_CLOCK_GET);
           client.subscribe(SUB_ALLARME_CLOCK_SET);

          } else {
       
            //Serial.print("failed with state ");
            //Serial.print(client.state());
            delay(500);
       
          }
      }  
  
}



void checkConnectWifi(){

    // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.println("TENTATIVO DI CONNESSIONE ALLA RETE WIFI");
    // Connect to WPA/WPA2 network:
    WiFi.begin(ssid, pass);
    delay(1000);
  }

}


void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  //Serial.println("IP Address: ");
  //Serial.println(ip);
  //Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  //Serial.println("MAC address: ");
  //printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  //Serial.println("SSID: ");
  //Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  //Serial.println("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  //Serial.println("signal strength (RSSI):");
  //Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  //Serial.println("Encryption Type:");
  //Serial.println(encryption, HEX);
  //Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      //Serial.println("0");
    }
    //Serial.println(mac[i], HEX);
    if (i > 0) {
      //Serial.println(":");
    }
  }
  //Serial.println();
}

void storeNfcKey(String nfcKey) {
  WiFiClient client;
  if (client.connect(server, 80)) {
    //Serial.println("connected");
    // Make a HTTP request:
    client.print(String("GET /AlarmIoT_WebServer/public/api/user/registration?nfc_key=" + nfcKey) + " HTTP/1.1\r\n" +
                 "Host: " + "192.168.43.175" + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n" +
                 "Accept: application/json"
                );
    client.println();
    //Serial.println("[Response:]");
    String line;
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        line = client.readStringUntil('\r');
        //Serial.println(line);
      }
    }

    client.stop();
    //Serial.println("\n[Disconnected]");
  }
}



