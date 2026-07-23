# Home Assistant – Heltec Wireless Paper V1.2 E-Paper

> **Mission:** Das Referenzprojekt für Home-Assistant-E-Paper-Dashboards.

ESPHome external component for the Heltec Wireless Paper V1.2 with the E0213A367 2.13-inch monochrome e-paper panel.

## v0.3.0-rc1

This release candidate adds panel sleep, VEXT power-off and wake-up/reinitialization while retaining the stable v0.2 refresh implementation.

## Hardware pins

| Function | GPIO |
|---|---:|
| SPI CLK | 3 |
| SPI MOSI | 2 |
| CS | 4 |
| DC | 5 |
| RESET | 6 |
| BUSY | 7 |
| VEXT, active LOW | 45 |
| Status LED | 18 |

## Installation

Copy this repository structure into the ESPHome configuration directory. Keep `components/heltec_epaper/` next to the YAML or adjust the local component path.

Copy `secrets.example.yaml` to `secrets.yaml` and enter the Wi-Fi credentials.

Start with:

- `examples/01_dashboard.yaml` for continuous operation
- `examples/09_deep_sleep.yaml` for battery testing

## Display power methods

The methods can be used in an ESPHome lambda:

```yaml
- lambda: |-
    id(epaper).sleep();
```

A later display update wakes and reinitializes the panel automatically:

```yaml
- component.update: epaper
```

Manual wake-up is also available:

```yaml
- lambda: |-
    id(epaper).wake_up();
```

## Important test status

v0.3.0-rc1 is deliberately marked as a release candidate. Full and partial refresh are based on the proven v0.2 implementation. The new sleep/VEXT sequence must be verified on the physical Wireless Paper V1.2 before v0.3 is declared stable.

## Security

Never store real Wi-Fi credentials in committed YAML files. Use ESPHome secrets.

## License

MIT
