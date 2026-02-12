#include <Arduino.h>
#include <DHTesp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <I2Cdev.h>
#include <Ticker.h>
// #include <mpu.h>
#include <oled.h>

#include <atomic>
#include <string>

using WebServer = ESP8266WebServer;

DHTesp dht;

Ticker ticker;

WebServer server{80};

uint64_t seconds;
uint64_t reportSecond = -1;

uint16_t t;
uint16_t h;

char buf[16];

auto base_url = "http://150.230.163.129/api/sensor/put?data=";

uint32_t parseInt(const String& str) {
  return std::atoi(server.arg(str).c_str());
}

int getTemperature() {
  // 热敏电阻传感参数
  const float K = 273.15;
  const float T2 = K + 25;
  // const float B = 3435.0;
  const float B = 3950.0;
  // const float R = 10000.0;  // 10k热敏电阻

  int v = analogRead(A0);

  Serial.print("Analog read A0: ");
  Serial.println(v);

  int t = (1 / (log(v / (1024.f - v)) / B + 1 / T2) - K) * 10;
  return t;
}

void initHttpServer() {
  server.on("/led", []() {
    auto v = parseInt("v");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, v ? LOW : HIGH);  // invert for builtin led

    Serial.print("led: ");
    Serial.println(v);

    auto reply = "running " + std::to_string(seconds) + " seconds.\n";
    server.send(200, "text/plain", reply.c_str());
  });

  pinMode(A0, INPUT);
  server.on("/temperature", []() {
    auto reply = std::to_string(t / 10) + std::string(".") +
                 std::to_string(t % 10) + "°C\n";
    server.send(200, "text/plain", reply.c_str());
  });

  server.on("/gpio", []() {
    auto pin = parseInt("pin");
    pinMode(pin, OUTPUT);
    auto v = parseInt("v");
    digitalWrite(pin, v ? HIGH : LOW);
    server.send(200, "text/plain", "OK\n");
  });

  // server.on("/mpu", []() {
  //   MPU.update();
  //   server.send(200, "text/plain", MPU.to_string());
  // });

  server.begin();
}

void setup() {
  seconds = 0;
  Serial.begin(115200);

  // Wire.setClock(1000);
  Wire.begin();
  // MPU.begin();

  OLED.begin();
  OLED.drawString(0, 0, "Connecting...");

  delay(1000);
  // put your setup code here, to run once:
  WiFi.begin("501A", "13855313958");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  OLED.clear();
  OLED.drawString(0, 0, "CONNECTED");
  delay(1000);
  OLED.clear();

  Serial.println();
  Serial.print("Connected, IP: ");
  Serial.println(WiFi.localIP());

  initHttpServer();

  dht.setup(16, DHTesp::DHT11);

  t = getTemperature();
  while (true) {
    auto th = dht.getTempAndHumidity();
    if (dht.getStatus() == DHTesp::ERROR_NONE) {
      h = th.humidity * 10;
      break;
    }
    Serial.print("DHT11 error: ");
    Serial.println(dht.getStatusString());
    delay(1000);
  }

  ticker.attach(1, []() {
    ++seconds;
    snprintf(buf, sizeof(buf), "%llu", seconds);
    OLED.drawString(0, 0, buf);
    t = getTemperature();
    snprintf(buf, sizeof(buf), "%d.%d C", t / 10, t % 10);
    OLED.drawString(1, 0, buf);
    if (seconds % 3 == 0) {
      Serial.println("DHT11 read");
      auto th = dht.getTempAndHumidity();
      if (dht.getStatus() == DHTesp::ERROR_NONE) {
        snprintf(buf, sizeof(buf), "%0.2f", th.temperature);
        OLED.drawString(2, 0, buf);
        snprintf(buf, sizeof(buf), "%0.2f", th.humidity);
        OLED.drawString(3, 0, buf);
        h = th.humidity * 10;
      } else {
        Serial.print("DHT11 error: ");
        Serial.println(dht.getStatusString());
      }
    }
  });
}

void loop() {
  // put your main code here, to run repeatedly:
  if (seconds % 60 == 0) {
    if (seconds != reportSecond) {
      WiFiClient client;
      HTTPClient http;
      Serial.println("reporting temperature to server...");
      auto url = base_url + String(t) + "%3A" + String(h);
      http.begin(client, url);
      int httpCode = http.GET();
      Serial.print("report temperature code: ");
      Serial.print(httpCode);
      Serial.print(", reply: ");
      Serial.println(http.getString());
      http.end();
      if (httpCode == 200) {
        reportSecond = seconds;
        Serial.println("report temperature success");
        // snprintf(buf, sizeof(buf), "put %llu", reportSecond);
        // OLED.drawString(2, 0, buf);
      }
    }
  }

  server.handleClient();
}
