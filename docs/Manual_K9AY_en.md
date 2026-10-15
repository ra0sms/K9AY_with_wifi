# K9AY receiving antenna controller with Wi-Fi interface

*Installation and setup guide*

## Specifications

| Parameter | Value |
| --- | --- |
| Supply voltage | 12–13.8 V DC |
| Current consumption | about 250 mA |
| Receiving directions | 8: N, NE, E, SE, S, SW, W, NW |
| Termination resistor (Rload) | 300, 390, 430, 470, 510, 560 Ohm |
| Built-in preamplifier | +15 dB, switched from the web page |
| Wi-Fi | 2.4 GHz, range up to 50 m with an entry-level router |
| Case | 170 × 120 × 55 mm, ABS plastic, IP68, cable glands for 3–7 mm cable |

## Connections

* Screw terminals **N S E W**: the two perpendicular loops, oriented North–South and East–West.
* Screw terminal **GROUND**: the radial (counterpoise) system.
* Power: 12–13.8 V DC from a supply rated at least 500 mA.

## Default Wi-Fi settings

| Parameter | Value |
| --- | --- |
| Controller network name | `K9AY_Loop_XXXXXX`, where `XXXXXX` is the unique controller number |
| Password | `1234567890` |
| Address on the controller network | `http://192.168.4.1` |
| Address on your home network | `http://k9ay.local` or the IP address assigned by your router |

By default the controller’s own access point stays on even after it joins your home network, so you can always reach it at `192.168.4.1`. The access point can be disabled (see “Wi-Fi settings”).

## First start and connecting to your home network

> The controller supports **2.4 GHz Wi-Fi only**. Your router must have a 2.4 GHz network and **DHCP** enabled.

1. Power on the controller and wait 5–10 seconds.
2. On your phone or computer, find the Wi-Fi network **K9AY_Loop_XXXXXX** and connect to it with the password **1234567890**.
3. Most devices open the controller page by themselves. If yours does not, open **http://192.168.4.1** in a browser.
4. Tap **WiFi Configuration**. The **Available Networks** section lists the networks found. Tap your network to copy its name into the **Network SSID** field.
5. Enter your network password in the **Password** field and press **Connect**.
6. After 10–20 seconds the **WIFI** LED lights up. The **WLAN Configuration** section shows the controller’s IP address on your network.
7. Disconnect from K9AY_Loop_XXXXXX and reconnect to your home network. The controller is now available at **http://k9ay.local** or at the IP address from step 6.

The `k9ay.local` address works on Windows 10/11, macOS, iOS and Linux. Some Android devices cannot open it, so use the IP address there. You can also find the IP address in your router’s list of DHCP clients. To keep the address from changing, reserve it for the controller in your router settings.

## Home page

The home page shows how you are connected, the controller uptime and the firmware version. It links to the other pages:

| Button | Address | Purpose |
| --- | --- | --- |
| Direction Control | `/switch` | antenna control |
| WiFi Configuration | `/wifi` | Wi-Fi connection settings |
| Direction Labels | `/labels` | names of the direction buttons |

## Antenna control (Direction Control)

* **Direction Control**: eight direction buttons arranged as a compass rose. The selected direction is highlighted in green and shown in the centre.
* **Rload Settings**: the loop termination resistor, 300–560 Ohm. Choose the value that gives the best rejection of signals from the back direction.
* **PREAMP ON / PREAMP OFF**: switches the +15 dB preamplifier.
* **TURN ALL OFF**: releases all relays, disconnects the termination resistor and switches off the preamplifier.

The status line above the buttons shows the current mode, e.g. `NE 470 Ohm +15dB`. The page refreshes itself every 2 seconds, so changes made from another device show up at once. If the connection to the controller is lost, the status line turns red.

The controller remembers the direction, resistor and preamplifier state. After a power loss it comes back in the same mode.

## Wi-Fi settings (WiFi Configuration)

* **SoftAP Configuration**: name, status (On/Off) and IP address of the controller’s access point.
* **WLAN Configuration**: the saved home network, connection status, IP address, signal level and connection time.
* **Available Networks**: networks found, sorted by signal level. **Scan** refreshes the list.
* **Connect to Network**: connects to another network. Network name up to 32 characters, password up to 64 characters.

To make the controller forget your home network, leave **Network SSID** empty and press **Connect**.

If the router cannot be reached (router off, wrong password), the controller tries again every 60 seconds. The K9AY_Loop_XXXXXX access point is active in the meantime.

**Disabling the access point.** Tick **Disable access point while connected to WLAN** and press **Save**. The access point turns off 30 seconds after the controller connects to your home network. From then on the controller is reachable only at `k9ay.local` or its IP address on your home network. If the saved network is unavailable for more than 15 seconds, the access point turns back on automatically. It is also on after power-on until the controller connects to your home network.

## Direction names (Direction Labels)

Each direction button can have its own name, such as an azimuth ("45°") or a region ("EU", "JA"). Names can be up to 23 bytes long; non-Latin characters take two bytes each. An empty field restores the default name. **Reset to defaults** restores all default names.

## Indicators

| WIFI LED | Meaning |
| --- | --- |
| On | connected to your home Wi-Fi network |
| Off | not connected to your home network (only the controller’s access point is active) |

## Control from software (HTTP API)

Scripts, logging software or home automation systems can control the controller with plain HTTP requests:

| Request | Action |
| --- | --- |
| `GET /api/state` | current state as JSON |
| `POST /set dir=NE` | direction: N, NE, E, SE, S, SW, W, NW |
| `POST /set r=470` | termination resistor: 300, 390, 430, 470, 510, 560 |
| `POST /set pa=1` | preamplifier: 1 = on, 0 = off |
| `POST /set off=1` | turn everything off |

Parameters can be combined in one request. Example: `curl -d "dir=NE&r=470&pa=1" http://k9ay.local/set`

## Troubleshooting

| Problem | Solution |
| --- | --- |
| K9AY_Loop_XXXXXX network is not visible | Check the power supply. Wait 10 seconds after power-on. Move closer to the controller. |
| The controller does not connect to the router | Check the password. Make sure the network is 2.4 GHz and DHCP is on. Enter the settings again on the WiFi Configuration page. |
| k9ay.local does not open | Use the IP address from the WiFi Configuration page or from the router’s DHCP client list. |
| The controller IP address changes | Reserve an IP address for the controller in the router’s DHCP settings. |
| No access through the home network | Connect to the K9AY_Loop_XXXXXX network and open http://192.168.4.1. If the access point is disabled, power-cycle the controller: after power-on the access point stays on until the controller connects to your home network, and for 30 more seconds. |
