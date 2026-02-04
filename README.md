# Solar-Synchronized Light Controller

An energy-saving automatic outside light for the PIC18F67K40 microcontroller. The device monitors ambient light with an LDR and turns a main LED on at dusk and off at dawn, while turning it off during the small hours (1am–5am) to save energy when few people are around—addressing the brief’s requirement for councils-style street lighting. It stays synchronized with the sun indefinitely by detecting dusk and dawn, computing solar midnight as the midpoint of night, and applying drift correction so the internal clock never drifts out of sync with real time. Daylight savings is inferred from day length, and a testing mode compresses 24 hours into 24 seconds for rapid development.

---

## Key Functions

| Module | Function | Description |
|--------|----------|-------------|
| **ADC** | `ADC_Init()` | Initializes the ADC in burst-average mode (32 samples) for LDR on RA3. |
| **ADC** | `ADC_ReadLDR()` | Returns LDR value 0–1023 (0 = dark, 1023 = bright). |
| **Timer** | `Timer_Init()` | Starts Timer0 with periodic interrupts for system ticks. |
| **Timer** | `Timer_GetTicks()` | Returns current tick count; used with `TICKS_PER_SECOND` for timekeeping. |
| **Timer** | `Timer_ResetTicks()` | Resets tick counter (optional recalibration). |
| **LEDs** | `LEDs_Init()` | Sets up 9-LED bus and initial pin states. |
| **LEDs** | `LEDs_SetMainLight(bool)` | Turns main streetlight (LED 9) on or off. |
| **LEDs** | `LEDs_SetClockDisplay(uint8_t hour)` | Shows hour 0–23 in 5-bit binary on LEDs 1–5. |
| **LEDs** | `LEDs_ToggleHeartbeat()` | Toggles heartbeat LED (LED 8) for status indication. |
| **main** | `SolarUpdate()` | State machine: dusk/dawn detection, drift correction. |
| **main** | `HandleDuskTransition()` | Records dusk time when transitioning day → night. |
| **main** | `HandleDawnTransition()` | Records dawn time, computes solar midnight, returns drift correction. |
| **main** | `CalculateDriftCorrection()` | Compares observed solar midnight to target and returns adjustment in minutes. |
| **main** | `UpdateSeasonFromDayLength()` | Uses day length to set target solar midnight (DST vs winter). |
| **main** | `ApplySync()` | Applies drift correction to internal clock. |
| **main** | `IsInEnergySaveWindow()` | True if current hour is between 1am and 5am. |

---

## User-Tweakable Constants

All user-facing constants are in **`config.h`**.

### Test mode

| Constant | Effect |
|----------|--------|
| `TEST_MODE` | Uncomment to enable: 1 day = 24 seconds (1 hour = 1 second). Comment out for normal 24-hour operation. |

### LDR thresholds

| Constant | Typical Range | Effect |
|----------|---------------|--------|
| `LDR_THRESHOLD_DUSK` | ~400 | Below this = transition to dark (dusk). |
| `LDR_THRESHOLD_DAWN` | ~600 | Above this = transition to light (dawn). Hysteresis avoids flicker. |

### Energy-saving window

| Constant | Default | Effect |
|----------|---------|--------|
| `ENERGY_SAVE_START_HOUR` | 1 | Hour (0–23) when light turns off. |
| `ENERGY_SAVE_END_HOUR` | 5 | Hour when light turns back on. |

### Solar / DST

| Constant | Default | Effect |
|----------|---------|--------|
| `SOLAR_MIDNIGHT_WINTER` | 0 | Target solar midnight (hour) in winter. |
| `SOLAR_MIDNIGHT_SUMMER` | 1 | Target solar midnight in summer (DST). |
| `DAY_LENGTH_SUMMER_MIN` | 14 | Day length (hours) above which summer/DST is assumed. |
| `DAY_LENGTH_WINTER_MAX` | 10 | Day length below which winter is assumed. |

### Timer (advanced)

| Constant | Effect |
|----------|--------|
| `TMR0_RELOAD_HIGH`, `TMR0_RELOAD_LOW` | Timer0 reload for 1 s tick (production) or short period (test). |
| `TICKS_PER_SECOND` | 1 in production; 3600 in test mode. |

### ADC hardware (in `adc.h`)

| Constant | Default | Effect |
|----------|---------|--------|
| `ADC_LDR_CHANNEL` | 3 | ADC channel for LDR (RA3/ANA3 on PIC18F67K40). |

---

## Build and Run

```bash
make
```

(Flash the generated hex to the PIC18F67K40 using your programmer.)

---

## Hardware

- **Target:** PIC18F67K40 @ 64 MHz
- **LDR:** RA3 (ANA3)
- **LEDs:** 9-LED bus across ports G, A, F, B (LEDs 1–5 = binary clock; 8 = heartbeat; 9 = main light)
