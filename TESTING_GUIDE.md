# Testing Guide - Solar-Synchronized Light Controller

## Quick Start Testing

### Phase 1: Compilation Test

1. **Open project in MPLAB X IDE**
2. **Set compiler**: XC8 (C99 standard)
3. **Build project**: Verify no compilation errors
4. **Expected warnings**: None if properly configured

### Phase 2: Test Mode Verification (24-Second Day)

#### Setup
1. Enable test mode in `config.h`:
   ```c
   #define TEST_MODE  // Uncomment this line
   ```
2. Rebuild project
3. Program PIC18F67K40

#### Test Sequence

##### Test 2.1: Basic Clock Function
**Objective**: Verify the RTC is incrementing correctly

1. Power on the device
2. Observe binary clock display (5 LEDs on PORTB)
3. **Expected**: LEDs count from 0 to 23 in ~24 seconds

| Second | Binary Display | Hour |
|--------|----------------|------|
| 0 | 00000 | 0 |
| 1 | 00001 | 1 |
| 2 | 00010 | 2 |
| 3 | 00011 | 3 |
| 4 | 00100 | 4 |
| 5 | 00101 | 5 |
| ... | ... | ... |
| 23 | 10111 | 23 |

**Pass Criteria**: Clock cycles through 0-23 and repeats

---

##### Test 2.2: Day/Night Detection
**Objective**: Verify LDR reading and state transitions

1. Start with LDR uncovered (bright light)
2. **Expected**: Main light OFF
3. Cover LDR with hand (simulate night)
4. **Expected**: Main light turns ON within 1-2 loop iterations
5. Uncover LDR
6. **Expected**: Main light turns OFF

**Pass Criteria**: Light responds to LDR changes with hysteresis

---

##### Test 2.3: Energy Saving Window
**Objective**: Verify 1am-5am light-off logic

**Setup**: Cover LDR to simulate continuous darkness

| Hour (Binary) | Expected Light State | Test Result |
|---------------|---------------------|-------------|
| 0 (00000) | ON | |
| 1 (00001) | OFF | |
| 2 (00010) | OFF | |
| 3 (00011) | OFF | |
| 4 (00100) | OFF | |
| 5 (00101) | ON | |
| 6 (00110) | ON | |
| ... | ON | |
| 23 (10111) | ON | |

**Pass Criteria**: 
- Light ON for hours 0, 5-23 when dark
- Light OFF for hours 1-4 even when dark

---

##### Test 2.4: Initial Calibration
**Objective**: Verify startup time estimation

**Test 4a - Dark Start**:
1. Power off device
2. Cover LDR completely
3. Power on
4. **Expected**: Clock starts at hour 0 (midnight)

**Test 4b - Bright Start**:
1. Power off device
2. Ensure LDR is in bright light
3. Power on
4. **Expected**: Clock starts at hour 12 (noon)

**Pass Criteria**: Correct initial time estimate based on light level

---

##### Test 2.5: Solar Midnight Calculation
**Objective**: Verify drift correction logic

**Procedure**:
1. Power on with LDR uncovered (starts at hour 12)
2. Wait for hour display to reach 18 (6pm simulated)
3. Cover LDR (simulate dusk at 6pm)
4. Wait for hour display to reach 6 (6am simulated) - 12 hours later
5. Uncover LDR (simulate dawn at 6am)
6. **Expected**: System calculates:
   - Night duration = 12 hours
   - Solar midnight = 18 + 6 = 24 = 0 (midnight)
   - Target midnight = 0 (winter) or 1 (summer)
   - Drift correction applied

**Observation**: Check if clock makes a sudden adjustment after dawn

**Pass Criteria**: System detects full cycle and applies correction

---

##### Test 2.6: Uneven Day/Night Cycles
**Objective**: Verify adaptive drift correction

**Procedure**:
1. Create artificial cycle:
   - Cover LDR at hour 20 (8pm)
   - Uncover at hour 4 (4am) - only 8 hours night
2. System should calculate:
   - Night duration = 8 hours
   - Solar midnight = 20 + 4 = 24 = 0
   - Expect drift correction

**Pass Criteria**: System adapts to non-standard day/night patterns

---

##### Test 2.7: DST Detection (Long Days)
**Objective**: Verify summer DST detection

**Procedure**:
1. Create long day pattern:
   - Cover LDR at hour 22 (10pm simulated)
   - Uncover at hour 6 (6am simulated) - 8 hour night, 16 hour day
2. Day duration = 24 - 8 = 16 hours (> 14 hour threshold)
3. **Expected**: System sets target solar midnight to 1am (DST active)

**Pass Criteria**: After this cycle, drift correction targets 1am instead of midnight

---

##### Test 2.8: DST Detection (Short Days)
**Objective**: Verify winter standard time detection

**Procedure**:
1. Create short day pattern:
   - Cover LDR at hour 16 (4pm simulated)
   - Uncover at hour 8 (8am simulated) - 16 hour night, 8 hour day
2. Day duration = 24 - 16 = 8 hours (< 10 hour threshold)
3. **Expected**: System sets target solar midnight to midnight (no DST)

**Pass Criteria**: System switches to standard time target

---

##### Test 2.9: Heartbeat LED
**Objective**: Verify system is running

1. Power on device
2. Observe heartbeat LED (RD1)
3. **Expected**: LED blinks periodically

**Pass Criteria**: Visible blinking indicates main loop is executing

---

### Phase 3: Production Mode Testing

#### Setup
1. Disable test mode in `config.h`:
   ```c
   //#define TEST_MODE  // Comment out this line
   ```
2. Rebuild project
3. Program PIC18F67K40

#### Test Sequence

##### Test 3.1: Real-Time Clock
**Objective**: Verify 1-second ticks in production mode

**Equipment**: Oscilloscope or logic analyzer

1. Program device
2. Monitor heartbeat LED or use breakpoint in RTC_TickISR()
3. **Expected**: ISR called every 1.0 second (±50ms)

**Pass Criteria**: Timer maintains accurate 1Hz rate

---

##### Test 3.2: 24-Hour Cycle
**Objective**: Verify full day operation

**Setup**: Install in actual deployment environment

1. Program device at dusk (recommended)
2. Monitor for 48 hours
3. Log:
   - Dusk transition time (day 1)
   - Dawn transition time (day 1)
   - Dusk transition time (day 2)
   - Dawn transition time (day 2)

**Expected Behavior**:
- Light turns ON at dusk
- Light turns OFF at 1am (binary: 00001)
- Light turns back ON at 5am (binary: 00101)
- Light turns OFF at dawn

**Pass Criteria**: 
- Correct transitions occur
- Second day shows improved accuracy (after first sync)

---

##### Test 3.3: Long-Term Drift Correction
**Objective**: Verify system maintains sync over weeks

**Setup**: Deploy for 2-4 weeks

1. Record actual dusk/dawn times (from weather data)
2. Note when device light transitions occur
3. **Expected**: Device tracks solar midnight within ±5 minutes

**Pass Criteria**: No manual adjustment needed over test period

---

## Troubleshooting

### Problem: Clock not incrementing
**Check**:
- Timer0 configuration (prescaler, reload values)
- Global interrupts enabled
- ISR implemented correctly
- Timer0 interrupt enable bit set

### Problem: Light doesn't respond to LDR
**Check**:
- ADC configuration (channel selection, ANSEL bits)
- LDR threshold values (may need tuning)
- GPIO output configuration (TRIS, LAT registers)

### Problem: Light stays on during 1am-5am
**Check**:
- Energy saving window constants
- Hour comparison logic in `app_control.c`
- Binary clock display shows correct hour

### Problem: No drift correction occurs
**Check**:
- First full night cycle completed (dusk → dawn)
- State machine transitioning correctly
- Drift calculation logic
- `SOLAR_Update()` returning non-zero value when correction needed

### Problem: Display shows incorrect binary
**Check**:
- Pin mapping in `config.h`
- PORTB configuration (TRIS bits)
- Mask application in `BSP_SetClockDisplay()`

## Test Results Template

```
Test Date: __________
Tester: __________
Hardware Version: __________
Software Version: __________

Test Mode Results:
[ ] Test 2.1: Basic Clock Function         Pass/Fail: ____
[ ] Test 2.2: Day/Night Detection          Pass/Fail: ____
[ ] Test 2.3: Energy Saving Window         Pass/Fail: ____
[ ] Test 2.4: Initial Calibration          Pass/Fail: ____
[ ] Test 2.5: Solar Midnight Calculation   Pass/Fail: ____
[ ] Test 2.6: Uneven Day/Night Cycles      Pass/Fail: ____
[ ] Test 2.7: DST Detection (Long Days)    Pass/Fail: ____
[ ] Test 2.8: DST Detection (Short Days)   Pass/Fail: ____
[ ] Test 2.9: Heartbeat LED                Pass/Fail: ____

Production Mode Results:
[ ] Test 3.1: Real-Time Clock              Pass/Fail: ____
[ ] Test 3.2: 24-Hour Cycle                Pass/Fail: ____
[ ] Test 3.3: Long-Term Drift Correction   Pass/Fail: ____

Notes:
_________________________________________________
_________________________________________________
_________________________________________________

Overall Assessment: Pass / Fail

Signature: ________________  Date: __________
```

## Performance Benchmarks

### Expected Accuracy
- **Initial startup**: ±12 hours (uncalibrated)
- **After first sync**: ±30 minutes
- **After 1 week**: ±5 minutes
- **Steady state**: ±2 minutes

### Response Times
- **LDR to state change**: < 1 main loop iteration (~1ms typical)
- **State change to light update**: < 1 main loop iteration
- **Display update**: Real-time (every loop)
- **Drift correction**: Applied immediately when calculated

### Resource Usage
- **Flash**: ~4-6 KB (varies with optimization)
- **RAM**: ~20 bytes (minimal static variables)
- **Stack**: ~50 bytes maximum (no recursion, minimal nesting)

## Conclusion

This testing guide ensures thorough verification of all system requirements. Complete test mode verification before proceeding to production deployment.
