<a name="v1.0.1"></a>

## [v1.0.1] - 2025-02-15

### Fix

- **Make:** Correct installation command in Makefile

### Refactor

- **misc:** Implement power settings and UI improvements
- **power:** Remove hardcoded power paths, prepare removals

<a name="v1.0.0"></a>

## [v1.0.0] - 2025-02-12

### Chore

- **build:** Add uninstall targets in Makefile

### Feat

- **gui:** Implement lock screen timeout setting
- **gui:** Implement QOrderedMap and populate dropdowns
- **gui:** Enhance power settings window appearance
- **power:** Implement LogindManager and SwayIdleManager
- **power:** Apply GUI settings to power profiles
- **power-management:** Implement lid close and power key actions
- **power-profiles:** Implement power source detection and profile switching
- **power-profiles:** Implement power profiles support
- **power-settings:** Implement power key action and UI updates
- **settings:** Save and load power profile settings
- **swayidle:** Implement swayidle config parsing and integration

### Fix

- **build:** Correctly set C++ compiler and remove DBus dependency

### Fix

- **swayidlemanager:** Properly start and manage swayidle process

### Refactor

- **app:** Improve application loading and settings
- **power-management:** Improve power profile application
- **power_monitoring:** Replace D-Bus with file-based power source detection
- **powerprofile:** Simplify power profile management
- **powerprofile:** Refactor power profile handling

### Refactor

- **power:** Decouple power supply status check

<a name="v0.0.2"></a>

## [v0.0.2] - 2025-01-22

### Chore

- **release:** Update release workflow for x86-64 binary

<a name="v0.0.1"></a>

## v0.0.1 - 2025-01-22

[Unreleased]: https://github.com/midoBB/pwrctl/compare/v1.0.1...HEAD
[v1.0.1]: https://github.com/midoBB/pwrctl/compare/v1.0.0...v1.0.1
[v1.0.0]: https://github.com/midoBB/pwrctl/compare/v0.0.2...v1.0.0
[v0.0.2]: https://github.com/midoBB/pwrctl/compare/v0.0.1...v0.0.2
