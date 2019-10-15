/**
* ACCENSIONE LED CON RFID 
* Per maggiori info: www.progettiarduino.com
* Importante, collegare il lettore RFID ai pin di Arduino come segue:
* MOSI: Pin 11 / ICSP-4
* MISO: Pin 12 / ICSP-1
* SCK: Pin 13 / ISCP-3
* SDA: Pin 10
* RST: Pin 9
*/
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

void setup()
{ 
  Serial.begin(9600);   //Apriamo la comunicazione con il monitor seriale
  SPI.begin(); 
  rfid.init();

}

void loop() {
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
                        delay(1500);
                        Serial.println();
                        Serial.println("GESTIONE MEMORIZZAZIONE CHIAVI");
                        cardmas = 1;
                        Serial.println("Chiavi slave: ");
                        Serial.println(slave);
                        delay(2000);
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
                 cardmas = 0;
                 Serial.println("Slave 3 salvata!");
                 delay(3000);
           }

           else if(cardmas == 1){
              Serial.println("Uscita modalità admin"); 
           }
         }
       }
               
         
  

        /*//Se il led è ON cambia lo stato   
          if ( ledOn == 1 && stato == 0){
                  delay(pausa);
                  stato = 1;
             } 
 
           
       // Se è stata attivata con la card Master la modalità "Memorizzazione chiavi",
       // premendo il pulsante di reset si azzera il database 
       // delle chiavi salvate fino a quel momento. 
       if (digitalRead(resetkey) == HIGH && cardmas == 1) {
               cardmas = 0;
               for (int i=0; i<16; i++){
                 EEPROM.write(i, 0);
                   } 
            
               Serial.println("Reset chiavi...");
               delay(3000);
               standby();
           }    */    
                      
       rfid.halt();
}


