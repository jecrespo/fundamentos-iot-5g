#include "DHTStable.h"

#define LED_ROJO 7
#define LED_AMARILLO 8
#define PULSADOR 5
#define LDR A1
#define DHT22 9

DHTStable DHT;

// variables para publicar periodicamente
long lastMsg = 0;
long lastMsg2 = 0;

// variable pulsador
boolean anterior_pulsador;
boolean estado_led_rojo = 0;
boolean estado_led_amarillo = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  delay(2000);

  Serial.println("Starting Arduino MKR1500...");
  Serial.println("Initializing ports...");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);        //inicializamos salida para led
  pinMode(LED_AMARILLO, OUTPUT);    //inicializamos salida para led
  pinMode(PULSADOR, INPUT_PULLUP);  // inicializamos entrada para pulsador

  anterior_pulsador = digitalRead(PULSADOR);

  delay(1500);
}

void loop() {
  //publicar cada 15 segundos
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    digitalWrite(LED_ROJO, estado_led_rojo);
    estado_led_rojo = !estado_led_rojo;
    int chk = DHT.read22(DHT22);
    if (chk == DHTLIB_OK) {
      Serial.print("Temperatura: ");
      Serial.println(DHT.getTemperature(), 1);
      Serial.print("Humedad: ");
      Serial.println(DHT.getHumidity(), 1);
    } else {
      Serial.println("Error sonda");
    }
  }

  if (now - lastMsg2 > 1000) {
    lastMsg2 = now;
    digitalWrite(LED_AMARILLO, !estado_led_amarillo);
    estado_led_amarillo = !estado_led_amarillo;
    Serial.print("LDR: ");
    Serial.println(analogRead(LDR));
  }

  //comprobar pulsación pulsador
  boolean estado_pulsador = digitalRead(PULSADOR);
  if (anterior_pulsador != estado_pulsador) {
    anterior_pulsador = estado_pulsador;
    if (estado_pulsador == LOW) {  //flanco descendente pull-up
      Serial.println("Pulsador Presionado");
    } else {
      Serial.println("Pulsador Soltado");
    }
  }
}
