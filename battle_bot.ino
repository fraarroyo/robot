/*
 * Battle Bot Code for ESP32
 * A small autonomous battle robot that detects and fights opponents
 * Controls 4 DC motors and uses ultrasonic sensors for obstacle detection
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include "config.h"

// Leg servo objects
Servo legFL;
Servo legFR;
Servo legBL;
Servo legBR;

Adafruit_8x8matrix faceMatrix = Adafruit_8x8matrix();
WebServer server(80);
WiFiUDP joystickUDP;

bool webControlActive = false;
unsigned long webControlTimeout = 0;
int webJoystickX = JOYSTICK_CENTER;
int webJoystickY = JOYSTICK_CENTER;
bool webEmoji = false;
bool webDance = false;

const char webAppHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Battle Bot Web Controller</title>
  <style>
    body { font-family: Arial, sans-serif; background:#111; color:#eee; margin:0; padding:20px; }
    .container { max-width:520px; margin:0 auto; }
    h1 { text-align:center; }
    .control { margin:16px 0; padding:16px; background:#1f1f1f; border-radius:16px; }
    label { display:block; margin:10px 0 6px; }
    .joystick-wrapper { position:relative; width:100%; padding-top:100%; margin:20px 0; }
    .joystick { position:absolute; top:0; left:0; right:0; bottom:0; border: 3px solid #444; border-radius:50%; background: radial-gradient(circle at center, #2a2a2a 0%, #121212 70%); touch-action:none; }
    .handle { position:absolute; width:96px; height:96px; border-radius:50%; background:#2979ff; border:3px solid #fff; transform:translate(-50%, -50%); left:50%; top:50%; box-shadow:0 0 20px rgba(41,121,255,0.4); }
    button { width:100%; padding:14px; margin:10px 0; background:#2979ff; color:#fff; border:none; border-radius:10px; font-size:16px; cursor:pointer; }
    button:active { background:#1c54b2; }
    .row { display:flex; gap:10px; }
    .row button { flex:1; }
    .status { text-align:center; margin-top:12px; font-size:14px; color:#ccc; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Battle Bot Web Controller</h1>
    <div class="control">
      <div class="joystick-wrapper">
        <div id="joystick" class="joystick">
          <div id="handle" class="handle"></div>
        </div>
      </div>
      <div class="row">
        <button id="emojiBtn">Emoji Face</button>
        <button id="danceBtn">Dance</button>
      </div>
      <div class="status" id="status">Connected to robot</div>
    </div>
  </div>
  <script>
    const joystick = document.getElementById('joystick');
    const handle = document.getElementById('handle');
    const emojiBtn = document.getElementById('emojiBtn');
    const danceBtn = document.getElementById('danceBtn');
    const status = document.getElementById('status');
    let emoji = 0;
    let dance = 0;
    let lastSend = 0;
    let active = false;
    let pointerId = null;
    let xValue = 2048;
    let yValue = 2048;

    function clamp(val, min, max) {
      return val < min ? min : val > max ? max : val;
    }

    function setHandle(left, top) {
      handle.style.left = `${left}%`;
      handle.style.top = `${top}%`;
    }

    function sendControl() {
      const now = Date.now();
      if (now - lastSend < 80) return;
      lastSend = now;
      const url = `/control?x=${xValue}&y=${yValue}&e=${emoji}&d=${dance}`;
      fetch(url).then(r => r.text()).then(text => {
        status.textContent = text;
      }).catch(() => {
        status.textContent = 'Cannot reach robot';
      });
      emoji = 0;
      dance = 0;
    }

    function updateJoystick(clientX, clientY) {
      const rect = joystick.getBoundingClientRect();
      const dx = clientX - (rect.left + rect.width / 2);
      const dy = clientY - (rect.top + rect.height / 2);
      const radius = rect.width / 2 - 48;
      const dist = Math.min(Math.hypot(dx, dy), radius);
      const angle = Math.atan2(dy, dx);
      const boundedX = Math.cos(angle) * dist;
      const boundedY = Math.sin(angle) * dist;
      setHandle(50 + (boundedX / rect.width) * 100, 50 + (boundedY / rect.height) * 100);
      xValue = clamp(Math.round(2048 + (boundedX / radius) * 2048), 0, 4095);
      yValue = clamp(Math.round(2048 - (boundedY / radius) * 2048), 0, 4095);
      sendControl();
    }

    function resetJoystick() {
      xValue = 2048;
      yValue = 2048;
      setHandle(50, 50);
      sendControl();
    }

    joystick.addEventListener('pointerdown', e => {
      active = true;
      pointerId = e.pointerId;
      joystick.setPointerCapture(pointerId);
      updateJoystick(e.clientX, e.clientY);
    });

    joystick.addEventListener('pointermove', e => {
      if (!active || e.pointerId !== pointerId) return;
      updateJoystick(e.clientX, e.clientY);
    });

    joystick.addEventListener('pointerup', e => {
      if (e.pointerId !== pointerId) return;
      active = false;
      joystick.releasePointerCapture(pointerId);
      resetJoystick();
    });

    emojiBtn.addEventListener('click', () => { emoji = 1; sendControl(); });
    danceBtn.addEventListener('click', () => { dance = 1; sendControl(); });
    setInterval(sendControl, 250);
  </script>
</body>
</html>
)rawliteral";

// Sensor readings
int distanceFront = 0;
int distanceLeft = 0;
int distanceRight = 0;

// Joystick inputs
int joystickX = JOYSTICK_CENTER;
int joystickY = JOYSTICK_CENTER;
bool joystickButtonPressed = false;
bool manualMode = false;

// Display and gesture state
bool emojiActive = false;
bool danceActive = false;
unsigned long emojiEndTime = 0;
unsigned long danceEndTime = 0;
unsigned long lastDanceStepTime = 0;
int danceStep = 0;

// Robot state
enum BotState {
  IDLE,
  SEEKING,
  ATTACKING,
  RETREATING,
  SPINNING
};

BotState currentState = SEEKING;

// ============================================
// SETUP & INITIALIZATION
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nBattle Bot Starting...");
  
  // Attach leg servos
  legFL.attach(SERVO_FL_PIN);
  legFR.attach(SERVO_FR_PIN);
  legBL.attach(SERVO_BL_PIN);
  legBR.attach(SERVO_BR_PIN);
  
  // Set servos to neutral stop position
  setLegSpeed(legFL, 0);
  setLegSpeed(legFR, 0);
  setLegSpeed(legBL, 0);
  setLegSpeed(legBR, 0);

  // Initialize ultrasonic sensor pins
  pinMode(SENSOR_TRIG_FRONT, OUTPUT);
  pinMode(SENSOR_ECHO_FRONT, INPUT);
  pinMode(SENSOR_TRIG_LEFT, OUTPUT);
  pinMode(SENSOR_ECHO_LEFT, INPUT);
  pinMode(SENSOR_TRIG_RIGHT, OUTPUT);
  pinMode(SENSOR_ECHO_RIGHT, INPUT);
  
  // Initialize indicator LEDs
  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_ATTACK, OUTPUT);

  // Initialize joystick input
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize WiFi and wireless joystick support
  initWiFi();
  initWirelessJoystick();
  initWebServer();

  // Initialize face display
  initFaceDisplay();
  displayFace();
  
  stopBot();
  blinkStatus(3);
  
  Serial.println("Battle Bot Ready!");
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  // Read sensor data and joystick input
  updateSensorReadings();
  readJoystick();

  if (emojiActive) {
    displayEmoji();
  } else if (danceActive) {
    danceSequence();
  } else {
    if (manualMode) {
      executeMovement();
    } else {
      decideAction();
      executeMovement();
    }
    displayFace();
  }

  // Send data for debugging
  debugOutput();
  server.handleClient();
}

// ============================================
// SENSOR FUNCTIONS
// ============================================
int getDistance(int trigPin, int echoPin) {
  // Send ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure pulse duration
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  // Calculate distance in cm
  int distance = duration * 0.034 / 2;
  
  return (distance > 0 && distance < 300) ? distance : 300; // Max 300cm
}

void updateSensorReadings() {
  distanceFront = getDistance(SENSOR_TRIG_FRONT, SENSOR_ECHO_FRONT);
  distanceLeft = getDistance(SENSOR_TRIG_LEFT, SENSOR_ECHO_LEFT);
  distanceRight = getDistance(SENSOR_TRIG_RIGHT, SENSOR_ECHO_RIGHT);
}

// ============================================
// DECISION LOGIC
// ============================================
void decideAction() {
  // Check if opponent/obstacle detected in front
  if (distanceFront < ATTACK_DISTANCE && distanceFront > 0) {
    currentState = ATTACKING;
    digitalWrite(LED_ATTACK, HIGH);
  }
  // Emergency retreat if too close
  else if (distanceFront < EMERGENCY_DISTANCE && distanceFront > 0) {
    currentState = RETREATING;
    digitalWrite(LED_ATTACK, LOW);
  }
  // Check sides for opponents
  else if ((distanceLeft < SEEK_DISTANCE || distanceRight < SEEK_DISTANCE) && 
           distanceFront > ATTACK_DISTANCE) {
    currentState = SPINNING;
    digitalWrite(LED_ATTACK, LOW);
  }
  // Search for opponents
  else {
    currentState = SEEKING;
    digitalWrite(LED_ATTACK, LOW);
  }
}

// ============================================
// MOVEMENT FUNCTIONS
// ============================================
void moveForward(int speed) {
  setLegSpeed(legFL, speed);
  setLegSpeed(legFR, speed);
  setLegSpeed(legBL, speed);
  setLegSpeed(legBR, speed);
}

void moveBackward(int speed) {
  setLegSpeed(legFL, -speed);
  setLegSpeed(legFR, -speed);
  setLegSpeed(legBL, -speed);
  setLegSpeed(legBR, -speed);
}

void turnLeft(int speed) {
  setLegSpeed(legFL, -speed);
  setLegSpeed(legFR, speed);
  setLegSpeed(legBL, -speed);
  setLegSpeed(legBR, speed);
}

void turnRight(int speed) {
  setLegSpeed(legFL, speed);
  setLegSpeed(legFR, -speed);
  setLegSpeed(legBL, speed);
  setLegSpeed(legBR, -speed);
}

void spin(int speed) {
  setLegSpeed(legFL, -speed);
  setLegSpeed(legFR, speed);
  setLegSpeed(legBL, -speed);
  setLegSpeed(legBR, speed);
}

void stopBot() {
  setLegSpeed(legFL, 0);
  setLegSpeed(legFR, 0);
  setLegSpeed(legBL, 0);
  setLegSpeed(legBR, 0);
}

void setLegSpeed(Servo &leg, int speed) {
  if (speed > MAX_SPEED) speed = MAX_SPEED;
  if (speed < -MAX_SPEED) speed = -MAX_SPEED;

  int pulse = SERVO_STOP_PWM + (speed * SERVO_MAX_DELTA) / MAX_SPEED;
  pulse = constrain(pulse, SERVO_STOP_PWM - SERVO_MAX_DELTA, SERVO_STOP_PWM + SERVO_MAX_DELTA);
  leg.writeMicroseconds(pulse);
}

// ============================================
// EXECUTION
// ============================================
void executeMovement() {
  if (manualMode) {
    applyJoystickControl();
    return;
  }

  switch(currentState) {
    case ATTACKING:
      moveForward(MAX_SPEED);
      break;
      
    case RETREATING:
      moveBackward(RETREAT_SPEED);
      break;
      
    case SPINNING:
      spin(SPIN_SPEED);
      break;
      
    case SEEKING:
      moveForward(SEEK_SPEED);
      break;
      
    case IDLE:
    default:
      stopBot();
      break;
  }
}

void readJoystick() {
  if (webControlActive && millis() < webControlTimeout) {
    joystickX = webJoystickX;
    joystickY = webJoystickY;
    joystickButtonPressed = false;
    manualMode = true;
    if (webEmoji) {
      webEmoji = false;
      startEmoji();
    }
    if (webDance) {
      webDance = false;
      startDance();
    }
    return;
  }

  webControlActive = false;

  if (WIFI_ENABLED && readWirelessJoystick()) {
    return;
  }

  joystickX = analogRead(JOYSTICK_X_PIN);
  joystickY = analogRead(JOYSTICK_Y_PIN);
  joystickButtonPressed = false;

  int xDelta = joystickX - JOYSTICK_CENTER;
  int yDelta = joystickY - JOYSTICK_CENTER;
  manualMode = abs(xDelta) > JOYSTICK_DEADZONE || abs(yDelta) > JOYSTICK_DEADZONE;
}

// Read wireless joystick packets from UDP and update manual control.
// Expected packet formats: "X:2048 Y:2048 E:0 D:0" or "X:2048,Y:2048,E:1,D:0".
bool readWirelessJoystick() {
  int packetSize = joystickUDP.parsePacket();
  if (packetSize <= 0) {
    return false;
  }

  char packet[JOYSTICK_UDP_BUFFER_SIZE];
  int len = joystickUDP.read(packet, JOYSTICK_UDP_BUFFER_SIZE - 1);
  if (len <= 0) {
    return false;
  }
  packet[len] = '\0';

  int x = JOYSTICK_CENTER;
  int y = JOYSTICK_CENTER;
  int e = 0;
  int d = 0;
  parseJoystickPacket(packet, x, y, e, d);

  joystickX = x;
  joystickY = y;
  joystickButtonPressed = false;

  if (e != 0) {
    startEmoji();
  }
  if (d != 0) {
    startDance();
  }

  int xDelta = joystickX - JOYSTICK_CENTER;
  int yDelta = joystickY - JOYSTICK_CENTER;
  manualMode = abs(xDelta) > JOYSTICK_DEADZONE || abs(yDelta) > JOYSTICK_DEADZONE;
  return true;
}

void parseJoystickPacket(char *packet, int &x, int &y, int &emoji, int &dance) {
  char *token = strtok(packet, ", \t\n");
  while (token != nullptr) {
    if (token[0] == 'X' && token[1] == ':') {
      x = atoi(token + 2);
    } else if (token[0] == 'Y' && token[1] == ':') {
      y = atoi(token + 2);
    } else if (token[0] == 'E' && token[1] == ':') {
      emoji = atoi(token + 2);
    } else if (token[0] == 'D' && token[1] == ':') {
      dance = atoi(token + 2);
    }
    token = strtok(nullptr, ", \t\n");
  }
}

void startEmoji() {
  emojiActive = true;
  emojiEndTime = millis() + EMOJI_DURATION_MS;
  displayEmoji();
}

void startDance() {
  danceActive = true;
  danceEndTime = millis() + DANCE_DURATION_MS;
  lastDanceStepTime = 0;
  danceStep = 0;
}

void initFaceDisplay() {
  Wire.begin();
  faceMatrix.begin(FACE_MATRIX_I2C_ADDRESS);
  displayFace();
}

void displayPattern(const uint8_t pattern[8]) {
  faceMatrix.clear();
  faceMatrix.drawBitmap(0, 0, pattern, 8, 8, LED_ON);
  faceMatrix.writeDisplay();
}

void displayFace() {
  static const uint8_t faceNormal[8] = {
    0b00100100,
    0b00100100,
    0b00000000,
    0b01000010,
    0b00111100,
    0b00000000,
    0b00000000,
    0b00000000
  };
  displayPattern(faceNormal);
}

void displayEmoji() {
  static const uint8_t emojiFace[8] = {
    0b01100110,
    0b11111111,
    0b11111111,
    0b00111100,
    0b01000010,
    0b00111100,
    0b00000000,
    0b00000000
  };
  if (millis() > emojiEndTime) {
    emojiActive = false;
    displayFace();
    return;
  }
  displayPattern(emojiFace);
}

void danceSequence() {
  if (millis() > danceEndTime) {
    danceActive = false;
    stopBot();
    displayFace();
    return;
  }

  unsigned long now = millis();
  if (now - lastDanceStepTime > DANCE_STEP_MS) {
    danceStep = (danceStep + 1) % 4;
    lastDanceStepTime = now;
  }

  if (danceStep == 0) {
    turnRight(SEEK_SPEED);
  } else if (danceStep == 1) {
    turnLeft(SEEK_SPEED);
  } else if (danceStep == 2) {
    moveForward(SEEK_SPEED);
  } else {
    moveBackward(RETREAT_SPEED);
  }

  static const uint8_t danceFrames[4][8] = {
    {0b00100100,0b00100100,0b01000010,0b10011001,0b10011001,0b01000010,0b00111100,0b00000000},
    {0b00100100,0b00100100,0b01000010,0b10011001,0b10011001,0b01000010,0b00100100,0b00011000},
    {0b00100100,0b00100100,0b01011010,0b10011001,0b10011001,0b01011010,0b00100100,0b00000000},
    {0b00100100,0b00100100,0b01000010,0b10100101,0b10100101,0b01000010,0b00100100,0b00011000}
  };

  displayPattern(danceFrames[danceStep]);
}

void applyJoystickControl() {
  int xDelta = joystickX - JOYSTICK_CENTER;
  int yDelta = joystickY - JOYSTICK_CENTER;

  if (abs(xDelta) < JOYSTICK_DEADZONE) xDelta = 0;
  if (abs(yDelta) < JOYSTICK_DEADZONE) yDelta = 0;

  int forwardSpeed = map(yDelta, -JOYSTICK_CENTER, JOYSTICK_CENTER, -MAX_SPEED, MAX_SPEED);
  int turnSpeed = map(xDelta, -JOYSTICK_CENTER, JOYSTICK_CENTER, -MAX_SPEED, MAX_SPEED);

  int leftSpeed = constrain(forwardSpeed + turnSpeed, -MAX_SPEED, MAX_SPEED);
  int rightSpeed = constrain(forwardSpeed - turnSpeed, -MAX_SPEED, MAX_SPEED);

  setLegSpeed(legFL, leftSpeed);
  setLegSpeed(legBL, leftSpeed);
  setLegSpeed(legFR, rightSpeed);
  setLegSpeed(legBR, rightSpeed);
}

// ============================================
// UTILITY FUNCTIONS
// ============================================
void blinkStatus(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_STATUS, HIGH);
    delay(100);
    digitalWrite(LED_STATUS, LOW);
    delay(100);
  }
}

void debugOutput() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) { // Print every 500ms
    Serial.print("State: ");
    printState(currentState);
    Serial.print(" | Front: ");
    Serial.print(distanceFront);
    Serial.print("cm | Left: ");
    Serial.print(distanceLeft);
    Serial.print("cm | Right: ");
    Serial.print(distanceRight);
    Serial.print("cm");
    if (manualMode) {
      Serial.print(" | JOY X:");
      Serial.print(joystickX);
      Serial.print(" Y:");
      Serial.print(joystickY);
      Serial.print(" | MANUAL");
    }
    if (emojiActive) {
      Serial.print(" | EMOJI");
    }
    if (danceActive) {
      Serial.print(" | DANCE");
    }
    Serial.println();
    lastPrint = millis();
  }
}

void printState(BotState state) {
  switch(state) {
    case IDLE: Serial.print("IDLE"); break;
    case SEEKING: Serial.print("SEEKING"); break;
    case ATTACKING: Serial.print("ATTACKING"); break;
    case RETREATING: Serial.print("RETREATING"); break;
    case SPINNING: Serial.print("SPINNING"); break;
  }
}

// ============================================
// WiFi FUNCTIONS (Optional)
// ============================================
void initWiFi() {
  if (!WIFI_ENABLED) {
    return;
  }

  Serial.println("Starting WiFi access point...");
  WiFi.softAP(SSID, PASSWORD);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);
}

void initWirelessJoystick() {
  if (!WIFI_ENABLED) {
    return;
  }

  if (joystickUDP.begin(JOYSTICK_WIFI_PORT) == 1) {
    Serial.print("Joystick UDP listener started on port ");
    Serial.println(JOYSTICK_WIFI_PORT);
  } else {
    Serial.println("Failed to start joystick UDP listener");
  }
}

void initWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webAppHtml);
  });

  server.on("/control", HTTP_GET, []() {
    int x = server.hasArg("x") ? server.arg("x").toInt() : JOYSTICK_CENTER;
    int y = server.hasArg("y") ? server.arg("y").toInt() : JOYSTICK_CENTER;
    int e = server.hasArg("e") ? server.arg("e").toInt() : 0;
    int d = server.hasArg("d") ? server.arg("d").toInt() : 0;

    webJoystickX = constrain(x, 0, 4095);
    webJoystickY = constrain(y, 0, 4095);
    if (e != 0) {
      webEmoji = true;
    }
    if (d != 0) {
      webDance = true;
    }
    webControlActive = true;
    webControlTimeout = millis() + WEB_CONTROL_TIMEOUT_MS;

    server.send(200, "text/plain", "Web control active");
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("Web controller server started on port 80");
}
