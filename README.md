# arduino-chickengate
Arduino based mechanic door controller

## ...what?
This is a door controller based on an Arduino. It opens a mechanic door of a hen house in the morning and closes it in the evening. That's it, basically. 

## Timing
Current date/time and open/close times are programmable via USB/serial console (need to wake up from power down sleep first by pressing one of the open or close buttons). Due to early sunset in winter time the open/close times are programmable for each month. I use a DS1307 Real Time Clock module, this allows to persist the timetable in its NVRAM.
Available commands:
- `time [hhmm]`: sets the current time
- `date [YYYYMMDD]`: sets the current date
- `gate [MM] [hhmm] [hhmm]`: sets for month MM the open and close times

## Mechanics
The door is openend/closed by a high torque gear motor, driven by a L9110 DC motor driver board. There are two buttons for manual opening/closing the door and two mechanical end switches (top and bottom).

## Power
Main issue is to keep power consumption low, because the controller as well as mechanics are driven by a small solar panel that feeds 4x1.2V rechargeable batteries. First of all, I got rid of the power LED on the ProMicro board by soldering it out. Due to high power consumption of the motor driver board, it is powered off via a transistor when not used. Finally onboard USB is disabled and the ProMicro is set to SLEEP_MODE_PWR_DOWN. Overall this brings power consumption down to around 20uA in idle mode.

## Hardware
- ProMicro Atmega 5V/16MHz (Dev-12640), https://cdn.sparkfun.com/datasheets/Dev/Arduino/Boards/ProMicro16MHzv1.pdf
- DS1307 Real Time Clock module
- L9110 DC motor driver board
- 6V high torque gear motor (ZnDiy-BRY 25GA-6V-80 DC 6V 80rpm)
- Solar panel, 0.5W, max voltage around 9V

## PINS
```
D2  --> I2C data (Realtime clock SDA)
D3  --> I2C clock (Realtime clock SCL)
D7  --> Wake up pin
D6  --> Motor A PWM Speed
D5  --> Motor A Direction
D9  --> Driver Power (Transistor Base)
D14 --> Up button, normally open
D15 --> Down button, normally open
D10 --> End switch top, normally closed
D16 --> End switch bottom, normally closed
```

## Scheme

![Scheme](https://github.com/gadget60/arduino-chickengate/blob/master/scheme.png)

## Images

![](https://github.com/gadget60/arduino-chickengate/blob/master/images/DSC_0789.JPG)


