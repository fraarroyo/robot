#include <WiFi.h>
#include <WiFiUdp.h>

// Wireless robot AP settings
const char* ssid = "BattleBotAP";
const char* password = "battlebot123";
IPAddress robotIp(192, 168, 4, 1);
const unsigned int robotPort = 4210;
const unsigned int localPort = 4211;

// Joystick input pins
const int JOY_X_PIN = 34;
const int JOY_Y_PIN = 35;
const int JOY_EMOJI_BUTTON_PIN = 32;
const int JOY_DANCE_BUTTON_PIN = 25;

// Joystick packet settings
const int JOY_CENTER = 2048;
const int JOY_DEADZONE = 150;
const unsigned long SEND_INTERVAL = 100;

WiFiUDP udp;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(JOY_EMOJI_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOY_DANCE_BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  if (udp.begin(localPort) == 1) {
    Serial.print("UDP local port: ");
    Serial.println(localPort);
  } else {
    Serial.println("Failed to start UDP");
  }
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend < SEND_INTERVAL) {
    return;
  }
  lastSend = now;

  int x = analogRead(JOY_X_PIN);
  int y = analogRead(JOY_Y_PIN);
  bool emojiPressed = digitalRead(JOY_EMOJI_BUTTON_PIN) == LOW;
  bool dancePressed = digitalRead(JOY_DANCE_BUTTON_PIN) == LOW;

  // Normalize noisy centered values
  if (abs(x - JOY_CENTER) < JOY_DEADZONE) {
    x = JOY_CENTER;
  }
  if (abs(y - JOY_CENTER) < JOY_DEADZONE) {
    y = JOY_CENTER;
  }

  char packet[64];
  int len = snprintf(packet, sizeof(packet), "X:%d Y:%d E:%d D:%d", x, y, emojiPressed ? 1 : 0, dancePressed ? 1 : 0);
  if (len < 0) {
    return;
  }

  udp.beginPacket(robotIp, robotPort);
  udp.write((uint8_t*)packet, len);
  udp.endPacket();

  Serial.print("Sent: ");
  Serial.println(packet);
}
