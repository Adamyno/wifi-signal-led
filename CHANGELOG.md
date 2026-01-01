# Changelog

All notable changes to this project will be documented in this file.

## [V0.1.2] - 2026-01-01
### Added
- Multi-tab Dashboard for connected mode: Status, Settings, About.
- Version constants added to `main.cpp`.
- `CHANGELOG.md` for project history tracking.

### Fixed
- Web interface button alignment: Restart and Reset buttons are now constrained to the same width as the info cards.
- Screen clearing logic in AP mode to preserve the status bar.

### Changed
- LED is now completely turned off when connected to WiFi (no more PWM signal).
- Version bumped from V0.1.1 to V0.1.2.

## [V0.1.1] - 2026-01-01
### Added
- Over-The-Air (OTA) update support.
- Static IP configuration for uploads in `platformio.ini`.

## [V0.1.0] - 2026-01-01 (Initial Release with Display)
### Added
- Integration of TJCTM24028-SPI 2.8" display.
- Dynamic status bar with WiFi signal icons and compact IP address.
- Code refactoring: modularized into `web_pages.h`, `display_utils.h/cpp`.
- Clean boot experience (no text on main area during boot).
