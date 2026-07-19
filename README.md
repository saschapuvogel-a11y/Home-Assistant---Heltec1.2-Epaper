# ESPHome Heltec Wireless Paper V1.2 — 0.2.0-rc1

ESPHome external component for the Heltec Wireless Paper V1.2 and its
E0213A367-BW monochrome panel.

## Confirmed hardware

The full-refresh 0.1 driver was confirmed on two separate Wireless Paper V1.2
boards. The panels were not defective; the decisive correction was using the
E-Paper SPI pins GPIO3/GPIO2 instead of the SX1262 LoRa SPI pins GPIO9/GPIO10.

| Signal | GPIO |
|---|---:|
| E-paper MOSI | 2 |
| E-paper CLK | 3 |
| E-paper CS | 4 |
| E-paper DC | 5 |
| E-paper RESET | 6 |
| E-paper BUSY | 7 |
| VEXT power | 45, active LOW |
| Status LED | 18 |

## 0.2 goal

0.2 adds experimental differential/partial refresh while retaining periodic
full refreshes to control ghosting.

```yaml
full_update_every: 10
```

The first refresh is full. The following nine are partial. Set the value to `1`
to use only the already proven full-refresh path.

## Installation

Copy `components/heltec_epaper` into `/config/esphome/components/`, keeping the
subfolder intact. Use the complete YAML under `examples/`.

## Test order

1. Compile and flash with `full_update_every: 1`.
2. Confirm the known-good full refresh still works.
3. Change to `full_update_every: 3` for a short partial-refresh test.
4. Watch the log for `Performing partial E0213A367 update`.
5. Check for correct changes, reduced flashing, and acceptable ghosting.

## Status

- Full refresh: confirmed on two boards
- Rotation and ESPHome drawing API: working in the confirmed configuration
- Partial refresh: experimental in 0.2.0-rc1
- Deep-sleep power preparation: not yet included
