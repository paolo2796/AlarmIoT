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

void setup() {
 
pinMode(triggerPort, OUTPUT);
pinMode(echoPort, INPUT);
pinMode(ledWhite, OUTPUT);
pinMode(ledBlue, OUTPUT);
Serial.begin(9600);
//init temperature's sensor
 dht.begin();
}
 
void loop() {

  //read sensor's temperature
  float temperature = dht.readTemperature();
  checkDistanceProximity(temperature);
  
  //Aspetta 1 secondo
  delay(1000);
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
 
  if(distance < 10){
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
      float cmmc= (float) v/10000;
      long distance = cmmc * durata/2;
      return distance;

  }
