const int ledBlue = 6;
const int ledWhite = 4;




/* START RFID PROPERTY */


/**
  ACCENSIONE LED CON RFID
  Per maggiori info: www.progettiarduino.com
  Importante, collegare il lettore RFID ai pin di Arduino uno come segue:
  MOSI: Pin 11 / ICSP-4
  MISO: Pin 12 / ICSP-1
  SCK: Pin 13 / ISCP-3
  SDA: Pin 10
  RST: Pin 9
*/

// FOR ARDUINO MEGA MOSI  MISO  SCK SS (slave)  SS (master) Level

//Mega1280 or Mega2560  51     50   52  53  - 5V
#include <SPI.h>
#include <RFID.h>
#include <EEPROM.h>


#define SS_PIN 10
#define RST_PIN 9

// Codice della chiave master.
#define masnum0 199
#define masnum1 3
#define masnum2 151
#define masnum3 27
#define masnum4 72



#define resetkey 6

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


#include <WiFiNINA.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "SuperFibra2 2.4GHz";        // your network SSID (name)
char pass[] = "vigorhome";    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


/* MQTT */

#include <PubSubClient.h>

#define ARDUINO_CLIENT_ID "arduino_uno_rev2"
#define CONNECTOR "mqtt"

#define  SUB_ALLARME_KEYNFC_REMOVE "casa/allarme/keynfc/remove"
#define  SUB_ALLARME_STATO "casa/allarme/stato"
IPAddress server(192, 168, 0, 104);// MTTQ server IP address

WiFiClient wifiClient;
PubSubClient client(wifiClient);


/* HTTP CLIENT */
#include <Bridge.h>
#include <HttpClient.h>


void initPins() {

  pinMode(ledWhite, OUTPUT);
  digitalWrite(ledWhite, HIGH);
  pinMode(ledBlue, OUTPUT);

}


/* END RFID  PROPERTY */





void setup() {


  //Initialize serial and wait for port to open:
  Serial.begin(9600);



  initPins();
  initWifi();

  //rfid init
  SPI.begin();
  rfid.init();

    initClientMQTT();


/*

 EEPROM.write(0,0);
 EEPROM.write(1,0);
 EEPROM.write(2,0);
 EEPROM.write(3,0);
 EEPROM.write(4,0);
 EEPROM.write(5,0); 
*/


}


void loop() {

  blinkLed();
  
  //verifico lettura rfid
  rfidRead();

    //maintain connection mqtt
    client.loop();

  //printCurrentNet();


  delay(1000);

}



/* Event's MQTT client */

void initClientMQTT(){
  
  
    client.setServer(server, 1883);
    client.setCallback(callback);


     while (!client.connected()) {
          Serial.println("Connecting to MQTT...");
       
          if (client.connect(ARDUINO_CLIENT_ID,"paolo", "paoletto")) {
       
            Serial.println("connected");  
           // Subscribe
           client.subscribe(SUB_ALLARME_STATO);
           client.subscribe(SUB_ALLARME_KEYNFC_REMOVE);
       
          } else {
       
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(500);
       
          }
      }

}


 void callback(char* topic, byte* payload, unsigned int length) {
 
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
   
    Serial.println("Message:");
    String message = "";
    for (int i = 0; i < length; i++) {
      message = message + (char)payload[i];
    }
  
   
    Serial.println(message);
    Serial.println("-----------------------");
    if(strcmp(topic,SUB_ALLARME_STATO)==0){
        if(message.equals("ACTIVE"))
          statusAlarm =ACTIVE;
        else if(message.equals("ALARMED"))
          statusAlarm =ALARMED;
        else if(message.equals("DISABLED"))
          statusAlarm=DISABLED;
    }
  
    else if(strcmp(topic,SUB_ALLARME_KEYNFC_REMOVE)==0)
        removeNFCKeyLocal(message);
    
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

          Serial.println(keynfc);
          Serial.println("-");
          Serial.println(EEPROMRead);

            if(keynfc.equals(EEPROMRead)){
                Serial.println("CHIAVE SLAVE VALIDA");
                EEPROM.write(j,0);
                EEPROM.write(j+1,0);
                EEPROM.write(j+2,0);
                EEPROM.write(j+3,0);
                EEPROM.write(j+4,0);
            }
            j=j+4;
        }
        delay(1000); 
 }


void blinkLed(){
  
      if(statusAlarm==ACTIVE)
          digitalWrite(ledWhite,HIGH);
      else if(statusAlarm==ALARMED){
          digitalWrite(ledBlue,HIGH);
          digitalWrite(ledWhite,LOW);
          delay(1000);
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
      Serial.println(sernum0);
      Serial.println(sernum1);
      Serial.println(sernum2);
      Serial.println(sernum3);
      Serial.println(sernum4);
      // Se il seriale letto corrisponde con il seriale Master
      // attiva o disattiva la modalita Memorizzazione chiavi
      // e in più visualizza l'elenco della chiavi salvate...
      if (sernum0 == masnum0 && sernum1 == masnum1 && sernum2 == masnum2 && sernum3 == masnum3 && sernum4 == masnum4) {
        if (cardmas == 0) {
          Serial.println("MODALITA' ADMIN");
          delay(1000);
          cardmas = 1;
          Serial.println("Chiavi slave: ");
          Serial.println(slave);
        }
        else {
          cardmas = 0;
          Serial.println("USCITA MODALITA ADMIN");
          delay(1000);
        }
      }//end if


      // Se invece il seriale letto corrisponde con uno dei tre gruppi
      // di chiavi memorizzate allora attiva o disattiva il Led
      else if(slave>0) {
        Serial.println("SONO QUI");
        int j=1;
        for(int i=1;i<=EEPROM.read(0);i++){
            if(sernum0==EEPROM.read(j) && sernum1==EEPROM.read(j+1) && sernum2==EEPROM.read(j+2) && sernum3==EEPROM.read(j+3) && sernum4==EEPROM.read(j+4)){
                Serial.println("CHIAVE SLAVE VALIDA");
                switchStatusAlarm();
            }

            j=j+4;
        }
        delay(1000);
      }

      // Se il seriale letto è diverso dal master e non è presente in memoria,
      // e se è attiva la modalita Memorizzazione chiavi, salva il seriale in memoria come slave
      else if (cardmas == 1 ){
        Serial.println("SONO QUI 2");
        int nextSlavePosition = (EEPROM.read(0)*5) +1;
        Serial.println("Chiave rilevata!");
        Serial.println(nextSlavePosition +1);
        EEPROM.write(0, EEPROM.read(0) + 1);
        EEPROM.write(nextSlavePosition, sernum0);
        EEPROM.write(nextSlavePosition + 1, sernum1);
        EEPROM.write(nextSlavePosition + 2, sernum2);
        EEPROM.write(nextSlavePosition + 3, sernum3);
        EEPROM.write(nextSlavePosition + 4, sernum4);
        Serial.println("Chiave salvata");
        Serial.println(nextSlavePosition);
        storeNfcKey(String(sernum0,DEC) + String(sernum1,DEC) + String(sernum2,DEC) + String(sernum3,DEC) + String(sernum4,DEC));
        delay(1000);        
      }
    }

    delay(1000);
  }

  rfid.halt();
}


void switchStatusAlarm() {

  if (statusAlarm == DISABLED) {
    statusAlarm = ACTIVE;
    Serial.println("ALLARME ATTIVO");
  }

  else if (statusAlarm == ALARMED || statusAlarm == ACTIVE) {
    statusAlarm = DISABLED;
    Serial.println("ALLARME DISATTIVATO");
  }

}





void initWifi() {

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
        // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  //printCurrentNet();
  //printWifiData();


}



/* WiFi */
void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.println("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.println("MAC address: ");
  //printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.println("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.println("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.println("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.println("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.println("0");
    }
    Serial.println(mac[i], HEX);
    if (i > 0) {
      Serial.println(":");
    }
  }
  Serial.println();
}

void storeNfcKey(String nfcKey) {
  // Initialize the client library
  WiFiClient client;

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.print(String("GET /AlarmIoT_WebServer/public/api/user/registration?nfc_key=" + nfcKey) + " HTTP/1.1\r\n" +
                 "Host: " + "192.168.0.104" + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n" +
                 "Accept: application/json"
                );
    client.println();
    Serial.println("[Response:]");
    String line;
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        line = client.readStringUntil('\r');
        Serial.println(line);
      }
    }


    client.stop();
    Serial.println("\n[Disconnected]");
  }
}



