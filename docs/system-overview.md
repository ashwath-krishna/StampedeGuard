# System overview

## Objective
Explore a low-cost, camera-free way to detect passage events at a monitored entry lane and display the resulting count on a local dashboard.

## Architecture

```text
VL53L1X distance sensor
        |
        | I²C (SDA GPIO 21, SCL GPIO 22)
        v
     ESP32
        |
        +--> Baseline calibration
        |
        +--> Moving-average filtering
        |
        +--> Threshold-based passage event
        |
        +--> Cumulative entry-event counter
        |
        +--> Wi-Fi access point + HTTP server
                    |
                    v
              Browser dashboard
```

## Firmware sequence
1. Start serial communication and I²C.
2. Initialize the VL53L1X in long-distance mode.
3. Set a 50 ms measurement timing budget and continuous measurement interval.
4. Collect up to 40 readings during empty-lane calibration; use the median of valid readings if at least 30 are available.
5. Read distance at approximately 100 ms intervals.
6. Apply a three-sample moving average.
7. Trigger an event when the filtered distance drops below the calibrated baseline by the configured detection threshold and the cooldown has elapsed.
8. Rearm the detector when the lane clears beyond the release threshold.
9. Serve dashboard data through the ESP32 web server.

## Current configurable parameters
- `DETECT_DROP`: distance decrease required to trigger a passage event (150 mm in the supplied code).
- `RELEASE_DROP`: distance condition used to rearm the detector (80 mm in the supplied code).
- `standingArea`: user-entered area used in the dashboard's simple calculated indicator.
- `cautionLevel` and `dangerLevel`: thresholds used to choose the displayed status label.

These are prototype parameters and require empirical calibration for any meaningful evaluation.
