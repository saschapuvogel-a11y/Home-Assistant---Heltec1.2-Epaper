# Power measurement protocol

Measure the complete board through USB at approximately 5 V. USB meters may miss short current peaks and may be inaccurate below a few milliamps.

| State | Power | Current | Notes |
|---|---:|---:|---|
| Startup peak | about 0.60 W | about 120 mA | First switch-on peak |
| Boot, rear LED blinking | about 0.40 W | about 80 mA | Network/startup phase |
| Wi-Fi connected, normal operation | 0.25–0.26 W | 51–58 mA | Measured baseline |
| Partial refresh peak | about 0.30 W | about 60 mA | Meter may smooth short peaks |
| Display sleep, ESP awake | TBD | TBD | Test with `id(epaper).sleep()` |
| ESP32 deep sleep | TBD | TBD | Meter may show 0.00 A below its resolution |

## Test sequence

1. Flash `examples/01_dashboard.yaml` and record normal operation and refresh values.
2. Call `id(epaper).sleep()` while leaving the ESP and Wi-Fi active; record the stable value.
3. Flash `examples/09_deep_sleep.yaml`; record the value after the device enters deep sleep.
4. Repeat each state three times and note the meter model and USB supply voltage.
