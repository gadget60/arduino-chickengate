#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <RTClib.h>
#include <Wire.h>

#define WAKE_UP_PIN 7           // D7 --> Wake up pin
#define WAKE_UP_INTERRUPT 4     // D7 INT
#define MOTOR_A_PWM 6           // D6 --> Motor B PWM Speed
#define MOTOR_A_DIR 5           // D5 --> Motor B Direction
#define MOTOR_DRIVER_POWER 9    // D9 --> Driver Power (Transistor Base)
#define BUTTON_UP 14            // D14 Up button, normally open
#define BUTTON_DOWN 15          // D15 Down button, normally open
#define ENDSWITCH_TOP 10        // D10 --> End switch top, normally closed
#define ENDSWITCH_BOTTOM 16     // D16 --> End switch bottom, normally closed

int RXLED = 17;                 // The RX LED has a defined Arduino pin
volatile bool watchdogActivated = false;
volatile bool externalActivated = false;
int sleepIterations = 0;
String inputString = "";         // a string to hold incoming data
int count = 0;                   // counter
int times[48];                   // gate open/close times

RTC_DS1307 rtc;

// external interrupt procedure
void externalWakeupProcedure()
{
  // Set the watchdog activated flag.
  externalActivated = true;
}

// watchdog timer interrupt procedure.
ISR(WDT_vect)
{
  // Set the watchdog activated flag.
  watchdogActivated = true;
}

void sleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // enables the sleep bit in the mcucr register
  sleep_enable();          
  
  attachInterrupt(WAKE_UP_INTERRUPT, externalWakeupProcedure, LOW);
  
  sleep_mode();

  // zzzz...
  // here the device is actually put to sleep!!

  sleep_disable();
  
  detachInterrupt(WAKE_UP_INTERRUPT);
}


void setup(void)
{ 

  pinMode( WAKE_UP_PIN, INPUT_PULLUP);
  
  pinMode( MOTOR_A_DIR, OUTPUT );
  pinMode( MOTOR_A_PWM, OUTPUT );
  pinMode( MOTOR_DRIVER_POWER, OUTPUT );
  
  pinMode( BUTTON_UP, INPUT_PULLUP);
  pinMode( BUTTON_DOWN, INPUT_PULLUP);
  
  pinMode( ENDSWITCH_TOP, INPUT_PULLUP);
  pinMode( ENDSWITCH_BOTTOM, INPUT_PULLUP);
  
  pinMode(RXLED, OUTPUT);  // Set RX LED as an output

  digitalWrite( MOTOR_A_DIR, HIGH );
  digitalWrite( MOTOR_A_PWM, HIGH );
  digitalWrite( MOTOR_DRIVER_POWER, LOW );
  
  Serial.begin(9600);
 
  rtc.begin();

  for (int i=0; i<48; i++) {
    times[i] = rtc.readnvram(i);
  }

  Serial.println("Initializing, waiting...");

  // wait some seconds - this is just to avoid bricking the arduino
  // if something goes wrong in the wake up process
  delay(5000);

  // setup watchdog timer:
  // Note that the default behavior of resetting the Arduino
  // with the watchdog will be disabled.

  // This next section of code is timing critical, so interrupts are disabled.
  noInterrupts();

  // Set the watchdog reset bit in the MCU status register to 0.
  MCUSR &= ~(1<<WDRF);

  // Set WDCE and WDE bits in the watchdog control register.
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set watchdog clock prescaler bits to a value of 8 seconds.
  WDTCSR = (1<<WDP0) | (1<<WDP3);

  // Enable watchdog as interrupt only (no reset).
  WDTCSR |= (1<<WDIE);

  // Enable interrupts again.
  interrupts();
  
  Serial.println("done.");

}


void openGate()
{
  if (digitalRead(ENDSWITCH_TOP) == LOW) {
  
    digitalWrite( MOTOR_A_DIR, LOW ); 
    digitalWrite( MOTOR_A_PWM, HIGH ); 
    digitalWrite( MOTOR_DRIVER_POWER, HIGH );
  
    int i=0;
    do {
      i++;
      delay(100);

      // should not take more than 60 secounds (600 * 0.1),
      // otherwise something is probably broken...
    } while ((i<600) && (digitalRead(ENDSWITCH_TOP) == LOW) && (digitalRead(BUTTON_DOWN) == HIGH));
  
    digitalWrite( MOTOR_DRIVER_POWER, LOW );
    digitalWrite( MOTOR_A_DIR, HIGH );
    digitalWrite( MOTOR_A_PWM, HIGH );
    
    delay(500);
  }
}


void closeGate()
{
  if (digitalRead(ENDSWITCH_BOTTOM) == LOW) {
    
    digitalWrite( MOTOR_A_DIR, HIGH ); 
    digitalWrite( MOTOR_A_PWM, LOW ); 
    digitalWrite( MOTOR_DRIVER_POWER, HIGH );
  
    int i=0;
    do {
      i++;
      delay(100);

      // should not take more than 60 secounds (600 * 0.1)
      // otherwise something is probably broken...
    } while ((i<600) && (digitalRead(ENDSWITCH_BOTTOM) == LOW) && (digitalRead(BUTTON_UP) == HIGH));
    
    // wait some extra time in order to mechanically close the gate
    delay(1500);
    
    digitalWrite( MOTOR_DRIVER_POWER, LOW );
    digitalWrite( MOTOR_A_DIR, HIGH );
    digitalWrite( MOTOR_A_PWM, HIGH );
  
    delay(500);
  }
}


void printDigit(int n)
{
  if (n < 10) {
    Serial.print("0");
  }
  Serial.print(n, DEC);
}


void timeCommand(String param)
{
  DateTime newDate = rtc.now();
  if (param != "") {
    DateTime curDate = rtc.now();
    int newHour = param.substring(0,2).toInt();
    int newMin = param.substring(2).toInt();
    newDate = DateTime (curDate.year(), curDate.month(), curDate.day(), newHour, newMin, 0);
    rtc.adjust(newDate);
  }
  printDigit(newDate.hour());
  Serial.print(':');
  printDigit(newDate.minute());
  Serial.print(':');
  printDigit(newDate.second());
  Serial.println();
}

void dateCommand(String param)
{
  DateTime newDate = rtc.now();
  if (param != "") {
    DateTime curDate = rtc.now();
    int newYear = param.substring(0,4).toInt();
    int newMonth = param.substring(4,6).toInt();
    int newDay = param.substring(6).toInt();
    newDate = DateTime (newYear, newMonth, newDay, curDate.hour(), curDate.minute(), 0);
    rtc.adjust(newDate);
  }
  printDigit(newDate.day());
  Serial.print('.');
  printDigit(newDate.month());
  Serial.print('.');
  printDigit(newDate.year());
  Serial.println();
}


void gateCommand(String param)
{
  if (param != "") {
    
    int month = param.substring(0,2).toInt();
    if (month > 0 && month < 13) {
      
      int openHour = param.substring(3,5).toInt();
      int openMin = param.substring(5,7).toInt();
      int closeHour = param.substring(8,10).toInt();
      int closeMin = param.substring(10,12).toInt();  
      
      rtc.writenvram(((month - 1) * 4) + 0, openHour);
      rtc.writenvram(((month - 1) * 4) + 1, openMin);
      rtc.writenvram(((month - 1) * 4) + 2, closeHour);
      rtc.writenvram(((month - 1) * 4) + 3, closeMin);

      // re-read all from nvram
      for (int i=0; i<48; i++) {
        times[i] = rtc.readnvram(i);
      }  
    }
  }
  
  for (int i=0; i<12; i++) {
    Serial.print("Month ");
    printDigit(i + 1);
    Serial.print(": open ");
    printDigit(times[(i*4)+0]);
    Serial.print(":");
    printDigit(times[(i*4)+1]);
    Serial.print(", close ");
    printDigit(times[(i*4)+2]);
    Serial.print(":");
    printDigit(times[(i*4)+3]);
    Serial.print("\n");
  }
  
}


void onWakeupFromButton()
{
  
  delay(100);
  sei();
  USBDevice.attach(); 
  delay(100);

  Serial.begin(9600);
  delay(100);

  count = 0;
  
  while (count < 600) {

    if (digitalRead(BUTTON_DOWN) == LOW) {
      count = 0;
      Serial.print("Close gate...");
      closeGate();
      Serial.print("done.");
    }
    if (digitalRead(BUTTON_UP) == LOW) {
      count = 0;
      Serial.print("Open gate...");
      openGate();
      Serial.print("done.");
    }
    
    // process the serial input
    while (Serial.available()) {
      count = 0;
      char inChar = (char) Serial.read();

      // execute command on newline
      if (inChar == '\n') {
        
        inputString.trim();
        
        String param = "";
        String command = inputString;
        int index = inputString.indexOf(" ");
        if (index >= 0) {
          param = inputString.substring(index);
          param.trim();
          command = command.substring(0, index);
        }
        
        if (command == "time") {
          timeCommand(param);
        } else if (command == "date") {
          dateCommand(param);
        } else if (command == "gate") {
          gateCommand(param);
        } else {
          Serial.println("Unknown command " + command);
          Serial.println("Available commands are:");
          Serial.println("- time [<hhmm>]");
          Serial.println("- date [<YYYYMMDD>]");
          Serial.println("- gate [<MM> <hhmm> <hhmm>]");
        }
        
        inputString = "";

      } else {

        // add it to the inputString:
        inputString += inChar;

      }
    }
     
    delay(100);
    count++; 
  }
  
  Serial.println("Timer: Entering Sleep mode after 60 seconds inactivity");

  // this delay is needed, the sleep
  // function will provoke a Serial error otherwise!!
  delay(100);     
                  
  // detach the USB
  USBCON |= _BV(FRZCLK);  //freeze USB clock
  PLLCSR &= ~_BV(PLLE);   // turn off USB PLL
  USBCON &= ~_BV(USBE);   // disable USB  

}


void onWakeupFromTimer()
{
  sleepIterations++;
  
  // make sure at least once a minute:
  if (sleepIterations >= 3) {

    digitalWrite(RXLED, LOW);
    delay(50);
    digitalWrite(RXLED, HIGH);

    DateTime curDate = rtc.now();
    int i = ( curDate.month() - 1) * 4;
    
    if (curDate.hour() == times[i + 0] && curDate.minute() == times[i + 1]) {
      openGate();
    }
    if (curDate.hour() == times[i + 2] && curDate.minute() == times[i + 3]) {
      closeGate();
    }

    // security setting: always close the door at 22pm
    if (curDate.hour() == 22 && curDate.minute() == 00) {
      closeGate();
    }

    sleepIterations = 0;
    // do something
  } 
}



void loop(void)
{

  // detach the USB
  USBCON |= _BV(FRZCLK);  // freeze USB clock
  PLLCSR &= ~_BV(PLLE);   // turn off USB PLL
  USBCON &= ~_BV(USBE);   // disable USB  

  while(true) {
    
    if (watchdogActivated) {
      // timer interrupt 
      onWakeupFromTimer();
      watchdogActivated = false;
    }    
    
    if (externalActivated) {
      // external interrupt
      onWakeupFromButton();
      externalActivated = false;
    }
    
    if (watchdogActivated == false && externalActivated == false) {
      sleep();
    }
 
  }

}

