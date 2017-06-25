#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);
IPAddress server(200, 5, 235, 52);

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

//relay luz porton (exterior)
int in1 = 7;

void callback(char* topic, byte* payload, unsigned int length) {
  String strTopic(topic);
  
  //payload = 0 status
  //payload = 1 change

  if((char)payload[0] == '0'){
    if (strTopic == "casa/luz/porton"){       
      if (digitalRead(in1)){
          mqttClient.publish("casa/luz/porton", "encendido");
          } else {
          mqttClient.publish("casa/luz/porton", "apagado");
          }
      }
    if (strTopic == "casa/temperatura/living"){
        sensors.requestTemperatures(); 
        char temp[10];
        dtostrf(sensors.getTempCByIndex(0), 5, 1, temp);
        delay(1);
        mqttClient.publish("casa/temperatura/living", temp);          
      }
    } else if((char)payload[0] == '1'){
      if (strTopic == "casa/luz/porton"){   
        digitalWrite(in1, !digitalRead(in1));
        if (digitalRead(in1)){
          mqttClient.publish("casa/luz/porton", "encendido");
          } else {
          mqttClient.publish("casa/luz/porton", "apagado");
            }
      }
    }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient", "mi_usuario", "mi_clave")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("casa","iniciado");
      // ... and resubscribe
      //mqttClient.subscribe("casa/temperatura/living");
      mqttClient.subscribe("casa/#");
    } else {
      Serial.print("failed, rc=");
      //Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  sensors.begin();
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);
  //Controlador relay luz porton
  pinMode(in1, OUTPUT);
  digitalWrite(in1, LOW);
   
  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);  
}
 
void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  //Serial.print(" Requesting temperatures...");
  ////sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.println("DONE");

  //Serial.print("Temperature for Device 1 is: ");
  //Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire

  ////char buffer[10];
  ////dtostrf(sensors.getTempCByIndex(0), 5, 1, buffer);
  ////Serial.println(buffer);
  //mqttClient.publish("casa/temperatura/living", buffer);
  
  mqttClient.loop();
  ////delay(5000);
}

