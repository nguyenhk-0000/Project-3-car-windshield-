# Project 3: Windshield Wiper Subsystem
## Team members: Nguyen Hieu Kien, Nathaniel  Goldstein 
### Project description:
This project is a safety system designed for student driver cars and is controlled by a small computer. 
To follow state safety laws, it ensures a supervisor is present by checking that both front seats are 
filled and both seatbelts are buckled before the car can start. If someone tries to start the engine 
without meeting these safety rules, an alarm sounds and the screen explains exactly what is missing.

Once the engine is running, the system also controls the windshield wipers with four different modes: 
high speed, low speed, intermittent, and off. In high and low speed modes, the wiper continuously 
sweeps back and forth at either 25 or 10 cycles per minute. In intermittent mode, the wiper moves at 
low speed but pauses at the bottom for a driver-selected delay of 1, 3, or 5 seconds before starting 
another sweep. The current wiper mode and delay setting are shown on the display.

### links:
This system was built from [Nathan's repository](https://github.com/goldstn2-oss/project-2-nathan-jacob)

### testing results summary:
## Testing Results

| Specification | Process | Result |
|---|---|---|
| When the driver sits down, the system prints "Welcome to enhanced alarm system model 218-W26" exactly once. | *1 button: driver_seat*<br>1. Press driver seat button<br>2. Press driver seat button again after reset<br>3. Press driver seat button while passenger seat is also pressed | All tests passed<br>1. Welcome message appears<br>2. Message appears only on first press<br>3. Message appears only once |
| Green LED lights only when both seats are occupied and both seatbelts are fastened. | *2 buttons: driver_seat, pass_seat; 2 switches: driver_belt, pass_belt*<br>1. All engaged<br>2. One seatbelt unfastened<br>3. One seat unoccupied<br>4. None engaged | All tests passed<br>1. Green LED on<br>2–4. Green LED off |
| If ignition button is pressed while green LED is on, engine starts (red LED on, green LED off, "Engine started" displayed). | *1 button: ignition*<br>1. Green LED on → press ignition | All tests passed<br>1. Red LED on, green LED off, message displayed |
| If ignition button is pressed while green LED is off, alarm sounds and error messages display all missing conditions. | *1 button: ignition*<br>1. Green LED off → press ignition<br>2. Multiple conditions unmet | All tests passed<br>1. Alarm sounds, error messages shown<br>2. All unmet conditions listed |
| Once engine is running, it stays running even if seats are vacated or seatbelts removed. | *Engine running → release all seat buttons and switches*<br>1. Remove driver<br>2. Remove passenger<br>3. Unfasten belts | All tests passed<br>1–3. Engine continues running (Red LED stays on) |
| When engine is running and ignition button is pressed again, engine stops (Red LED off). | *Engine running → press ignition* | All tests passed<br>1. Red LED off, engine stops |
| Wipers only run if engine is running. If engine is off, wipers remain stationary at 0°. | *Engine off → set wiper mode to HI, LO, or INT* | All tests passed<br>1. Wiper remains at 0°, no motion |
| In HI mode, wiper continuously cycles 0° → 90° → 0° at 25 rpm (period ≈ 2.4 seconds). | *Engine on → set wiper mode to HI*<br>1. Measure time for 10 cycles | All tests passed<br>1. Time ≈ 24 seconds (2.4 sec/cycle) |
| In LO mode, wiper continuously cycles 0° → 90° → 0° at 10 rpm (period ≈ 6 seconds). | *Engine on → set wiper mode to LO*<br>1. Measure time for 10 cycles | All tests passed<br>1. Time ≈ 60 seconds (6 sec/cycle) |
| In INT mode, wiper runs at low speed with a delay at 0° based on delay selector: SHORT (1s), MEDIUM (3s), LONG (5s). | *Engine on → set wiper mode to INT*<br>1. Set delay to SHORT, measure 10 cycles<br>2. Set delay to MEDIUM, measure 10 cycles<br>3. Set delay to LONG, measure 10 cycles | All tests passed<br>1. Each cycle ≈ 7 sec (6 sec LO + 1s delay)<br>2. Each cycle ≈ 9 sec<br>3. Each cycle ≈ 11 sec |
| LCD displays selected wiper mode. In INT mode, also displays delay time (SHORT, MEDIUM, LONG). | *Engine on → change wiper mode and delay*<br>1. Switch to HI, LO, OFF<br>2. Switch to INT with each delay | All tests passed<br>1. Display shows HI, LO, OFF<br>2. Display shows INT with SHORT/MEDIUM/LONG |
| If wiper mode is turned to OFF while wipers are moving, they complete the current cycle and stop at 0°. | *Engine on, wipers in HI/LO/INT → turn mode to OFF mid-cycle* | All tests passed<br>1. Wiper finishes current sweep, stops at 0° |
| If wiper mode is turned to OFF while in INT mode delay, wipers remain stationary. | *Engine on, wipers in INT and in delay → turn mode to OFF* | All tests passed<br>1. Wiper stays at 0°, no further motion |
| If engine is turned off while wipers are moving, they complete the current cycle and stop at 0°. | *Engine on, wipers in HI/LO/INT → press ignition to stop engine mid-cycle* | All tests passed<br>1. Wiper finishes sweep, stops at 0°, engine off |
| If engine is turned off while in INT mode delay, wipers remain stationary. | *Engine on, wipers in INT and in delay → press ignition to stop engine* | All tests passed<br>1. Wiper stays at 0°, engine off |
| [Challenge] If engine is turned off while wipers are moving, they freeze in place. When engine restarts, they return to 0° at low speed. | *Engine on, wipers moving → turn off engine → restart engine* | All tests passed<br>1. Wipers freeze mid-sweep<br>2. On restart, they complete cycle slowly to 0° |
