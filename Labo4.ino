#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/*************** WiFi (de la guía) *****************/
#define WLAN_SSID "ARTEFACTOS"
#define WLAN_PASS "ARTEFACTOS"

/*************** Adafruit IO (MQTT) ***************/
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // Usa 8883 con WiFiClientSecure para TLS
#define AIO_USERNAME "GuillePereira" // <-- tu usuario AIO
#define AIO_KEY "aio_aio_QBPV68kSJmqrLNagz52jr5EBpPKK" // <-- tu key AIO

/*************** Pines ESP32 ***********************/
const int LM35_PIN = 34; // ADC1 (recomendado con WiFi activo)
const int LED_PIN = 4;   // opcional: LED externo

/*************** MQTT Cliente *********************/
WiFiClient client;
// Si quisieras TLS, usa: WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(
  &client,
  AIO_SERVER,
  AIO_SERVERPORT,
  AIO_USERNAME, // clientID (puede ser igual al username)
  AIO_USERNAME, // username para MQTT (Adafruit IO usa tu AIO username)
  AIO_KEY       // password/KEY
);

/*************** Feeds AIO *************************/
// Publicar temperatura en el feed "temperatura"
Adafruit_MQTT_Publish temperatura_feed = Adafruit_MQTT_Publish(
  &mqtt, AIO_USERNAME "/feeds/temperatura"
);

/*************** Tiempos **************************/
unsigned long lastPub = 0;
const unsigned long PUB_EVERY_MS = 2000; // publica cada 2s

/*************** Utilidades ***********************/
void connectWiFi() {
  Serial.print("Conectando a WiFi ");
  Serial.println(WLAN_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print("\nConectado. IP: ");
  Serial.println(WiFi.localIP());
}

void MQTT_connect() {
  if (mqtt.connected()) return;

  Serial.print("Conectando a Adafruit IO (MQTT) ... ");
  int8_t ret;
  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) { // 0 = conectado
    Serial.print("Error MQTT: ");
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando en 5s...");
    mqtt.disconnect();
    delay(5000);
    if (--retries == 0) {
      Serial.println("Sin conexión MQTT. Verifica credenciales/red y reinicia.");
      return;
    }
  }
  Serial.println("¡MQTT conectado!");
}

/**
 * Lectura de temperatura LM35 con impresión en Serial.
 * - ADC ESP32: 12 bits (0-4095), Vref ≈ 3.3V
 * - LM35: 10 mV/°C => 0.01 V/°C
 * Fórmula: T(°C) = (ADC/4095) * 3.3 * 100
 */
float readTempC() {
  // Opcional: mejora rango/linealidad del ADC
  // analogReadResolution(12); // por defecto ya es 12 bits
  // analogSetAttenuation(ADC_11db); // ~3.6V máx. medible (global)
  // analogSetPinAttenuation(LM35_PIN, ADC_11db);

  // Promedio simple para reducir ruido:
  const int N = 10;
  uint32_t acc = 0;
  for (int i = 0; i < N; i++) {
    acc += analogRead(LM35_PIN);
    delay(2);
  }
  int raw = acc / N;

  // Conversión a voltaje y luego a °C
  float volts = raw * (3.3f / 4095.0f);
  float tempC = volts * 100.0f;

  // Imprimir datos en monitor serial
  Serial.print("Lectura ADC: ");
  Serial.print(raw);
  Serial.print(" | Voltaje: ");
  Serial.print(volts, 3); // 3 decimales
  Serial.print(" V");
  Serial.print(" | Temperatura: ");
  Serial.print(tempC, 2); // 2 decimales
  Serial.println(" °C");

  return tempC;
}

/******************** Setup ***********************/
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectWiFi();
  // Si vas a usar TLS, configura WiFiClientSecure y cambia AIO_SERVERPORT a 8883
}

/********************* Loop ***********************/
void loop() {
  MQTT_connect();

  // Publica temperatura cada 2 s
  unsigned long now = millis();
  if (now - lastPub >= PUB_EVERY_MS) {
    lastPub = now;

    float tC = readTempC(); // ya imprime raw/volts/°C en Serial

    if (!temperatura_feed.publish(tC)) {
      Serial.println("❌ Publicación fallida.");
    } else {
      Serial.println("✅ Publicación OK.");
    }
  }

  // Mantener viva la conexión MQTT
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
}
