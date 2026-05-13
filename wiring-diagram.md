# Battle Bot Wiring Diagram

## Robot ESP32 (Battle Bot)

### 1) Continuous-Rotation Leg Servos
- Front-left servo signal: GPIO `26`
- Front-right servo signal: GPIO `33`
- Back-left servo signal: GPIO `17`
- Back-right servo signal: GPIO `4`

Power:
- Servo VCC → external `5V` supply
- Servo GND → common ground with ESP32

> Important: tie servo ground and ESP32 ground together.

### 2) Ultrasonic Sensors
- Front sensor trig: GPIO `15`
- Front sensor echo: GPIO `14`
- Left sensor trig: GPIO `13`
- Left sensor echo: GPIO `12`
- Right sensor trig: GPIO `27`
- Right sensor echo: GPIO `35`

### 3) LED Face Module
- SDA → GPIO `21`
- SCL → GPIO `22`
- VCC → `5V` (or `3.3V` if the module supports it)
- GND → common ground

### 4) Indicator LEDs
- Status LED → GPIO `5`
- Attack LED → GPIO `18`

### 5) Optional Robot Fallback Joystick
- Joystick X → GPIO `34`
- Joystick Y → GPIO `39`
- Joystick button → GPIO `36`
- Joystick VCC → `5V`
- Joystick GND → common ground

### 6) Wireless Control
- Robot runs as WiFi AP: `BattleBotAP`
- Password: `battlebot123`
- UDP port: `4210`

---

## Controller ESP32

### 1) Analog Joystick
- Joystick X → GPIO `34`
- Joystick Y → GPIO `35`
- Joystick VCC → `5V`
- Joystick GND → GND

### 2) Controller Buttons
- Emoji button → GPIO `32`
- Dance button → GPIO `25`
- Other side of each button → GND

> Note: buttons use `INPUT_PULLUP`, so pressing the button pulls the pin LOW.

### 3) Wireless Link
- Controller connects to robot AP: `BattleBotAP`
- Password: `battlebot123`
- Sends UDP packets to robot IP: `192.168.4.1`
- Robot UDP port: `4210`
- Controller local UDP port: `4211`

---

## Power and Grounding

- Robot ESP32 power: `5V` or `Vin` supply
- Use a separate 5V supply for servos if the servo current draw is high.
- Connect all grounds: servos, LED matrix, ultrasonic sensors, and ESP32 must share a common ground.

- Controller ESP32 power: `5V` or USB power.

---

## Summary Table

| Device | Pin | Function |
|---|---|---|
| Robot ESP32 | 26 | Front-left servo signal |
| Robot ESP32 | 33 | Front-right servo signal |
| Robot ESP32 | 17 | Back-left servo signal |
| Robot ESP32 | 4 | Back-right servo signal |
| Robot ESP32 | 15 | Ultrasonic front trig |
| Robot ESP32 | 14 | Ultrasonic front echo |
| Robot ESP32 | 13 | Ultrasonic left trig |
| Robot ESP32 | 12 | Ultrasonic left echo |
| Robot ESP32 | 27 | Ultrasonic right trig |
| Robot ESP32 | 35 | Ultrasonic right echo |
| Robot ESP32 | 21 | LED matrix SDA |
| Robot ESP32 | 22 | LED matrix SCL |
| Robot ESP32 | 5 | Status LED |
| Robot ESP32 | 18 | Attack LED |
| Robot ESP32 | 34 | Optional joystick X |
| Robot ESP32 | 39 | Optional joystick Y |
| Robot ESP32 | 36 | Optional joystick button |
| Controller ESP32 | 34 | Joystick X |
| Controller ESP32 | 35 | Joystick Y |
| Controller ESP32 | 32 | Emoji button |
| Controller ESP32 | 25 | Dance button |

---

## Notes

- The robot and controller communicate wirelessly using UDP.
- The controller sends joystick and command packets to the robot.
- The robot can display a face, emoji, and perform a dance when commanded.
