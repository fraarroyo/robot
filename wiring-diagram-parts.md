# Battle Bot Wiring Diagram with Parts Visualization

## Parts List

- Robot ESP32 board
- 4 continuous-rotation servos (for legs)
- 3 ultrasonic sensors
- 8x8 LED matrix module (I2C)
- 2 indicator LEDs (status, attack)
- WiFi joystick controller ESP32
- Analog joystick module
- 2 push buttons (emoji, dance)
- Power supply for robot and servos
- Jumper wires and common ground

---

## Robot Wiring Visualization

### Robot components

- `Robot ESP32`
- `Servo FL` = front-left leg servo
- `Servo FR` = front-right leg servo
- `Servo BL` = back-left leg servo
- `Servo BR` = back-right leg servo
- `Ultrasonic front` sensor
- `Ultrasonic left` sensor
- `Ultrasonic right` sensor
- `LED matrix` face display
- `Status LED`
- `Attack LED`

### Robot connections

Robot ESP32
- GPIO `26` → Servo FL signal
- GPIO `33` → Servo FR signal
- GPIO `17` → Servo BL signal
- GPIO `4`  → Servo BR signal
- GPIO `15` → Ultrasonic front TRIG
- GPIO `14` → Ultrasonic front ECHO
- GPIO `13` → Ultrasonic left TRIG
- GPIO `12` → Ultrasonic left ECHO
- GPIO `27` → Ultrasonic right TRIG
- GPIO `35` → Ultrasonic right ECHO
- GPIO `21` → LED matrix SDA
- GPIO `22` → LED matrix SCL
- GPIO `5`  → Status LED
- GPIO `18` → Attack LED

Power:
- Servos VCC → 5V power supply
- Servos GND → common ground with ESP32
- LED matrix VCC → 5V or 3.3V per module
- LED matrix GND → common ground
- Ultrasonic sensors VCC → 5V
- Ultrasonic sensors GND → common ground

---

## Controller Wiring Visualization

### Controller components

- `Controller ESP32`
- `Analog Joystick`
- `Emoji button`
- `Dance button`

### Controller connections

Controller ESP32
- GPIO `34` → joystick X signal
- GPIO `35` → joystick Y signal
- GPIO `32` → emoji button
- GPIO `25` → dance button

Power:
- Joystick VCC → 5V
- Joystick GND → GND
- Buttons second terminal → GND
- Controller power → USB or 5V supply

---

## Wireless Link

- Robot AP SSID: `BattleBotAP`
- Password: `battlebot123`
- Robot IP: `192.168.4.1`
- Robot UDP port: `4210`
- Controller local UDP port: `4211`

Controller sends packets like:
- `X:2048 Y:2048 E:0 D:0`
- `X:1024 Y:2048 E:1 D:0`
- `X:2048 Y:2048 E:0 D:1`

---

## Visualization Diagram (Text)

```
               [Robot ESP32]
                |     |    \
      +---------+     |     +------------------+
      |               |                        |
 [Servo FL]       [Ultrasonic Front]     [LED Matrix]
      |               |                        |
 [Servo FR]       [Ultrasonic Left]      [Status LED]
      |               |                        |
 [Servo BL]       [Ultrasonic Right]     [Attack LED]
      |
 [Servo BR]
```

```
               [Controller ESP32]
                  |        |      \
             [Joystick X] [Joystick Y]
                  |        |
             [Emoji Button] [Dance Button]
```

---

## Notes

- The robot has a parts-based wiring layout: servos control each leg, sensors provide obstacle detection, and the LED matrix displays face/emojis.
- The controller ESP32 sends wireless joystick and command data to the robot.
- Make sure all grounds are common between the robot board, servos, sensors, and LED module.
