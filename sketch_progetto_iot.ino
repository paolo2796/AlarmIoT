const int triggerPort = 2;
const int echoPort = 3;
const int ledBlue = 6;
const int ledWhite = 4;


//library dht22
#include <DHT.h>
//Constants
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

//Variables
int chk;
float hum;  //Stores humidity value
float temp = 25; //Stores temperature value


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
#define PUB_PROXIMITY "casa/database/registration_nfc"   // MTTQ topic for proximity [CM]
#define CONNECTOR "mqtt"
IPAddress server(192, 168, 0, 104);// MTTQ server IP address

WiFiClient wifiClient;
PubSubClient client(wifiClient);


/* HTTP CLIENT */
#include <Bridge.h>
#include <HttpClient.h>



void initSensorProximity() {

  pinMode(triggerPort, OUTPUT);
  pinMode(echoPort, INPUT);

}

void initPins() {

  pinMode(ledWhite, OUTPUT);
  digitalWrite(ledWhite, HIGH);
  pinMode(ledBlue, OUTPUT);

}




/* END RFID  PROPERTY */

void setup() {


  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  initSensorProximity();
  initPins();
  initWifi();
  //init temperature's sensor
  dht.begin();
  //rfid init
  SPI.begin();
  rfid.init();

  /*
    Serial.println("MESSAGGIO DA INVIARE");
    // MTTQ parameters
    client.setServer(server, 1883);
    client.connect(ARDUINO_CLIENT_ID, "paolo", "paoletto");
    client.setCallback(callback);

    client.publish(PUB_PROXIMITY, "A");

    Serial.println("MESSAGGIO PUBBLICATO"); */


}


void loop() {

  //printCurrentNet();

  //verifico lettura rfid
  rfidRead();

  if (statusAlarm == DISABLED)
    return;

  //read sensor's temperature
  float temperature = dht.readTemperature();
  checkDistanceProximity(temperature);





  //Aspetta 1 secondo
  delay(1000);
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
        int j=1;
        for(int i=1;i<=EEPROM.read(0);i++){
            if(sernum0==EEPROM.read(j) && sernum1==EEPROM.read(j++) && sernum2==EEPROM.read(j++) && sernum3==EEPROM.read(j++) && sernum4==EEPROM.read(j++)){
                Serial.println("CHIAVE SLAVE VALIDA, NUMERO :" + i);
                checkStatusAlarm();
            }
        }
        delay(3000);
      }

      // Se il seriale letto è diverso dal master e non è presente in memoria,
      // e se è attiva la modalita Memorizzazione chiavi, salva il seriale in memoria come slave
      else if (cardmas == 1 ){
        int nextSlavePosition = (EEPROM.read(0)*5) +1;
        Serial.println("Chiave rilevata!");
        EEPROM.write(0, EEPROM.read(0) + 1);
        EEPROM.write(nextSlavePosition, sernum0);
        EEPROM.write(nextSlavePosition++, sernum1);
        EEPROM.write(nextSlavePosition++, sernum2);
        EEPROM.write(nextSlavePosition++, sernum3);
        EEPROM.write(nextSlavePosition++, sernum4);
        Serial.println("Chiave salvata, NUMERO : " + EEPROM.read(0));
        storeNfcKey(String(sernum0,DEC) + String(sernum1,DEC) + String(sernum2,DEC) + String(sernum3,DEC) + String(sernum4,DEC));
        delay(3000);        
      }
    }

    delay(1000);
  }

  rfid.halt();
}


void checkStatusAlarm() {

  if (statusAlarm == DISABLED) {
    statusAlarm = ACTIVE;
    Serial.println("ALLARME ATTIVO");
  }

  else if (statusAlarm == ALARMED || statusAlarm == ACTIVE) {
    statusAlarm = DISABLED;
    Serial.println("ALLARME DISATTIVATO");
  }

}



/* SENSOR PROXIMITY */
void checkDistanceProximity(float temperature) {

  //porta bassa l'uscita del trigger
  digitalWrite( triggerPort, LOW );
  //invia un impulso di 10microsec su trigger
  digitalWrite( triggerPort, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( triggerPort, LOW );

  long durata = pulseIn( echoPort, HIGH );
  long distance = calcDistance(temperature, durata);
  Serial.print("DISTANZA ");
  Serial.println(distance);
  //Serial.println(distance);

  //dopo 38ms è fuori dalla portata del sensore
  if ( durata > 38000 ) {
    Serial.println("Fuori portata   ");
  }

  if (distance < 10 || statusAlarm == ALARMED) {

    //l'allarme ha rilevato l'intrusione. Imposto lo stato corrente
    statusAlarm = ALARMED;
    digitalWrite(ledWhite, LOW);
    digitalWrite(ledBlue, HIGH);
    delay(300);
    digitalWrite(ledWhite, HIGH);
    digitalWrite(ledBlue, LOW);
  }
  else {
    digitalWrite(ledWhite, HIGH);
    digitalWrite(ledBlue, LOW);
  }
}


long calcDistance(float temperature, int durata) {
  float v = 331.4 + (0.62 * temperature);
  Serial.println("TEMPERATURA");
  Serial.println(temperature);
  float cmmc = (float) v / 10000;
  long distance = cmmc * durata / 2;
  return distance;

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
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();


}



/* WiFi */
void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
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
