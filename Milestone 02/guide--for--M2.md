# Milestone 2 — Implementation Steps

## 1. Prepared the two ESP32 circuits

We used the same connections on both boards.

| Component | GPIO |
|---|---|
| DHT11 DATA | 26 |
| Button with external pull-down | 27 |
| Red LED | 32 |
| White LED | 33 |

We first checked local button control and DHT11 readings.

## 2. Connected the boards to Wi-Fi

We configured both ESP32s to use the same Wi-Fi network as the computer. A simple test printed each board’s IP address in Serial Monitor at 115200 baud.

## 3. Started the MQTT broker

We used Mosquitto on the computer. We checked the computer’s IPv4 address and used it in the Mosquitto listener configuration and both ESP32 sketches.

The devices communicated using **MQTT over TCP port 1883**.

During setup, Mosquitto initially failed because its configuration contained an old IP address. Updating the listener address resolved this.

## 4. Tested MQTT communication

We used the PubSubClient library to connect each ESP32 to Mosquitto.

Each board had a unique client ID:

```text
elech423-esp32-a
elech423-esp32-b
```

We confirmed the connections through Serial Monitor and Mosquitto’s connection logs.

## 5. Added remote LED control

Each board subscribed to its own command topic:

```text
elech423/esp32-a/white/set
elech423/esp32-b/white/set
```

An `ON` command set GPIO33 HIGH. An `OFF` command set it LOW.

After applying a command, the board published its output state on its `/white/state` topic. We tested this by sending commands from the computer.

## 6. Published sensor measurements

Both boards read the DHT11 approximately every two seconds using `millis()` scheduling.

They published temperature and humidity as JSON:

```json
{"temperature":24.5,"humidity":50.0}
```

Each device used a separate sensor topic:

```text
elech423/esp32-a/sensor
elech423/esp32-b/sensor
```

We checked that Mosquitto received publications from both devices.

## 7. Connected A’s button to B’s white LED

A continued to control its own red LED locally. We also made it publish `ON` or `OFF` to B’s command topic when its button state changed.

The message followed this path:

```text
A’s button → ESP32 A → Mosquitto → ESP32 B → B’s white LED
```

We tested both actions:

- Pressing A’s button turned on A’s red LED and B’s white LED.
- Releasing it turned both LEDs off.

## 8. Checked simultaneous operation

We kept both boards connected and watched their sensor publications in the broker log.

B initially experienced a connection timeout. After restarting it, both devices resumed publishing regularly, and the A-to-B LED test worked.

## 9. Added the live dashboard

We ran `dashboard.py` on the computer. It subscribed to the sensor topics, stored recent readings in memory and served `dashboard.html` at `http://localhost:8080`.

We verified that the dashboard displayed both devices and updated the temperature and humidity graphs. A appeared as a solid line and B as a dashed line.

## 10. Prepared the threat model and presentation

We matched the Data Flow Diagram to the working implementation and incorporated the team’s STRIDE clarifications.

For Milestone 3, we proposed:

- HMAC authentication for commands.
- Sequence counters for replay protection.
- AES-GCM encryption for sensor telemetry.

The verified Milestone 2 demonstration consists of MQTT communication, remote LED control and live sensor charts. The security controls remain proposed work.
