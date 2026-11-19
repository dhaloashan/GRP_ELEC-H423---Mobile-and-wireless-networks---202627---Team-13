# Milestone 2 — Running the Complete Project

Run PowerShell commands from the root of the cloned Team 13 repository. Paths below match its folders. Replace network placeholders only in your local working copies.


## 1. Install the required software

Install Arduino IDE, the ESP32 board package, Python 3 and Mosquitto.

In Arduino IDE, open Library Manager and install:

- **PubSubClient** by Nick O’Leary
- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit

Connect both ESP32 boards and the computer to the same Wi-Fi network.

## 2. Check the computer’s IP address

Open PowerShell and run:

```powershell
ipconfig
```

Find **IPv4 Address** under the active Wi-Fi adapter.

The commands below use this placeholder:

```text
YOUR_COMPUTER_IP
```

Replace `YOUR_COMPUTER_IP` with your computer’s current Wi-Fi IPv4 address in both ESP32 sketches, the local Mosquitto configuration and the terminal commands below.

## 3. Open and configure ESP32 A’s code

Use this repository sketch. Copy it into a local folder named `ESP32_A` and open `ESP32_A.ino` there in Arduino IDE:

```text
Milestone 02\Code\ESP32_A.ino
```

Change these three lines to match your network:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_COMPUTER_IP";
```

Check that A uses these topics:

```cpp
const char* COMMAND_TOPIC = "elech423/esp32-a/white/set";
const char* STATE_TOPIC = "elech423/esp32-a/white/state";
const char* SENSOR_TOPIC = "elech423/esp32-a/sensor";
```

Its MQTT connection must use A’s unique client ID:

```cpp
mqtt.connect("elech423-esp32-a")
```

A’s button-control code must publish to B’s command topic:

```cpp
mqtt.publish(
  "elech423/esp32-b/white/set",
  pressed ? "ON" : "OFF"
);
```

Select **ESP32 Dev Module** and A’s COM port, then click **Upload**.

## 4. Open and configure ESP32 B’s code

Copy this sketch into a local folder named `ESP32_B` and open `ESP32_B.ino` there:

```text
Milestone 02\Code\ESP32_B.ino
```

Set the same Wi-Fi network and computer address:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_COMPUTER_IP";
```

Check B’s topics:

```cpp
const char* COMMAND_TOPIC = "elech423/esp32-b/white/set";
const char* STATE_TOPIC = "elech423/esp32-b/white/state";
const char* SENSOR_TOPIC = "elech423/esp32-b/sensor";
```

Its MQTT client ID must be:

```cpp
mqtt.connect("elech423-esp32-b")
```

Select B’s COM port and click **Upload**.

Each board has its own COM port. Both can remain connected to the same computer.

## 5. Check the Mosquitto configuration

Create the local configuration from the example included in the repository:

```powershell
Copy-Item ".\Milestone 02\mosquitto-lab.conf.example" ".\Milestone 02\mosquitto-lab.conf"
```

Open the new file in a text editor:

```text
.\Milestone 02\mosquitto-lab.conf
```

It should contain:

```text
listener 1883 YOUR_COMPUTER_IP
allow_anonymous true
persistence false
```

| Setting | Purpose |
|---|---|
| `listener 1883 YOUR_COMPUTER_IP` | Accept MQTT connections on TCP port 1883 at the computer’s address. |
| `allow_anonymous true` | Allow clients to connect without a username or password. |
| `persistence false` | Keep broker state in memory without saving it across restarts. |

Save the file after checking the address.

## 6. Start Mosquitto — Terminal 1

Open PowerShell and run:

```powershell
& "C:\Program Files\mosquitto\mosquitto.exe" -c ".\Milestone 02\mosquitto-lab.conf" -v
```

This command opens **`mosquitto.exe`**, using **`mosquitto-lab.conf`**.

- `&` runs the quoted program path in PowerShell.
- `-c` selects the configuration file.
- `-v` displays detailed connection and message logs.

Wait for:

```text
mosquitto version 2.1.2 running
```

Keep Terminal 1 open.

## 7. Confirm both boards are connected

Open each board’s Serial Monitor at **115200 baud**.

Press **EN/Reset** on B and then on A if necessary. Each board should print:

```text
MQTT connected!
```

Sensor readings should follow:

```text
{"temperature":24.5,"humidity":50.0}
```

Values depend on the environment. The Mosquitto log should show publications from both `elech423-esp32-a` and `elech423-esp32-b`.

## 8. Monitor MQTT messages — Terminal 2

Open another PowerShell window and run:

```powershell
& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h YOUR_COMPUTER_IP -t "elech423/#" -v
```

This opens **`mosquitto_sub.exe`**, the MQTT subscriber client.

- `-h` selects the broker address.
- `-t` selects the topic filter.
- `elech423/#` receives all topics below `elech423/`.
- `-v` prints the topic and payload.

Expected messages include:

```text
elech423/esp32-a/sensor {"temperature":26.1,"humidity":51.0}
elech423/esp32-b/sensor {"temperature":25.8,"humidity":63.0}
```

Keep Terminal 2 open.

## 9. Test A’s button controlling B

Press and hold A’s button.

- A’s red LED should turn on.
- B’s white LED should turn on.

Release the button. Both should turn off.

Terminal 2 should show:

```text
elech423/esp32-b/white/set ON
elech423/esp32-b/white/state ON
elech423/esp32-b/white/set OFF
elech423/esp32-b/white/state OFF
```

A publishes the command, Mosquitto forwards it, and B applies it. B then reports the output state.

## 10. Start the dashboard — Terminal 3

Open another PowerShell window and run:

```powershell
python ".\Milestone 02\dashboard\dashboard.py" --broker YOUR_COMPUTER_IP
```

This runs **`dashboard.py`**. It receives measurements through MQTT and serves the dashboard.

The same folder must contain **`dashboard.html`**, which provides the page layout and graphs.

Open:

```text
http://localhost:8080
```

Check that both ESP32 devices show recent readings. The temperature and humidity graphs should update approximately every two seconds as new measurements arrive.

Keep Terminal 3 open.

## 11. Control an LED from the computer — Terminal 4

Open another PowerShell window.

To turn **A’s white LED on**, run:

```powershell
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h YOUR_COMPUTER_IP -t "elech423/esp32-a/white/set" -m "ON"
```

To turn it **off**, run:

```powershell
& "C:\Program Files\mosquitto\mosquitto_pub.exe" -h YOUR_COMPUTER_IP -t "elech423/esp32-a/white/set" -m "OFF"
```

These commands open **`mosquitto_pub.exe`**, the MQTT publishing client.

- `-t` selects A’s command topic.
- `-m` supplies the message.
- Write `ON` and `OFF` in uppercase.

To control B’s white LED, replace `esp32-a` with `esp32-b` in the topic. A’s next button-state command may change B’s LED again.

## 12. Final demonstration check

| Test | Expected result |
|---|---|
| A and B connect | Both print `MQTT connected!`. |
| A’s button is pressed | A’s red LED and B’s white LED turn on. |
| A’s button is released | Both turn off. |
| Terminal publishes ON/OFF to A | A’s white LED follows the command. |
| MQTT monitor runs | Sensor, command and state messages appear. |
| Dashboard runs | Both devices’ values and graphs update. |

During the presentation, keep the broker, subscriber and dashboard terminals open. Demonstrate the button control and graphs beside their relevant slides.

To finish, press **Ctrl+C** in each running terminal and disconnect the boards.
## Privacy and implementation status

Keep real Wi-Fi credentials and the completed local configuration out of Git. Store configured sketches in your local Arduino folders. Review screenshots before sharing so that passwords and personal file paths remain private.

The current demonstration uses plaintext MQTT with anonymous broker access. HMAC command authentication, replay counters and AES-GCM telemetry encryption remain proposed Milestone 3 controls.
