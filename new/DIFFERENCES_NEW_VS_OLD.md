# Differences Between `new/` and `old/`

Summary of how the **new** implementation differs from the **old** (reference) codebase. File names: **new** uses PascalCase (`Main.c`, `Config.h`, `ADC.c`, `LEDS.c`, `Timer.c`); **old** uses lowercase (`main.c`, `config.h`, `adc.c`, `leds.c`, `timer.c`).

---

## 1. File layout and extras

| Item | new/ | old/ |
|------|------|------|
| Makefile | No | Yes (`Makefile` for xc8-cc) |
| ADC channel definition | In `Config.h` as `ADC_LDR_CHANNEL 0x03` | In `adc.h` as `ADC_LDR_CHANNEL 3` |
| Config include guard | `Config_H` | `CONFIG_H` |
| Extra docs | `READme_project`, `README.md` | `PROJECT_PROMPT.md`, `README.md` |

---

## 2. Main.c / main.c

| Aspect | new/ | old/ |
|--------|------|------|
| Structure | Single `main()`, inline solar logic in one big loop | `SystemInit()`, then loop; solar logic in helpers (`SolarUpdate`, `HandleDuskTransition`, `HandleDawnTransition`, `ApplySync`, etc.) |
| Time type | No `Time_t` struct; uses `g_hours`, `g_minutes`, `g_seconds` directly | `Time_t` struct; `GetTime()` / `SetTime()` |
| Timekeeping | One block: advance every `TICKS_PER_SECOND` (same for TEST_MODE and production) | Same idea: advance every `TICKS_PER_SECOND` |
| Solar state | Same enum `SolarState_t` (UNKNOWN, DAY, NIGHT) and hysteresis (DUSK/DAWN thresholds) | Same; plus `g_last_dusk_time`, `g_first_cycle_complete` |
| Sensor check rate | TEST_MODE: every `TICKS_PER_SECOND`; production: every 60 ticks | LDR read every loop iteration (no interval) |
| Heartbeat | Time-based: toggle every `TICKS_PER_SECOND * 2` | Iteration-based: toggle every 30000 loop iterations |
| Loop delay | `__delay_ms(10)` at end of loop | No delay |
| Initial `g_target_solar_midnight` | `0` | `SOLAR_MIDNIGHT_WINTER` |

Logic (timekeeping, dusk/dawn, drift correction, energy save 1am–5am) is aligned; **new** is more compact, **old** is more split into functions and documented.

---

## 3. Config.h / config.h

| Aspect | new/ | old/ |
|--------|------|------|
| Timer reload (TEST_MODE) | `0xFFFC` (0xFF, 0xFC), `TICKS_PER_SECOND = 3600` | Same |
| Timer reload (production) | `0x0BDB` (0x0B, 0xDB), `TICKS_PER_SECOND = 1` | Same |
| Prescaler | 1:256 (in Timer.c) | 1:256 |
| `ADC_LDR_CHANNEL` | In Config.h, value `0x03` | Not in config.h; in adc.h as `3` |
| `SECONDS_PER_HOUR` | Defined (3600) | Not defined |
| Comments | Shorter | Longer (e.g. timer math, TEST_MODE explanation) |

Behavior is the same; **new** keeps ADC channel in Config, **old** in adc.h.

---

## 4. ADC.c / adc.c

| Aspect | new/ | old/ |
|--------|------|------|
| Mode | **Single conversion** (no burst-average); no `ADCON2bits.MD` or `ADRPT` | **Burst-average**: `ADCON2bits.MD = 0b010`, `ADRPT = 32` |
| Result register | `ADRESH` / `ADRESL` (one 10-bit conversion, 0–1023) | `ADFLTRH` / `ADFLTRL`, then `>> 5` (32-sample average) |
| Timeout | Yes: `ADC_ReadLDR()` has a timeout; returns 512 if ADC never completes | No timeout; can hang if ADC never clears `ADGO` |
| Includes | `ADC.h`, `Config.h` (for `ADC_LDR_CHANNEL`) | `adc.h` only |

**new** is tuned for “burst average doesn’t work on my board”: single conversion, no MD/ADRPT, plus timeout so the rest of the system (e.g. LEDs) doesn’t get stuck.

---

## 5. LEDS.c / leds.c

| Aspect | new/ | old/ |
|--------|------|------|
| ANSEL for LED pins | **Yes**: `ANSELA2`, `ANSELA4`, `ANSELA5`, `ANSELF6`, `ANSELF0` cleared so LEDs 3–7 are digital outputs | No ANSEL; only TRIS/LAT |
| Pin usage | Same mapping (RG0, RG1, RA2, RF6, RA4, RA5, RF0, RB0, RB1) | Same |
| Includes | `LEDS.h`, `Config.h` (for `HOURS_PER_DAY`) | `leds.h`, `config.h` |

**new** explicitly forces analog-capable pins (e.g. RA2, RF6) to digital so LEDs 3 and 4 don’t stay stuck on.

---

## 6. Timer.c / timer.c

| Aspect | new/ | old/ |
|--------|------|------|
| Prescaler | `T0CON1bits.T0CKPS = 0b1000` (1:256) | Same |
| Reload | From Config (TMR0_RELOAD_HIGH/LOW) | Same |
| Logic | Same: ISR clears flag, reloads, increments `s_tick_count`; `Timer_GetTicks` / `Timer_ResetTicks` | Same |
| Style | Shorter comments | More comments (e.g. “atomic 32-bit read”) |

Functionally the same.

---

## 7. Headers (ADC.h, LEDS.h, Timer.h)

| File | new/ | old/ |
|------|------|------|
| **ADC.h** | Only declarations; no `ADC_LDR_CHANNEL` (lives in Config.h) | Declarations + `ADC_LDR_CHANNEL` and longer descriptions |
| **LEDS.h** | Minimal declarations | Same declarations, more @brief/@param |
| **Timer.h** | Minimal declarations, typo “elasped” | Same declarations, no typo, more documentation |

**new** headers are slimmer; **old** has more Doxygen-style comments and keeps `ADC_LDR_CHANNEL` in the ADC header.

---

## 8. Summary table

| Area | new/ | old/ |
|------|------|------|
| **ADC mode** | Single conversion, no burst-average, timeout on read | Burst-average, 32 samples, no timeout |
| **LED pins** | ANSEL cleared for RA2, RA4, RA5, RF6, RF0 (digital) | No ANSEL; digital by default/tristate |
| **Main loop** | One place: timekeeping + solar + LED + heartbeat + 10 ms delay | Same behavior via helpers; no loop delay; heartbeat by iteration count |
| **Build** | No Makefile in new/ | Makefile present in old/ |
| **Naming** | PascalCase filenames, `Config_H`, includes `Config.h` / `ADC.h` / `LEDS.h` / `Timer.h` | Lowercase filenames, `CONFIG_H`, includes `config.h` / `adc.h` / `leds.h` / `timer.h` |

The main behavioral differences are: **new** uses single-conversion ADC with a timeout and clears ANSEL on LED pins; **old** uses burst-average ADC and does not touch ANSEL. The rest is structure, naming, and documentation.
