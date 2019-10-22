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
float temp=25; //Stores temperature value



/* START RFID PROPERTY */


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
int slave=0; // Contatore delle chiavi salvate
    
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


void initSensorProximity(){
  
  pinMode(triggerPort, OUTPUT);
  pinMode(echoPort, INPUT);
  
}

void initPins(){
  
  pinMode(ledWhite, OUTPUT);
  digitalWrite(ledWhite,HIGH);
  pinMode(ledBlue, OUTPUT);
  
}


/* END RFID  PROPERTY */

void setup() {


initSensorProximity();
initPins();
 

Serial.begin(9600);
//init temperature's sensor
 dht.begin();


  //rfid init
 SPI.begin(); 
  rfid.init();
}
 
void loop() {

    //verifico lettura rfid
  rfidRead();

  if(statusAlarm == DISABLED)
    return;

  //read sensor's temperature
  float temperature = dht.readTemperature();
  checkDistanceProximity(temperature);


  //Aspetta 1 secondo
  delay(1000);
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

                   checkStatusAlarm();
                   delay(3000);

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
                    Serial.println("Slave 1 salvata!");
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


void checkStatusAlarm(){
      
   if(statusAlarm==DISABLED){
          statusAlarm = ACTIVE;
          Serial.println("ALLARME ATTIVO");
    }

     else if(statusAlarm == ALARMED || statusAlarm == ACTIVE){
          statusAlarm = DISABLED;
          Serial.println("ALLARME DISATTIVATO"); 
     }
  
}


void checkDistanceProximity(float temperature){
     
  //porta bassa l'uscita del trigger
  digitalWrite( triggerPort, LOW );
  //invia un impulso di 10microsec su trigger
  digitalWrite( triggerPort, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( triggerPort, LOW );
   
  long durata = pulseIn( echoPort, HIGH );
  long distance = calcDistance(temperature,durata);
  Serial.print("DISTANZA ");
  Serial.println(distance);
  //Serial.println(distance);
   
  //dopo 38ms è fuori dalla portata del sensore
  if( durata > 38000 ){
    Serial.println("Fuori portata   ");
  }
 
  if(distance < 10 || statusAlarm == ALARMED){
    
   //l'allarme ha rilevato l'intrusione. Imposto lo stato corrente
   statusAlarm = ALARMED;
   digitalWrite(ledWhite,LOW);
   digitalWrite(ledBlue,HIGH);
   delay(300);
   digitalWrite(ledWhite,HIGH);
   digitalWrite(ledBlue,LOW);
  }
  else{
   digitalWrite(ledWhite,HIGH);
   digitalWrite(ledBlue,LOW);
 }
}


 long calcDistance(float temperature, int durata){
      float v = 331.4 + (0.62 * temperature);
      Serial.println("TEMPERATURA");
      Serial.println(temperature);
      float cmmc= (float) v/10000;
      long distance = cmmc * durata/2;
      return distance;

  }
