#include <ArduinoJson.h>


#include <WiFiNINA.h>
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "SuperFibra2 2.4GHz";        // your network SSID (name)
char pass[] = "vigorhome";    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


const int triggerPort = 2;
const int echoPort = 3;

//Variables
int chk;
float hum;  //Stores humidity value
float temp = 25; //Stores temperature value

/* HTTP CLIENT */
#include <Bridge.h>
#include <HttpClient.h>


//library dht22
#include <DHT.h>
//Constants
#define DHTPIN 13     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

/* MQTT */

#include <PubSubClient.h>

#define SENSOR1_ID "sensor1"
#define CONNECTOR "mqtt"
IPAddress server(192,168,0,103);// MTTQ server IP address

WiFiClient wifiClient;
PubSubClient client(wifiClient);


enum StatusSensor {
  ENABLED,
  DISABLED,
};

StatusSensor statusSensor;

void setup() {

  statusSensor = ENABLED;
  // put your setup code here, to run once:

    //Initialize serial and wait for port to open:
  Serial.begin(9600);
  initSensorProximity();
  //init temperature's sensor
  dht.begin();
  initWifi();
  initClientMQTT();
  


}

void loop() {
  // put your main code here, to run repeatedly:


  client.loop();

  
  if(statusSensor == DISABLED)
    return;

    //read sensor's temperature
  float temperature = dht.readTemperature();
  checkDistanceProximity(temperature);

  delay(1000);
}



void initSensorProximity() {

  pinMode(triggerPort, OUTPUT);
  pinMode(echoPort, INPUT);

}

/* Event's MQTT client */

void initClientMQTT(){
  
  
    client.setServer(server, 1883);
    client.setCallback(callback);


   while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(SENSOR1_ID,"paolo", "paoletto")) {
 
      Serial.println("connected");  
      client.subscribe("casa/sensori/request");
      client.subscribe("casa/sensori/" SENSOR1_ID);


 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(500);
 
    }
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

  //dopo 38ms è fuori dalla portata del sensore
  if ( durata > 38000 ) {
    //Serial.println("Fuori portata   ");
  }


  if (distance < 10) {
    Serial.println("INTRUSIONE");

    //l'allarme ha rilevato l'intrusione. Imposto lo stato corrente
     client.publish("casa/allarme/stato","{\"client_id\":\"sensor1\",\"data\":\"StatusAlarm.ALARMED\"}",2);
  }

}


long calcDistance(float temperature, int durata) {
  float v = 331.4 + (0.62 * temperature);
  float cmmc = (float) v / 10000;
  long distance = cmmc * durata / 2;
  return distance;

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
    if(strcmp(topic,"casa/sensori/request")==0){
      if(statusSensor==ENABLED)
        client.publish("casa/sensori/response","{\"client_id\":\"sensor1\",\"enabled\":true}");
      else
        client.publish("casa/sensori/response","{\"client_id\":\"sensor1\",\"disabled\":true}");
    }

    else if(strcmp(topic,"casa/sensori/" SENSOR1_ID)==0){

      Serial.println("AFAMMOK");

          if(message.equals("disabled"))
              statusSensor = DISABLED;
          else if(message.equals("enabled")){
              statusSensor = ENABLED;
          }
    }
  
    
 }




void initWifi() {

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    //Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    //Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
        // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }

  // you're connected now, so print out the data:
  //Serial.println("You're connected to the network");
  //printCurrentNet();
  //printWifiData();


}




