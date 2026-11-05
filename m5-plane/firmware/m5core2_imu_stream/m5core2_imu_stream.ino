// M5Stack IMU -> Wi-Fi -> MQTT -> the flight-twin web page
//
// Samples the IMU 100 times a second and publishes 10 samples per message (10 messages a second) to
//   ifm-met/iiot/m5/<DEVICE_ID>/imu      payload: "ax,ay,az,gx,gy,gz;ax,ay,..."  (accel in g, gyro in deg/s)
//   ifm-met/iiot/m5/<DEVICE_ID>/status   "online" / "offline" (retained, offline is the last will)
// Button A publishes "zero" on .../cmd so the page re-levels the plane.
//
// Core2 version: libraries M5Core2 (by M5Stack) and PubSubClient (by Nick O'Leary). Board: M5Core2.
// For other M5Stack models use m5_imu_stream (M5Unified).
// Wi-Fi must be plain WPA2 with a password (a phone hotspot is ideal; eduroam will not work).

#include <M5Core2.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---- edit these three lines ----
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* DEVICE_ID = "anand-m5";            // must match ?id= on the page (default anand-m5)
// --------------------------------

const char* BROKER = "broker.hivemq.com";      // free public MQTT broker; the page connects to it over WebSockets
const uint16_t PORT = 1883;

WiFiClient net;
PubSubClient mqtt(net);
char tImu[64], tStatus[64], tCmd[64];
char buf[900];
int nBuf = 0, nSamples = 0;
uint32_t next = 0, sent = 0, lastScreen = 0;

void show(const char* line1, const char* line2 = "") {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(4, 4);
  M5.Lcd.println(line1);
  M5.Lcd.println(line2);
}

void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  show("Wi-Fi...", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 20000) delay(250);
}

void ensureMqtt() {
  if (mqtt.connected()) return;
  ensureWifi();
  if (WiFi.status() != WL_CONNECTED) { show("No Wi-Fi", "check SSID/pass"); delay(2000); return; }
  show("MQTT...", BROKER);
  String cid = String("m5-") + DEVICE_ID + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (mqtt.connect(cid.c_str(), tStatus, 0, true, "offline")) {
    mqtt.publish(tStatus, "online", true);
    show("Streaming", DEVICE_ID);
  } else {
    show("MQTT failed", "retrying");
    delay(2000);
  }
}

void setup() {
  M5.begin();        // LCD, power, touch buttons, serial
  M5.IMU.Init();
  M5.Lcd.setTextSize(2);
  snprintf(tImu, sizeof tImu, "ifm-met/iiot/m5/%s/imu", DEVICE_ID);
  snprintf(tStatus, sizeof tStatus, "ifm-met/iiot/m5/%s/status", DEVICE_ID);
  snprintf(tCmd, sizeof tCmd, "ifm-met/iiot/m5/%s/cmd", DEVICE_ID);
  mqtt.setServer(BROKER, PORT);
  mqtt.setBufferSize(1024);
  mqtt.setKeepAlive(15);
  ensureMqtt();
  next = micros();
}

void loop() {
  if (!mqtt.connected()) { ensureMqtt(); next = micros(); nBuf = nSamples = 0; return; }
  mqtt.loop();
  M5.update();
  if (M5.BtnA.wasPressed()) mqtt.publish(tCmd, "zero");

  if ((int32_t)(micros() - next) < 0) return;
  next += 10000;                                   // 100 Hz
  float ax, ay, az, gx, gy, gz;
  M5.IMU.getAccelData(&ax, &ay, &az);
  M5.IMU.getGyroData(&gx, &gy, &gz);
  nBuf += snprintf(buf + nBuf, sizeof(buf) - nBuf, "%s%.3f,%.3f,%.3f,%.1f,%.1f,%.1f",
                   nSamples ? ";" : "", ax, ay, az, gx, gy, gz);
  if (++nSamples >= 10) {
    mqtt.publish(tImu, buf);
    nBuf = nSamples = 0; sent++;
  }
  if (millis() - lastScreen > 1000) {
    lastScreen = millis();
    M5.Lcd.setCursor(4, 60);
    M5.Lcd.printf("msgs %lu  \nax %+.2f  \nay %+.2f  \naz %+.2f  ", (unsigned long)sent, ax, ay, az);
  }
}
