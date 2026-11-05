// M5Stack HIT ALARM for the flight-twin page
//
// When a target hits the plane on the web page, the page publishes to
//   ifm-met/iiot/m5/<PLANE_ID>/hit        payload: {"n":<hit count>,"t":<ms>}
// This device subscribes to it, flashes the screen, buzzes (vibration motor on a Core2)
// and beeps. Button A switches the sound on and off. It reports its state on
//   ifm-met/iiot/m5/<PLANE_ID>/alarm      "sound on" / "sound off" / "offline" (retained, offline is the last will)
//
// Libraries (Arduino IDE > Library Manager): M5Unified, PubSubClient (by Nick O'Leary)
// Works on any M5Stack (Core2, Basic/Gray/Fire, CoreS3, StickC Plus...). Models without a
// vibration motor use a low buzz tone instead. Wi-Fi: plain WPA2, 2.4 GHz (a phone hotspot is ideal).

#include <M5Unified.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---- edit these lines ----
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char* PLANE_ID  = "anand-m5";          // the plane's device id (same as ?id= on the page)
// --------------------------

const char* BROKER = "broker.hivemq.com";
const uint16_t PORT = 1883;

WiFiClient net;
PubSubClient mqtt(net);
char tHit[64], tAlarm[64];
bool soundOn = true;
uint32_t hits = 0;
uint32_t alarmStart = 0;      // when the current alarm began (0 = idle)
bool beep2 = false;           // second beep of the current alarm played
bool hasMotor = false;

void drawIdle() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextSize(2);
  M5.Display.drawString("HIT ALARM", 8, 8);
  M5.Display.setTextSize(1);
  M5.Display.drawString(String("plane: ") + PLANE_ID, 8, 32);
  M5.Display.setTextSize(3);
  M5.Display.drawString(String("Hits ") + hits, 8, M5.Display.height() / 2 - 20);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(soundOn ? TFT_GREEN : TFT_ORANGE, TFT_BLACK);
  M5.Display.drawString(soundOn ? "Sound ON " : "Sound OFF", 8, M5.Display.height() - 52);
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.drawString("Button A: sound on/off", 8, M5.Display.height() - 22);
}

void publishState() {
  mqtt.publish(tAlarm, soundOn ? "sound on" : "sound off", true);
}

void onMessage(char* topic, byte* payload, unsigned int len) {
  if (strcmp(topic, tHit) != 0) return;
  hits++;
  alarmStart = millis();
  beep2 = false;
  // screen flash
  M5.Display.fillScreen(TFT_RED);
  M5.Display.setTextColor(TFT_WHITE, TFT_RED);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(5);
  M5.Display.drawString("HIT!", M5.Display.width() / 2, M5.Display.height() / 2 - 10);
  M5.Display.setTextSize(2);
  M5.Display.drawString(String("#") + hits, M5.Display.width() / 2, M5.Display.height() / 2 + 40);
  // buzz: vibration motor where there is one
  if (hasMotor) M5.Power.setVibration(200);
  // beep: two short tones, plus a low buzz on models without a motor
  if (soundOn) {
    M5.Speaker.tone(2600, 110);
    if (!hasMotor) M5.Speaker.tone(180, 300, 1);   // second channel: low buzz
  }
}

void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawString("Wi-Fi...", 8, 8);
  M5.Display.drawString(WIFI_SSID, 8, 34);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 20000) delay(250);
}

void ensureMqtt() {
  if (mqtt.connected()) return;
  ensureWifi();
  if (WiFi.status() != WL_CONNECTED) { M5.Display.drawString("No Wi-Fi", 8, 70); delay(2000); return; }
  M5.Display.drawString("MQTT...", 8, 70);
  String cid = String("m5alarm-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  if (mqtt.connect(cid.c_str(), tAlarm, 0, true, "offline")) {
    mqtt.subscribe(tHit);
    publishState();
    drawIdle();
  } else {
    M5.Display.drawString("MQTT failed", 8, 100);
    delay(2000);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Speaker.setVolume(200);
  auto b = M5.getBoard();
  hasMotor = (b == m5::board_t::board_M5StackCore2 || b == m5::board_t::board_M5Tough);
  snprintf(tHit, sizeof tHit, "ifm-met/iiot/m5/%s/hit", PLANE_ID);
  snprintf(tAlarm, sizeof tAlarm, "ifm-met/iiot/m5/%s/alarm", PLANE_ID);
  mqtt.setServer(BROKER, PORT);
  mqtt.setCallback(onMessage);
  mqtt.setKeepAlive(15);
  ensureMqtt();
}

void loop() {
  M5.update();
  if (!mqtt.connected()) { ensureMqtt(); return; }
  mqtt.loop();

  if (M5.BtnA.wasPressed()) {
    soundOn = !soundOn;
    if (soundOn) M5.Speaker.tone(1800, 60);    // short confirmation chirp
    publishState();
    if (!alarmStart) drawIdle();
  }

  if (alarmStart) {
    uint32_t dt = millis() - alarmStart;
    if (soundOn && !beep2 && dt > 150) { M5.Speaker.tone(1700, 160); beep2 = true; }   // second, lower beep
    if (hasMotor && dt > 350) M5.Power.setVibration(0);
    if (dt > 900) { alarmStart = 0; if (hasMotor) M5.Power.setVibration(0); drawIdle(); }
  }
}
