/*
   Ejemplo base publicación/suscripción MQTT con Arduino MKR NB 1500
   Librería MQTT: https://github.com/knolleary/pubsubclient
   Librería para MKR NB 1500: https://www.arduino.cc/reference/en/libraries/mkrnb/
*/

#include <MKRNB.h>  // libreria GSM para GSM Shield
#include <PubSubClient.h>
#include "secrets.h"
#include "DHTStable.h"

/*
secrets.h define los siguimetes secrets
#define ID_DISPOSITIVO "xxxx"   // id dispositivo
#define BROKER_USER "xxxx"      // usuario broker
#define BROKER_PASSWORD "xxxx"  // password broker
*/

//Pines
#define LED_ROJO 7
#define LED_AMARILLO 8
#define PULSADOR 5
#define LDR A1
#define DHT22 9

DHTStable DHT;

#define SECRET_PINNUMBER ""
const char PINNUMBER[] = SECRET_PINNUMBER;

// initialize the library instance
NBClient gprsclient;  // Client service for TCP connection
GPRS gprsAccess;      // GPRS access
NB nbAccess;          // NB access: include a 'true' parameter for debug enabled

IPAddress server(217, 160, 207, 137);  //Dirección del broker MQTT aprendiendonodered.com

String ledRojoTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/ledrojo";
String ledAmarilloTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/ledamarillo";
String ledBuiltinTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/ledbuiltin";
String pulsadorTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/pulsador";
String initTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/inicio";
String temperaturaTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/temperatura";
String humedadTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/humedad";
String iluminacionTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/iluminacion";
String lwtTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/status";

// connection state
boolean notConnected = true;

//MQTT client
PubSubClient client(gprsclient);

// variables para publicar periodicamente
long lastMsg = 0;

// variable pulsador
volatile byte state_pulsador = LOW;

//Funcion Callback para topics suscritos
void callback(char* topic, byte* payload, unsigned int length) {
  String payload_s = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payload_s += (char)payload[i];
  }
  Serial.println();
  if (topic == ledBuiltinTopic) {
    if (payload_s == "ON") {
      Serial.println("Enciendo led integrado");
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.println("Apago led integrado");
      digitalWrite(LED_BUILTIN, LOW);
    }
  } else if (topic == ledRojoTopic) {
    if (payload_s == "ON") {
      Serial.println("Enciendo led rojo");
      digitalWrite(LED_ROJO, HIGH);
    } else {
      Serial.println("Apago led rojo");
      digitalWrite(LED_ROJO, LOW);
    }
  } else if (topic == ledAmarilloTopic) {
    if (payload_s == "ON") {
      Serial.println("Enciendo led amarillo");
      digitalWrite(LED_AMARILLO, HIGH);
    } else {
      Serial.println("Apago led amarillo");
      digitalWrite(LED_AMARILLO, LOW);
    }
  }
}

void reconnect(String texto) {
  //Conexión LTE-M o NB-IoT
  while (notConnected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) && (gprsAccess.attachGPRS() == GPRS_READY)) {
      notConnected = false;
      Serial.println("Connected!!");
    } else {
      Serial.println("Not connected!!");
      delay(5000);
    }
  }

  //Get IP.
  IPAddress LocalIP = gprsAccess.getIPAddress();
  Serial.print("Server IP address= ");
  Serial.println(LocalIP);

  Serial.println("Cellular Connection Established...");
  Serial.println("Device id: " + String(ID_DISPOSITIVO));

  // Iniciar Conexión MQTT
  Serial.println("Starting MQTT client");
  client.setServer(server, 1883);
  client.setCallback(callback);

  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect(ID_DISPOSITIVO, BROKER_USER, BROKER_PASSWORD, lwtTopic.c_str(), 2, true, "KO")) {
    Serial.println("connected to MQTT broker");
    // ... and resubscribe
    client.subscribe(ledBuiltinTopic.c_str());
    client.subscribe(ledRojoTopic.c_str());
    client.subscribe(ledAmarilloTopic.c_str());
    client.publish(initTopic.c_str(), texto.c_str());
    client.publish(lwtTopic.c_str(), "OK", true);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    notConnected = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  delay(2000);

  Serial.println("Starting Arduino MKR1500...");
  Serial.println("Device id: " + String(ID_DISPOSITIVO));
  Serial.println("Initializing ports...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);        //inicializamos salida para led
  pinMode(LED_AMARILLO, OUTPUT);    //inicializamos salida para led
  pinMode(PULSADOR, INPUT_PULLUP);  // inicializamos entrada para pulsador
  attachInterrupt(digitalPinToInterrupt(PULSADOR), pulsado, FALLING);

  Serial.print("Connecting NB IoT / LTE Cat M1 network...");
  reconnect("Inicio");

  delay(1500);
}

void loop() {
  if (!client.connected()) {
    notConnected = true;
    Serial.println("Error Conexión. Intento reconexión");
    reconnect("error_conexion");
  }
  client.loop();

  //publicar cada 5 segundos
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    int chk = DHT.read22(DHT22);
    if (chk == DHTLIB_OK) {
      //Serial.print("Temperatura: ");
      //Serial.println(DHT.getTemperature(), 1);
      client.publish(temperaturaTopic.c_str(), String(DHT.getTemperature()).c_str());
      //Serial.print("Humedad: ");
      //Serial.println(DHT.getHumidity(), 1);
      client.publish(humedadTopic.c_str(), String(DHT.getHumidity()).c_str());
    } else {
      Serial.println("Error sonda");
    }
    //Serial.print("LDR: ");
    //Serial.println(analogRead(LDR));
    client.publish(iluminacionTopic.c_str(), String(analogRead(LDR)).c_str());
  }

  if (state_pulsador == HIGH) {
    state_pulsador = LOW;
    Serial.println("Pulsador Presionado");
    client.publish(pulsadorTopic.c_str(), "Pulsador Presionado");
  }
}

void pulsado() {
  state_pulsador = HIGH;
}