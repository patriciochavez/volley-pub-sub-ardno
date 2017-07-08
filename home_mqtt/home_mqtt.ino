#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire1(2);
OneWire oneWire2(4);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);
IPAddress server(200, 5, 235, 52);

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

//relay luz porton (exterior)
int in1 = 7;
int sonido = 0;

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
    if (strTopic == "casa/estado/temperatura"){
        int outOfRangeE = 0;
        while (outOfRangeE < 5){
        sensors1.requestTemperatures();        
        delay(1);
        float tempE = sensors1.getTempCByIndex(0);
        if (tempE > -127.0){
        char temp1[10];
        dtostrf(tempE, 5, 1, temp1);
        mqttClient.publish("casa/temperatura/exterior", temp1);
        outOfRangeE = 10;
        } else {
          outOfRangeE += 1;
          }         
        }
        int outOfRangeL = 0;
        float tempL = sensors2.getTempCByIndex(0);
        while (outOfRangeL < 5){        
        sensors2.requestTemperatures(); 
        delay(1);
        if ( tempL > -127.0){     
          char temp2[10];
          dtostrf(tempL, 5, 1, temp2);   
          mqttClient.publish("casa/temperatura/living", temp2);     
          outOfRangeL = 10;
        } else {
          outOfRangeL += 1;
          }         
        }
      }    
    } else if((char)payload[0] == '1'){
      if (strTopic == "casa/luz/porton"){   
        digitalWrite(in1, !digitalRead(in1));
        if (digitalRead(in1)){
          beep(100);
          beep(100);
          beep(100);
          mqttClient.publish("casa/luz/porton", "encendido");
          } else {
          mqttClient.publish("casa/luz/porton", "apagado");
            }
      }
    }

    if (strTopic == "casa/buzzer/distancia" && sonido == 1){
      if((char)payload[0] == '1'){
        //sonido 100mtrs
        beep(50);
        beep(50);
        beep(50);          
        } else if((char)payload[0] == '2'){
        //sonido 200mtrs
        beep(50);
        beep(50);          
        } else if((char)payload[0] == '3'){
        //sonido 300mtrs
        beep(50);          
        }
    }

    if (strTopic == "casa/buzzer/sonido"){
         if((char)payload[0] == '1'){
          sonido = !sonido;
          mqttClient.publish("casa/estado/buzzer", "0");
         }
    }

  if (strTopic == "casa/estado/buzzer"){
         if(sonido == 0){ //sin sonido
            mqttClient.publish("casa/buzzer/sonido", "apagado");
         } else if(sonido == 1){ //con sonido
            mqttClient.publish("casa/buzzer/sonido", "encendido");
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

void beep(unsigned char pausa){
            analogWrite(9, 20);
            delay(pausa);                 // Espera
            analogWrite(9, 0);            // Apaga
            delay(pausa);                 // Espera
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
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  digitalWrite(2, LOW); //Disable internal pull-up.
  digitalWrite(4, LOW);
  sensors1.begin();
  sensors2.begin();
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);
  //Controlador relay luz porton
  pinMode(in1, OUTPUT);
  digitalWrite(in1, LOW);
  pinMode(9, OUTPUT);
             
  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);  
}
 
void loop()
{ 
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}
