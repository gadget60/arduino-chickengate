# arduino-chickengate
Arduino based mechanic door controller

## ...what?
This is an automatic door controller based on a ProMicro Atmega 5V/16MHz (Dev-12640). It opens a mechanic door of a hen house in the morning and closes it in the evening. That's it, basically. 

## Timing
Current date/time and open/close times are programmable via USB/serial console (need to wake up from power down sleep first by pressing one of the open or close buttons). Due to early sunset in winter time the open/close times are programmable for each month. I use a DS1307 Real Time Clock module, this allows to persist the timetable in its NVRAM.
Available commands:
`time [hhmm]`: sets the current time
`date [YYYYMMDD]`: sets the current date
`gate [MM] [hhmm] [hhmm]`: sets for month MM the open and close times

## Mechanics
The door is openend/closed by a high torque gear motor, driven by a L9110 DC stepper driver board. There are two buttons for manual opening/closing the door and two mechanical end switches (top and bottom).

## Power
Main issue is to keep power consumption low, because the controller as well as mechanics are driven by a small solar panel that feeds 4x1.2V rechargeable batteries. First of all, I got rid of the power LED on the ProMicro board by soldering. Due to high power consumption of the motor driver board, it is powered off via a transistor in idle mode. Finally onboard USB is disabled and the ProMicro is set to SLEEP_MODE_PWR_DOWN.

## Hardware
- ProMicro Atmega 5V/16MHz (Dev-12640)
- DS1307 Real Time Clock module
- L9110 DC stepper driver board
- 6V high torque gear motor (ZnDiy-BRY 25GA-6V-80 DC 6V 80rpm)

## PINS
```
#define WAKE_UP_PIN 7           // D7 --> Wake up pin
#define WAKE_UP_INTERRUPT 4     // D7 INT
#define MOTOR_A_PWM 6           // D6 --> Motor A PWM Speed
#define MOTOR_A_DIR 5           // D5 --> Motor A Direction
#define MOTOR_DRIVER_POWER 9    // D9 --> Driver Power (Transistor Base)
#define BUTTON_UP 14            // D14 Up button, normally open
#define BUTTON_DOWN 15          // D15 Down button, normally open
#define ENDSWITCH_TOP 10        // D10 --> End switch top, normally closed
#define ENDSWITCH_BOTTOM 16     // D16 --> End switch bottom, normally closed
```

