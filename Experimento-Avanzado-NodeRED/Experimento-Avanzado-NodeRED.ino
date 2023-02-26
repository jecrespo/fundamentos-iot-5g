/*
   Ejemplo base publicación/suscripción MQTT con Arduino MKR NB 1500
   Librería MQTT: https://github.com/knolleary/pubsubclient
   Librería para MKR NB 1500: https://www.arduino.cc/reference/en/libraries/mkrnb/
*/

#include <MKRNB.h>  // libreria GSM para GSM Shield
#include <PubSubClient.h>
#include "secrets.h"

/*
secrets.h define los siguimetes secrets
#define ID_DISPOSITIVO "xxxx"   // id dispositivo
#define BROKER_USER "xxxx"      // usuario broker
#define BROKER_PASSWORD "xxxx"  // password broker
*/

//Pines
#define LED 3                // Se utiliza para el LED conectado el pin 1 del arduino
#define PULSADOR 4           // Se utiliza para el pulsador conectado al pin 2
#define SECRET_PINNUMBER ""  //Usar en caso de tener PIN la tarjeta SIM

const char PINNUMBER[] = SECRET_PINNUMBER;

// initialize the library instance
NBClient gprsclient;  // Client service for TCP connection
GPRS gprsAccess;      // GPRS access
NB nbAccess;          // NB access: include a 'true' parameter for debug enabled

IPAddress server(217, 160, 207, 137);  //Dirección del broker MQTT aprendiendonodered.com

String ledTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/led";
String ledBuiltinTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/ledbuiltin";
String pulsadorTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/pulsador";
String initTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/inicio";
String publish_15sec = "cursomqtt/" + String(ID_DISPOSITIVO) + "/dato15s";
String lwtTopic = "cursomqtt/" + String(ID_DISPOSITIVO) + "/status";

// connection state
boolean notConnected = true;

//MQTT client
PubSubClient client(gprsclient);

// variables para publicar periodicamente
long lastMsg = 0;
int value = 0;
char msg[50];

// variable pulsador
boolean anterior_pulsador;

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
      Serial.println("Enciendo led");
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.println("Apago led");
      digitalWrite(LED_BUILTIN, LOW);
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
    client.publish(initTopic.c_str(), texto.c_str());
    client.publish(lwtTopic.c_str(), "OK",true);
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
  pinMode(LED, OUTPUT);             //inicializamos salida para led
  pinMode(PULSADOR, INPUT_PULLUP);  // inicializamos entrada para pulsador

  anterior_pulsador = digitalRead(PULSADOR);

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

  //publicar cada 15 segundos
  long now = millis();
  if (now - lastMsg > 15000) {
    lastMsg = now;
    ++value;
    snprintf(msg, 50, "hello world 15s #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publish_15sec.c_str(), msg);
  }

  //comprobar pulsación pulsador
  boolean estado_pulsador = digitalRead(PULSADOR);
  if (anterior_pulsador != estado_pulsador) {
    anterior_pulsador = estado_pulsador;
    if (estado_pulsador == LOW) {  //flanco descendente pull-up
      Serial.println("Pulsador Presionado");
      client.publish(pulsadorTopic.c_str(), "Pulsador Presionado");
    } else {
      Serial.println("Pulsador Soltado");
      client.publish(pulsadorTopic.c_str(), "Pulsador Soltado");
    }
  }
}