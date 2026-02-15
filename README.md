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

The system is designed with safety in mind, if the wipers are moving and the driver turns them off or 
shuts down the engine, the wipers always complete their current sweep and return to the parked position 
at 0 degrees. This prevents the wipers from stopping mid-windshield where they could block the driver's 
view.

For this project, we chose to use a position servo motor for the wiper mechanism because the motion we 
want to achieve is less than 180 degrees, which the position servo is perfectly suited for. Position 
servos provide precise angular control, allowing us to accurately sweep between 0 and 90 degrees and 
reliably return to the exact parked position. This precision is essential for mimicking real windshield 
wiper behavior and ensuring the wiper consistently stops at the bottom of its stroke. All timing was 
verified with a stopwatch, and each mode operates within the expected parameters.

### links:
This system was built from [Nathan's repository](https://github.com/goldstn2-oss/project-2-nathan-jacob)

### How we determind our speed and our testing results summary:

Given for Low/Intermittent: (10rev/min)(1min/60sec)(360deg/1 rev) = 60 deg/sec
(90 deg/60(deg/sec)) =  1.5 second  --> this number is how long it takes the servo to rotate 90 deg

Calculated for High: (25rev/min)(1min/60sec)(360deg/1 rev) = 150 deg/sec
(90 deg/150(deg/sec)) = 0.6 second  --> this number is how long it takes the servo to rotate 90 deg

We know, very generally, the duty cycle controls the servo motor's position (angle). Therefore we knew that we could change the duty cycle at some rate to increase/decrease the speed. Knowing this we created this formula to calculate how much we would have to change the duty cycle per each speed.

duty increase per cycle = ((LEDC_DUTY_MAX - LEDC_DUTY_MIN)*(seconds per 90degree cyle))/(how long it takes the servo to rotate 90 deg)

In simple terms (LEDC_DUTY_MAX - LEDC_DUTY_MIN) is essentially 90 degrees, seconds per 90 degree cycle for us is 0.024, and the 'how long it takes the servo to rotate 90 degree is what we calculated at the beginning of this section;

so our final equation is either (low/intermittent)

duty increase per cycle = ((LEDC_DUTY_DIFF)*(0.024))/(1.5)

or (high)

duty increase per cycle = ((LEDC_DUTY_DIFF)*(0.024))/(0.6)


We observed improved accuracy at larger loop delays. This is likely because very small update intervals caused duty cycle changes to occur faster than the servo could respond. Increasing the loop delay most likely allowed the servo’s internal control loop to settle between updates, resulting in smoother motion and more accurate timing.

We tested this below:  

|             HIGH             |                                                     |
|:----------------------------:|-----------------------------------------------------|
|  GOAL is 150 degrees per sec |                                                     |
| Time Per 10 Sweeps           | Degrees per sec (180 Degrees*10)/Time Per 10 Sweeps |
| 11.77                        | 152.93                                              |
| 11.97                        | 150.37                                              |
| 11.88                        | 151.52                                              |
| 11.91                        | 151.13                                              |
| 11.8                         | 152.93                                              |
| 11.89                        | 151.38                                              |
| 11.89                        | 151.38                                              |
| 11.68                        | 154.109589                                          |
| 11.94                        | 150.75                                              |
| 11.89                        | 151.37                                              |

|     Low && Intermittent     |                                                     |
|:---------------------------:|-----------------------------------------------------|
|  GOAL is 60 degrees per sec |                                                     |
| Time Per 10 Sweeps          | Degrees per sec (180 Degrees*10)/Time Per 10 Sweeps |
| 29.63                       | 60.74                                               |
| 29.80                       | 60.40                                               |
| 29.88                       | 60.24                                               |
| 29.73                       | 60.54                                               |
| 29.76                       | 60.48                                               |
| 29.52                       | 60.97                                               |
| 29.85                       | 60.30                                               |
| 29.84                       | 60.32                                               |
| 29.67                       | 60.67                                               |
| 29.4                        | 61.22                                               |

Reflections:

- High Speed: The average is approximately 151.52 degrees per second 
- Percent Difference: 1.01%
- Low Speed: The average is approximately 60.59 degrees per second.
- Percent Difference: 0.98%

The error could probably be due to human reaction time when using the stopwatch. Since the measurements were taken manually over 10 sweeps, small delays in pressing start or stop can introduce slight variations in the calculated degrees per second, leading to the 1% difference from the theoretical goals.


## IGNITION SUBSYSTEM 
| Specification | Process | Result |
|---|---|---|
| When the driver sits down, the system prints "Welcome to enhanced alarm system model 218-W26" exactly once. | *1 button: driver_seat*<br>1. Press driver seat button<br>2. Press driver seat button again after reset<br>3. Press driver seat button while passenger seat is also pressed | All tests passed<br>1. Welcome message appears<br>2. Message appears only on first press<br>3. Message appears only once |
| Green LED lights only when both seats are occupied and both seatbelts are fastened. | *2 buttons: driver_seat, pass_seat; 2 switches: driver_belt, pass_belt*<br>1. All engaged<br>2. One seatbelt unfastened<br>3. One seat unoccupied<br>4. None engaged | All tests passed<br>1. Green LED on<br>2–4. Green LED off |
| If ignition button is pressed while green LED is on, engine starts (red LED on, green LED off, "Engine started" displayed). | *1 button: ignition*<br>1. Green LED on → press ignition | All tests passed<br>1. Red LED on, green LED off, message displayed |
| If ignition button is pressed while green LED is off, alarm sounds and error messages display all missing conditions. | *1 button: ignition*<br>1. Green LED off → press ignition<br>2. Multiple conditions unmet | All tests passed<br>1. Alarm sounds, error messages shown<br>2. All unmet conditions listed |
| When engine is running and ignition button is pressed again, engine stops (Red LED off). | *Engine running → press ignition* | All tests passed<br>1. Red LED off, engine stops |
| When engine is running and the seatbelts are unbuckled or if the driver seat is left vacant, the ignition stays on. | 1. Press driver seat button <br> 2. Press driver seat belt button <br> 3. Press passenger seat button | All tests passed <br> 1-3. the Red LED stays on which means the ignition is still on |

## WINDSHIELD WIPER SUBSYSTEM 
| Specification | Process | Result |
|---|---|---|
| Wipers only run if engine is running. If engine is off, wipers remain stationary at 0°. | *Engine off → set wiper mode to HI, LO, or INT* | All tests passed<br>1. Wiper remains at 0°, no motion |
| In HI mode, wiper continuously cycles 0° → 90° → 0° at 25 rpm. | *Engine on → set wiper mode to HI*<br>1. Measure time for 10 cycles | All tests passed<br>1. Time ≈ 24 seconds (2.4 sec/cycle) |
| In LO mode, wiper continuously cycles 0° → 90° → 0° at 10 rpm. | *Engine on → set wiper mode to LO*<br>1. Measure time for 10 cycles | All tests passed<br>1. Time ≈ 60 seconds (6 sec/cycle) |
| In INT mode, wiper runs at low speed with a delay at 0° based on delay selector: SHORT (1s), MEDIUM (3s), LONG (5s). | *Engine on → set wiper mode to INT*<br>1. Set delay to SHORT, measure 10 cycles<br>2. Set delay to MEDIUM, measure 10 cycles<br>3. Set delay to LONG, measure 10 cycles | All tests passed<br>1. Each cycle ≈ 7 sec (6 sec LO + 1s delay)<br>2. Each cycle ≈ 9 sec<br>3. Each cycle ≈ 11 sec |
| LCD displays selected wiper mode. In INT mode, also displays delay time (SHORT, MEDIUM, LONG). | *Engine on → change wiper mode and delay*<br>1. Switch to HI, LO, OFF<br>2. Switch to INT with each delay | All tests passed<br>1. Display shows HI, LO, OFF<br>2. Display shows INT with SHORT/MEDIUM/LONG |
| If engine is turned off while wipers are moving, they complete the current cycle and stop at 0°. | *Engine on, wipers in HI/LO/INT → press ignition to stop engine mid-cycle* | All tests passed<br>1. Wiper finishes sweep, stops at 0°, engine off |
| If engine is turned off while in INT mode delay, wipers remain stationary. | *Engine on, wipers in INT and in delay → press ignition to stop engine* | All tests passed<br>1. Wiper stays at 0°, engine off |
