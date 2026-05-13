/*
 * Configuration File for Battle Bot
 * Pin definitions and tunable parameters
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// ESP32 PIN DEFINITIONS
// ============================================

// Servo Pins for continuous-rotation legs
#define SERVO_FL_PIN  26  // Front-left leg servo
#define SERVO_FR_PIN  33  // Front-right leg servo
#define SERVO_BL_PIN  17  // Back-left leg servo
#define SERVO_BR_PIN  4   // Back-right leg servo

// Ultrasonic Sensor Pins - Front
#define SENSOR_TRIG_FRONT  15  // Trigger pin
#define SENSOR_ECHO_FRONT  14  // Echo pin

// Ultrasonic Sensor Pins - Left
#define SENSOR_TRIG_LEFT   13  // Trigger pin
#define SENSOR_ECHO_LEFT   12  // Echo pin

// Ultrasonic Sensor Pins - Right
#define SENSOR_TRIG_RIGHT  27  // Trigger pin
#define SENSOR_ECHO_RIGHT  35  // Echo pin

// Indicator LEDs
#define LED_STATUS  5   // Green LED - Bot status
#define LED_ATTACK  18  // Red LED - Attack indicator

// ============================================
// SERVO LEG SETTINGS
// ============================================
#define SERVO_STOP_PWM      1500  // Microseconds for stop
#define SERVO_MAX_DELTA     400   // Microsecond range from stop to full speed

// ============================================
// MOTOR SPEED SETTINGS
// ============================================
#define MAX_SPEED        255  // Maximum motor speed
#define ATTACK_SPEED     200  // Speed when attacking
#define SEEK_SPEED       150  // Speed when seeking
#define RETREAT_SPEED    180  // Speed when retreating
#define SPIN_SPEED       180  // Speed when spinning

// Joystick Input Pins
#define JOYSTICK_X_PIN       34   // Analog X axis
#define JOYSTICK_Y_PIN       39   // Analog Y axis
#define JOYSTICK_BUTTON_PIN  36   // Optional push button for manual enable
#define JOYSTICK_CENTER      2048 // ESP32 ADC midpoint for 12-bit resolution
#define JOYSTICK_DEADZONE    150  // Deadzone around center

// ============================================
// SENSOR THRESHOLDS (in centimeters)
// ============================================
#define ATTACK_DISTANCE    40  // Distance to start attacking
#define SEEK_DISTANCE      80  // Distance to detect opponent while seeking
#define EMERGENCY_DISTANCE 15  // Distance to trigger emergency retreat
#define MAX_SENSOR_RANGE   300 // Maximum sensor detection range

// ============================================
// BEHAVIOR PARAMETERS
// ============================================
#define LOOP_DELAY        100 // Main loop delay in milliseconds
#define DEBUG_INTERVAL    500 // Debug output interval in milliseconds
#define SENSOR_TIMEOUT    30000 // Ultrasonic sensor timeout in microseconds

// ============================================
// WiFi SETTINGS (Optional)
// ============================================
#define SSID            "BattleBotAP"
#define PASSWORD        "battlebot123"
#define WIFI_ENABLED    true  // Set to true to enable WiFi access point

// ============================================
// LED FACE DISPLAY
// ============================================
#define FACE_MATRIX_I2C_ADDRESS 0x70
#define EMOJI_DURATION_MS       3000
#define DANCE_DURATION_MS       5000
#define DANCE_STEP_MS           200

// ============================================
// Wireless Joystick Settings
// ============================================
#define JOYSTICK_WIFI_PORT      4210  // UDP port for joystick packets
#define JOYSTICK_UDP_BUFFER_SIZE 64   // Packet buffer size
#define WEB_CONTROL_TIMEOUT_MS  2500  // milliseconds to keep web control active

// Wireless joystick packet format:
//   X:2048 Y:2048 B:0
//   X:2048,Y:2048,B:1
// X and Y are 0-4095 (ESP32 ADC range), B is 0 or 1.

#endif // CONFIG_H
