#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

//RTC_DATA_ATTR aloca a variável na memoria RTC
RTC_DATA_ATTR int bootCount = 0;

#define USE_SERIAL Serial
DynamicJsonDocument doc(512);
int pinon = 3;
const uint8_t trig_pin = 2;
const uint8_t echo_pin = 1;
uint32_t print_timer;
int distancia;
int counter;
int medida;
char data[80];
char hora[80];


const char* ssid = "EBR-TAB";
const char* password = "oe9-oe4r35j@!";
const char* website = "https://tabua-de-mare.vercel.app/send";

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = -5400;
const int daylightOffset_sec = -5400;

const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";


void printLocalTime() {
  
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);
  
  strftime(data, 80, "%d/%m/%Y", timeinfo);
  Serial.println(data);
  strftime(hora, 80, "%H:%M:%S", timeinfo);
  Serial.println(hora);

  timeinfo = localtime(&now-60);

   
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void setup() {

  //Wire.begin();
  pinMode(pinon, OUTPUT);
  USE_SERIAL.begin(115200);
  
  digitalWrite(pinon, HIGH);
  ++bootCount;
  counter = bootCount;
  WiFi.begin(ssid, password);

  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  digitalWrite(trig_pin, LOW);
}


void loop() {

      // Espera 0,5s (500ms) entre medições.
  if (millis() - print_timer > 500) {
  print_timer = millis();
  // Pulso de 5V por pelo menos 10us para iniciar medição.
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(11);
  digitalWrite(trig_pin, LOW);
  /* Mede quanto tempo o pino de echo ficou no estado alto, ou seja,
  o tempo de propagação da onda. */
  uint32_t pulse_time = pulseIn(echo_pin, HIGH);
  /* A distância entre o sensor ultrassom e o objeto será proporcional a velocidade
  do som no meio e a metade do tempo de propagação. Para o ar na
  temperatura ambiente Vsom = 0,0343 cm/us. */
  float distancia = 0.01715 * pulse_time;
  // Imprimimos o valor na porta serial;
  
  Serial.print(distancia, 2);
  Serial.println(" cm");
   
  float distanceM = (0.01715 * pulse_time) /100.0;
  Serial.print("Distância(m):");
  Serial.println(distanceM, 2);
  
  Serial.println();

  sntp_set_time_sync_notification_cb(timeavailable);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  delay(5000);
  sntp_set_time_sync_notification_cb(timeavailable);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  

  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(website);
    
    printLocalTime();
    

    doc["api_key"] = API_KEY;
    doc["counter"] = counter;
    doc["medida"] = String(distanceM, 2);
    doc["data"] = data;
    doc["hora"] = hora;
    
    http.addHeader("Content-Type", "application/json");
    String output;
    serializeJson(doc, output);
    String httpResponseCode = output;
    int httpCode = http.POST(httpResponseCode);

    if (httpCode > 0) {
      USE_SERIAL.printf("[HTTP] POST... code %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USE_SERIAL.println(payload);
      }
    } else {
      USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

      http.end();
      digitalWrite(pinon, LOW);
      esp_sleep_enable_timer_wakeup(300 * 1000000);
      Serial.println("Processo concluido, entrando em modo sleep!");
      esp_deep_sleep_start();
  }
  } else {
      digitalWrite(pinon, LOW);
      esp_sleep_enable_timer_wakeup(300 * 1000000);
      Serial.println("Falha de WIFI, entrando em modo sleep!");
      esp_deep_sleep_start();
    }

  
  
}
