# Design notes

## Mission

Das Referenzprojekt für Home-Assistant-E-Paper-Dashboards.

## Responsibilities

The display driver handles:

- E0213A367 initialization
- full and partial refresh
- BUSY timeout handling
- panel deep-sleep command
- Wireless Paper V1.2 VEXT control
- clean reinitialization after wake-up

ESPHome handles:

- Wi-Fi and Home Assistant connectivity
- scheduling
- ESP32 deep sleep and wake-up sources
- dashboard rendering

## Power model

GPIO45 controls VEXT and is active LOW on Wireless Paper V1.2.

- `wake_up()` enables VEXT and fully initializes the panel.
- `sleep()` sends controller command `0x10`, waits briefly, then disables VEXT.
- A later `update()` automatically wakes and reinitializes the display.
- The e-paper image remains visible without power.

## Conservative release policy

v0.3.0-rc1 keeps the proven refresh sequences from v0.2 and adds power management around them. Battery mode uses a full refresh on every boot because controller RAM is not retained after VEXT is removed.
