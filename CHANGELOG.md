# Changelog

## 0.3.0-rc1

- Added public `sleep()` and `wake_up()` methods.
- Added E0213A367 deep-sleep command followed by VEXT shutdown.
- Added automatic panel wake-up and full reinitialization before an update.
- Added explicit power and sleep state tracking.
- Removed duplicate VEXT activation during setup.
- Updated driver version logging.
- Added dashboard and deep-sleep examples.
- Added power-measurement protocol and design documentation.
- Sanitized Wi-Fi credentials and introduced `secrets.example.yaml`.

## 0.2.x

- Stable full and partial refresh.
- Stable BUSY handling.
- Differential frame reference update without visible second refresh.
