# Changelog

All notable changes to this project will be documented in this file.

## [0.2.9] - 2026-01-01
### Changed
- **Web OTA Test**: Bumped version to verify manual firmware upload functionality via the web dashboard.

## [0.2.8] - 2026-01-01

### Changed
- **UI Layout**: Changed cards to `display: block` with centered margins to ensure they always stack vertically, improving consistency on wider screens.

## [1.2.7] - 2026-01-01

### Added
- **Web Firmware Update**: Added a new "Firmware Update" card to the Settings tab, allowing users to upload and flash new `.bin` firmware files directly from the browser.

## [0.2.6] - 2026-01-01

### Changed
- **UI Polish**: Applied `box-sizing: border-box` globally and adjusted input field widths to 100% to fix alignment issues in the "Transmission Config" section.

## [0.2.5] - 2026-01-01

### Added
- **Extended Test Metrics**: The Transmission connection test now retrieves and displays both **Download** and **Upload** speeds from the server as definitive proof of a successful RPC connection.

## [0.2.4] - 2026-01-01

### Fixed
- **Test Result Display**: The web interface now correctly displays the full status message from the server (including download speed) instead of a generic "Connection OK!" text.

## [0.2.3] - 2026-01-01

### Added
- **Deep Transmission Integration**: Implemented full RPC handshake including Basic Auth and CSRF Session ID handling.
- **Speed Monitoring**: The connection test now returns the current download speed (`session-stats`) as proof of valid connection.

## [0.2.2] - 2026-01-01

### Changed
- **UI Refinement**: Replaced browser alerts with an in-page status message for the Transmission test. This solves visibility issues on desktop browsers and provides cleaner feedback.

## [0.2.1] - 2026-01-01

### Fixed
- **Transmission Test Button**: Fixed a logical disconnect where the test only used previously saved settings. It now uses the current values in the input fields.
- **UI Feedback**: Added a "Testing..." state and disabled the Test button during network activity to provide visual feedback.

## [0.2.0] - 2026-01-01

### Added
- **Transmission Integration**: Added configuration fields (Host, Port, Path, User, Pass) to the Settings tab.
- **Persistent Storage**: All Transmission settings are now saved to `config.json` alongside WiFi credentials.
- **Connection Test**: Added functionality to test connection to the Transmission server directly from the web interface.

### Changed
- Version bumped to 0.2.0.
- `saveConfig` refactored to handle multiple setting types without data loss.

## [0.1.6] - 2026-01-01

### Fixed
- REMOVED the extra closing `</div>` tag in `dashboard_html` that caused buttons to leak outside the Status tab container. This definitely fixes the duplicate button issue.

### Changed
- Version bumped to 0.1.6.

## [0.1.5] - 2026-01-01

### Fixed
- Fixed visual bug where buttons appeared at the top of Settings and About tabs due to malformed HTML in the Status tab.

### Changed
- Version bumped to 0.1.5.

## [0.1.4] - 2026-01-01

### Added
- Restart and Reset buttons added to the Settings tab.
- Placeholder action for the Reset button in the Settings tab.

### Changed
- Version bumped to 0.1.4.

## [0.1.3] - 2026-01-01

### Changed
- Removed 'V' prefix from version number (e.g., "V0.1.2" -> "0.1.3").
- Version bumped to 0.1.3.

## [0.1.2] - 2026-01-01

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
