# Solar-Synchronized Light Controller - Implementation Guide

## Overview

This project implements a solar-synchronized outdoor light controller for the PIC18F67K40 microcontroller. The system automatically tracks the sun, adjusts for Daylight Savings Time (DST), and implements energy-saving mode by turning off the light between 1am and 5am.

## Architecture

The system follows a **layered architecture** with clear separation of concerns:

```
┌─────────────────────────────────────┐
│   Application Layer (main.c)        │
│   - Super-loop                       │
│   - Module orchestration             │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   Logic Layer                        │
│   - app_control: Light rules         │
│   - solar_mgr: Solar tracking & DST  │
│   - rtc_soft: Software RTC           │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   Hardware Abstraction (bsp)         │
│   - GPIO, ADC, Timer, Interrupts     │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   Hardware (PIC18F67K40)             │
└─────────────────────────────────────┘
```

## File Structure

| File | Purpose |
|------|---------|
| `config.h` | System configuration, constants, and hardware mappings |
| `bsp.c/h` | Board Support Package - Hardware Abstraction Layer |
| `rtc_soft.c/h` | Software Real-Time Clock with critical sections |
| `solar_mgr.c/h` | Solar tracking, DST detection, drift correction |
| `app_control.c/h` | Application logic and control rules |
| `main.c` | Entry point and super-loop |

## Key Features

### 1. Test Mode
Enable rapid testing by compressing 24 hours into 24 seconds:
- Uncomment `#define TEST_MODE` in `config.h`
- 1 virtual hour = 1 real second
- Allows quick verification of day/night cycles and energy-saving logic

### 2. Solar Tracking
- Monitors LDR (Light Dependent Resistor) with hysteresis filtering
- Detects Dusk and Dawn transitions
- Calculates Solar Midnight as the midpoint between dusk and dawn
- Applies drift corrections to keep time synchronized with the sun

### 3. Automatic DST Adjustment
- Monitors day length (time between dusk and dawn)
- **Summer (DST Active)**: Day length > 14 hours → Target solar midnight = 1am
- **Winter (No DST)**: Day length < 10 hours → Target solar midnight = midnight
- No manual intervention required

### 4. Energy Saving Mode
- Light automatically turns OFF between 1am and 5am
- Saves energy during hours when few people are active
- Light remains OFF even if it's dark during this window

### 5. Binary Clock Display
- Displays current hour (0-23) in binary on LED array
- Uses 5 LEDs (RB0-RB4) for 5-bit representation
- Example: Hour 5 = `00101` (binary)

### 6. Critical Sections
- Implements proper atomic operations for 8-bit PIC18
- All 16-bit variable access protected with GIE disable/enable
- Prevents data corruption between ISR and main loop

## Hardware Connections

Based on `config.h` pin mappings:

| Function | Pin | Direction |
|----------|-----|-----------|
| Main Light Control | RD0 | Output (to relay/MOSFET) |
| Heartbeat LED | RD1 | Output |
| Binary Clock Display | RB0-RB4 | Output (5 LEDs) |
| LDR (Light Sensor) | RA0 (AN0) | Analog Input |

## How It Works

### Startup Sequence

1. **Power On**: `BSP_Init()` configures hardware
2. **Initial Calibration**: 
   - Read LDR value
   - If dark: assume midnight (00:00)
   - If bright: assume noon (12:00)
3. **First Cycle**: System runs freely, waiting for first dusk-to-dawn cycle
4. **First Sync**: After detecting complete night cycle, calculate drift and snap to solar time
5. **Continuous Operation**: Apply incremental corrections to maintain sync

### Main Loop Operation

```c
while(1) {
    // 1. Read sensors
    Time_t now = RTC_GetTime();
    uint16_t light_level = BSP_GetLDR();
    
    // 2. Update solar state and get drift correction
    int16_t drift = SOLAR_Update(light_level, now);
    if (drift != 0) {
        RTC_ApplySync(drift);  // Adjust clock
    }
    
    // 3. Apply control logic
    APP_Task(now, SOLAR_IsDark());  // Control light based on rules
    
    // 4. Update display
    BSP_SetClockDisplay(now.hours);  // Show hour in binary
}
```

### Light Control Logic

```
IF (is_dark) AND (hour NOT between 1-5):
    Light ON
ELSE:
    Light OFF
```

Truth table:

| Condition | Hour | Light State |
|-----------|------|-------------|
| Day | Any | OFF |
| Night | 0, 5-23 | ON |
| Night | 1, 2, 3, 4 | OFF (Energy Saving) |

## Testing Strategy

### Test Mode (24-Second Day)

1. **Enable Test Mode**:
   ```c
   // In config.h
   #define TEST_MODE
   ```

2. **Expected Behavior**:
   - Binary clock counts 0-23 in ~24 seconds
   - Cover LDR (simulate night):
     - Light ON when display shows 0, 5-23
     - Light OFF when display shows 1-4
   - Uncover LDR (simulate day):
     - Light OFF regardless of time

3. **Drift Test**:
   - Provide uneven day/night cycles
   - Example: Cover for 15 seconds, uncover for 9 seconds
   - System should automatically adjust to find new "solar midnight"

### Production Mode

1. **Disable Test Mode**:
   ```c
   // In config.h
   // #define TEST_MODE  // Commented out
   ```

2. **Initial Deployment**:
   - Install at dusk or dawn for faster calibration
   - First sync will occur after first complete night cycle (dusk → dawn)
   - Large initial correction is normal on first sync

3. **Long-Term Operation**:
   - System tracks solar midnight continuously
   - Automatically adjusts for seasonal changes (DST)
   - Incremental corrections keep time accurate

## Tuning Parameters

Adjust these in `config.h` based on your environment:

### LDR Thresholds
```c
#define LDR_THRESHOLD_DUSK  400   // Dark transition (lower = earlier turn-on)
#define LDR_THRESHOLD_DAWN  600   // Light transition (higher = later turn-off)
```

### Energy Saving Window
```c
#define ENERGY_SAVE_START_HOUR  1  // Start hour (1am)
#define ENERGY_SAVE_END_HOUR    5  // End hour (5am)
```

### DST Detection Thresholds
```c
#define DAY_LENGTH_SUMMER_MIN   14  // Hours (> 14 = summer)
#define DAY_LENGTH_WINTER_MAX   10  // Hours (< 10 = winter)
```

## Critical Implementation Notes

### 1. PIC18 8-Bit Constraints
- **No hardware divider**: Avoid `/` and `%` operators
- **Use bit-shifting**: `>> 1` for divide by 2, `>> 5` for divide by 32
- **16-bit atomicity**: All multi-byte reads/writes use critical sections

### 2. Timer Configuration
- Timer0 generates periodic interrupts
- **Test Mode**: ~277μs ticks (3600 per second)
- **Production**: 1.0s ticks (1 per second)
- ISR calls `RTC_TickISR()` which updates the software clock

### 3. ADC Configuration
- Uses ADCC (ADC with Computation) feature
- Hardware averages 32 samples automatically
- Reduces noise without CPU overhead

## Debugging Tips

### Heartbeat LED
The heartbeat LED toggles periodically in the main loop:
- Blinking = System is running
- Stuck = Main loop frozen (check for infinite loops)

### Binary Clock Display
Monitor the LED array to verify:
- Clock is incrementing
- Time appears reasonable
- Transitions occur at expected moments

### Serial Output (Optional)
Add UART debugging to monitor:
- Dusk/Dawn timestamps
- Calculated solar midnight
- Drift corrections applied

## Future Enhancements

1. **EEPROM Storage**: Save last known time to survive power cycles
2. **Temperature Compensation**: Adjust timer for temperature drift
3. **Manual Override**: Button to temporarily disable energy saving
4. **Fade In/Out**: PWM control for smooth light transitions
5. **Multiple Zones**: Control multiple lights with different schedules

## Compliance with Specification

| Requirement | Implementation |
|-------------|----------------|
| Monitor light level and control LED | ✅ BSP_GetLDR() + BSP_SetMainLight() |
| Display hour in binary | ✅ BSP_SetClockDisplay() |
| Turn off 1am-5am | ✅ app_control.c energy saving logic |
| Adjust for DST | ✅ solar_mgr.c day-length based detection |
| Maintain solar sync | ✅ solar_mgr.c drift correction |
| Fully automatic | ✅ No user input required |
| Testing mode | ✅ TEST_MODE in config.h |

## License

Educational project for ECM module coursework.

## Author

ECM Student Project - 2026
