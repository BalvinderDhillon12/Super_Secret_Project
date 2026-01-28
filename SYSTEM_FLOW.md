# System Operational Flow

## State Machine Diagram

```
                     POWER ON
                        |
                        v
                  +-----------+
                  | BSP_Init  |
                  +-----------+
                        |
                        v
                  +-----------+
                  | Read LDR  |
                  +-----------+
                        |
                        v
                 Is LDR < DUSK?
                   /         \
                 YES          NO
                  |            |
                  v            v
            Set 00:00     Set 12:00
            (Midnight)      (Noon)
                  |            |
                  +-----+------+
                        |
                        v
                  +-----------+
                  | SOLAR_Init|
                  +-----------+
                        |
                        v
                  +-----------+
                  |Enable ISR |
                  +-----------+
                        |
                        v
        +---------------+---------------+
        |       MAIN SUPER-LOOP         |
        +-------------------------------+
                        |
        +---------------+---------------+
        |                               |
        v                               v
    Read Sensors              Update Solar State
    - RTC_GetTime()           - SOLAR_Update()
    - BSP_GetLDR()            - Detect Dusk/Dawn
        |                     - Calculate drift
        |                     - Update DST
        +---------+-----------+
                  |
                  v
            Drift != 0?
               /    \
             YES     NO
              |      |
              v      |
        RTC_ApplySync |
              |      |
              +------+
                  |
                  v
          +----------------+
          | APP_Task()     |
          | - Check time   |
          | - Check dark   |
          | - Set light    |
          +----------------+
                  |
                  v
          +----------------+
          | Update Display |
          | Binary Clock   |
          +----------------+
                  |
                  v
          +----------------+
          | Heartbeat LED  |
          +----------------+
                  |
                  +--------> Loop back
```

## Solar State Machine

```
    UNKNOWN STATE (Initial)
           |
           v
    LDR < DUSK? ----NO----> LDR > DAWN?
           |                     |
          YES                   YES
           |                     |
           v                     v
    +------------+        +------------+
    |   NIGHT    |        |    DAY     |
    |  (Dark)    |        |  (Bright)  |
    +------------+        +------------+
           |                     |
           |                     |
    LDR > DAWN?           LDR < DUSK?
           |                     |
          YES                   YES
           |                     |
           +-------+   +---------+
                   |   |
                   v   v
              +-----------+
              | DAWN Edge |
              | - Record  |
              | - Calculate|
              | - Sync    |
              +-----------+
                   |
                   v
              Back to DAY
```

## Time Synchronization Flow

```
                    DUSK DETECTED
                         |
                         v
                  +-------------+
                  | Record Time |
                  | dusk_time   |
                  +-------------+
                         |
                         v
                    Wait for DAWN
                         |
                         v
                    DAWN DETECTED
                         |
                         v
        +--------------------------------+
        | Calculate Night Duration       |
        | night_dur = dawn - dusk        |
        +--------------------------------+
                         |
                         v
        +--------------------------------+
        | Calculate Solar Midnight       |
        | solar_mid = dusk + night_dur/2 |
        +--------------------------------+
                         |
                         v
        +--------------------------------+
        | Determine Season (DST)         |
        | day_dur = 1440 - night_dur     |
        | if day_dur > 14h: target = 1am |
        | if day_dur < 10h: target = 0am |
        +--------------------------------+
                         |
                         v
        +--------------------------------+
        | Calculate Drift                |
        | drift = solar_mid - target     |
        +--------------------------------+
                         |
                         v
                  drift > threshold?
                    /          \
                  YES           NO
                   |            |
                   v            v
            Apply Correction  Continue
            RTC_ApplySync()      |
                   |             |
                   +------+------+
                          |
                          v
                    Next Cycle
```

## Light Control Decision Tree

```
                    APP_Task()
                         |
                         v
                   Is it Dark?
                    /       \
                  NO         YES
                  |           |
                  v           v
             Light OFF    What hour?
                             |
                +------------+------------+
                |                         |
             0, 5-23                    1-4
                |                         |
                v                         v
           Light ON                  Light OFF
                                   (Energy Saving)
```

## Interrupt Service Routine Flow

```
        TIMER0 INTERRUPT
               |
               v
        +-------------+
        | Clear Flag  |
        | TMR0IF = 0  |
        +-------------+
               |
               v
        +-------------+
        | Reload TMR0 |
        +-------------+
               |
               v
        +-------------+
        | RTC_TickISR |
        +-------------+
               |
               v
      TEST_MODE enabled?
         /          \
       YES           NO
        |            |
        v            v
   Increment     Increment
   tick_counter  seconds
        |            |
        v            v
   >= 3600?      seconds++
        |            |
       YES           |
        |            |
        v            v
   Reset counter  >= 60?
   Increment         |
   seconds          YES
        |            |
        +-----+------+
              |
              v
        minutes++ 
              |
              v
          >= 60?
              |
             YES
              |
              v
         hours++
              |
              v
          >= 24?
              |
             YES
              |
              v
        hours = 0
        (Rollover)
              |
              v
        Return from ISR
```

## Critical Section Example

```
    Main Loop wants to read time
               |
               v
        +-------------+
        | Disable GIE |  <-- Enter Critical
        +-------------+
               |
               v
        +-------------+
        | Copy hours  |
        +-------------+
               |
               v
        +-------------+
        | Copy min    |
        +-------------+
               |
               v
        +-------------+
        | Copy sec    |
        +-------------+
               |
               v
        +-------------+
        | Enable GIE  |  <-- Exit Critical
        +-------------+
               |
               v
        Use copied values safely
        (No corruption from ISR)
```

## Data Flow Diagram

```
+----------+         +----------+
|  LDR     |-------->| ADCC     |
| (Analog) |         | Hardware |
+----------+         +----------+
                          |
                          v (ADC value 0-1023)
                    +------------+
                    | solar_mgr  |
                    | - Filter   |
                    | - Detect   |
                    | - Calculate|
                    +------------+
                          |
              +-----------+-----------+
              |                       |
              v (is_dark)             v (drift)
        +-----------+           +-----------+
        |app_control|           | rtc_soft  |
        |Light Rule |           | ApplySync |
        +-----------+           +-----------+
              |                       |
              v                       v
        +-----------+           +-----------+
        | BSP       |           | time      |
        | SetLight  |           | update    |
        +-----------+           +-----------+
              |                       |
              v                       v
        +-----------+           +-----------+
        | Relay/    |           | Binary    |
        | MOSFET    |           | Display   |
        +-----------+           +-----------+
```

## Timing Diagram (Test Mode - 24 Second Day)

```
Time (s)  |  0    4    8    12   16   20   24
----------|-----|-----|-----|-----|-----|-----|-
Hour      |  0    4    8    12   16   20    0
LDR       | Dark|Dark |Bright------|Dark-|Dark
Light     | ON  |OFF  |OFF---------|ON---|ON--
State     |Night|ESav |Day---------|Night|Night

ESav = Energy Saving Window (1am-5am)
```

## Example 24-Hour Cycle (Production Mode)

```
Time      | LDR   | Hour | Light State | Reason
----------|-------|------|-------------|------------------------
00:00     | Dark  | 0    | ON          | Night, not in ESav window
01:00     | Dark  | 1    | OFF         | Energy saving starts
02:00     | Dark  | 2    | OFF         | Energy saving active
03:00     | Dark  | 3    | OFF         | Energy saving active
04:00     | Dark  | 4    | OFF         | Energy saving active
05:00     | Dark  | 5    | ON          | Energy saving ends
06:00     | Dark  | 6    | ON          | Still dark
07:00     | Light | 7    | OFF         | Dawn detected
08:00     | Light | 8    | OFF         | Daytime
...       | Light | ... | OFF         | Daytime continues
18:00     | Light | 18   | OFF         | Still light
19:00     | Dark  | 19   | ON          | Dusk detected
20:00     | Dark  | 20   | ON          | Night
21:00     | Dark  | 21   | ON          | Night
22:00     | Dark  | 22   | ON          | Night
23:00     | Dark  | 23   | ON          | Night
```

## Memory Map (Approximate)

```
RAM Usage:
+------------------+
| ISR Variables    |  ~6 bytes (hours, minutes, seconds)
+------------------+
| Solar State      |  ~10 bytes (state, timestamps)
+------------------+
| Stack            |  ~50 bytes (max depth)
+------------------+
| Local Variables  |  ~10 bytes (loop vars)
+------------------+
Total RAM: ~76 bytes

Flash Usage:
+------------------+
| Config Bits      |  14 bytes
+------------------+
| ISR              |  ~200 bytes
+------------------+
| BSP Functions    |  ~1.5 KB
+------------------+
| RTC Functions    |  ~800 bytes
+------------------+
| Solar Manager    |  ~1.2 KB
+------------------+
| App Control      |  ~300 bytes
+------------------+
| Main Loop        |  ~200 bytes
+------------------+
Total Flash: ~4-6 KB (varies with optimization)
```

## Boot Sequence Timing

```
Event                    | Time (ms) | Cumulative
-------------------------|-----------|------------
Power On                 | 0         | 0
Oscillator Stable        | <1        | <1
BSP_Init - GPIO          | <1        | <1
BSP_Init - ADC           | <1        | <2
BSP_Init - Timer0        | <1        | <3
RTC_Init - Read LDR      | ~1        | ~4
RTC_Init - Set Time      | <1        | ~4
SOLAR_Init               | <1        | ~4
Enable Interrupts        | <1        | ~5
Enter Main Loop          | <1        | ~5

Total Boot Time: < 5ms
First Loop Iteration: < 10ms
```

## Performance Characteristics

### Loop Execution Time
- **Typical**: 100-500 μs per iteration
- **Maximum**: ~2 ms (with ADC conversion)
- **Frequency**: ~1000-10000 Hz (depends on operations)

### Interrupt Latency
- **Entry**: ~10-20 instruction cycles (~1.25-2.5 μs @ 16MHz)
- **ISR Duration**: ~50-100 instruction cycles
- **Exit**: ~10-20 instruction cycles
- **Total**: ~10 μs typical

### ADC Conversion Time
- **Single Sample**: ~20 μs (with FOSC/64 clock)
- **32-Sample Average**: ~640 μs
- **Called**: Once per main loop iteration

### Response Times
- **LDR Change to Detection**: < 1 loop iteration (~1 ms)
- **State Change to Output**: < 1 loop iteration
- **Time Update to Display**: < 1 loop iteration
- **Drift Correction**: Applied immediately (next loop)

This operational flow demonstrates the system's real-time behavior and timing characteristics.
