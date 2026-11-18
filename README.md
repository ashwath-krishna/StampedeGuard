# StampedeGuard
### ESP32-based entry-event monitoring prototype for crowded venues

StampedeGuard is an embedded-systems prototype built around an **ESP32** and a **VL53L1X time-of-flight distance sensor**. It detects changes in distance at a monitored entry lane, records passage events, and serves a small live dashboard over the ESP32's Wi-Fi access point.

The broader idea is to explore how low-cost, camera-free sensing could support crowd-management teams at large public gatherings.

> **Project status:** Working prototype code and dashboard. The current hardware setup is a single-point entry-event detector, not a complete venue-wide crowd-counting or crowd-safety system. See [limitations](docs/safety-and-limitations.md).

---

## Concept in a real-world setting

The image below is a **concept illustration** of a possible StampedeGuard deployment at a large public rally. It is intended to communicate the use case and dashboard concept; it is not a photograph of a field deployment and its displayed figures are illustrative only.

![StampedeGuard concept at a large public rally](assets/stampedeguard-rally-concept.png)

The same general problem can arise at:
- Political rallies and public speeches
- Temple festivals and large religious gatherings
- Concerts and cultural events
- University festivals and campus events
- Public celebrations, fairs, and queues
- Transport hubs and other high-footfall public spaces

These are potential application contexts, not validated deployments.

## What the current prototype does

1. The ESP32 initializes the VL53L1X sensor over I²C.
2. At startup, it samples the sensor while the monitored lane is clear and calculates a median baseline.
3. It reads distance measurements periodically and applies a three-reading moving average.
4. A sufficiently large drop from the baseline triggers a passage event, subject to a short cooldown.
5. The ESP32 hosts a Wi-Fi access point and serves a browser dashboard.
6. The dashboard displays the cumulative detected entry-event count, a calculated entry-count-per-area indicator, sensor distance, sensor health, and a configurable status label.

## Hardware and software

| Component | Role |
|---|---|
| ESP32 development board | Main controller and Wi-Fi access point |
| VL53L1X time-of-flight sensor | Measures distance changes at one monitored lane |
| Arduino IDE | Firmware development and upload |
| Pololu VL53L1X Arduino library | Sensor interface |
| Browser | Displays the ESP32-hosted dashboard |

### Wiring used in the firmware

| VL53L1X signal | ESP32 pin |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| GND | GND |
| VIN | Follow the voltage requirements of your specific sensor breakout board |

Check the documentation for your exact VL53L1X breakout before connecting its power pin. Do not assume every breakout has the same voltage regulation or logic-level support.

## Repository structure

```text
StampedeGuard/
├── README.md
├── LICENSE
├── .gitignore
├── firmware/
│   └── StampedeGuard.ino
├── assets/
│   └── stampedeguard-rally-concept.png
└── docs/
    ├── system-overview.md
    ├── safety-and-limitations.md
    └── testing-plan.md
```

## Getting started

1. Install the Arduino IDE and the ESP32 board package.
2. Install the **VL53L1X** library used by the firmware (the code uses the `VL53L1X` API provided by the Pololu library).
3. Connect the sensor to the ESP32 using the I²C pins listed above.
4. Open `firmware/StampedeGuard.ino`.
5. Before uploading, replace `CHANGE_THIS_TO_A_PRIVATE_PASSWORD` with a private Wi-Fi access-point password. **Do not commit your real password to a public repository.**
6. Select the correct ESP32 board and serial port, then upload.
7. Open Serial Monitor at **115200 baud**. The firmware prints the access-point name and dashboard IP address.
8. Connect a phone or laptop to the `StampedeGuard` Wi-Fi network and open the printed IP address in a browser.
9. Keep the sensor's field of view clear during startup calibration.

## Dashboard/API endpoints

| Endpoint | Purpose |
|---|---|
| `/` | Serves the dashboard |
| `/api` | Returns the current count, distance, baseline, calculated indicator, area, sensor health, and status |
| `/config` | Accepts venue area and status-threshold settings |
| `/reset` | Resets the demonstration counter |

## Important interpretation of the displayed numbers

The concept image includes **6,054 people** and **743 capacity remaining** to illustrate a large-event dashboard scenario. These are **mockup values**, not measurements produced by the current firmware.

The current firmware does not calculate venue-wide occupancy or remaining capacity. It counts detected entry events at one sensor location; it does not count exits or reliably separate people who pass together. Its `density` value is calculated from cumulative entry events divided by a user-entered area, so it must not be interpreted as actual people-per-square-metre crowd density. The status label is therefore a prototype display indicator, not a safety determination.

## Current development status

- [x] ESP32 firmware structure
- [x] VL53L1X initialization and empty-lane baseline calibration
- [x] Distance filtering and threshold-based passage-event detection
- [x] ESP32 Wi-Fi access point and web server
- [x] Browser dashboard and configuration endpoints
- [ ] Bidirectional entry/exit counting
- [ ] Multi-person passage validation
- [ ] Independent occupancy ground-truth comparison
- [ ] Venue-scale area mapping and calibrated density estimation
- [ ] Field testing and safety validation

## Next research and engineering steps

The next stage is to test the entry detector under controlled conditions, document missed and duplicate detections, and compare the sensor count with a manually recorded reference count. A later version could investigate multiple sensing points, direction detection, and a better method for estimating occupancy. Those features should be treated as future work until implemented and evaluated.

## Safety note

StampedeGuard is an educational engineering prototype. It is **not certified, validated, or suitable for managing real crowd safety**. Do not use its status label, count, or concept-image values to make real-world entry, evacuation, or capacity decisions. See [Safety and limitations](docs/safety-and-limitations.md).
