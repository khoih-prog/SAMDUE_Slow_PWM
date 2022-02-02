# SAMDUE_Slow_PWM Library

[![arduino-library-badge](https://www.ardu-badge.com/badge/SAMDUE_Slow_PWM.svg?)](https://www.ardu-badge.com/SAMDUE_Slow_PWM)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/SAMDUE_Slow_PWM.svg)](https://github.com/khoih-prog/SAMDUE_Slow_PWM/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/SAMDUE_Slow_PWM/blob/main/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/SAMDUE_Slow_PWM.svg)](http://github.com/khoih-prog/SAMDUE_Slow_PWM/issues)

---
---

## Table of Contents

* [Important Change from v1.2.0](#Important-Change-from-v120)
* [Why do we need this SAMDUE_Slow_PWM library](#why-do-we-need-this-SAMDUE_Slow_PWM-library)
  * [Features](#features)
  * [Why using ISR-based PWM is better](#why-using-isr-based-pwm-is-better)
  * [Currently supported Boards](#currently-supported-boards)
  * [Important Notes about ISR](#important-notes-about-isr)
* [Changelog](changelog.md)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
  * [Use Arduino Library Manager](#use-arduino-library-manager)
  * [Manual Install](#manual-install)
  * [VS Code & PlatformIO](#vs-code--platformio)
* [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)
* [Usage](#usage)
  * [1. Init ISR_PWM](#1-init-ISR_PWM)
  * [2. Set PWM Frequency, dutycycle, attach irqCallbackStartFunc and irqCallbackStopFunc functions](#2-Set-PWM-Frequency-dutycycle-attach-irqCallbackStartFunc-and-irqCallbackStopFunc-functions)
* [Examples](#examples)
  * [ 1. ISR_8_PWMs_Array](examples/ISR_8_PWMs_Array)
  * [ 2. ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)
  * [ 3. ISR_8_PWMs_Array_Simple](examples/ISR_8_PWMs_Array_Simple)
  * [ 4. ISR_Changing_PWM](examples/ISR_Changing_PWM)
  * [ 5. ISR_Modify_PWM](examples/ISR_Modify_PWM)
  * [ 6. multiFileProject](examples/multiFileProject) **New**
* [Example ISR_8_PWMs_Array_Complex](#Example-ISR_8_PWMs_Array_Complex)
* [Debug Terminal Output Samples](#debug-terminal-output-samples)
  * [1. ISR_8_PWMs_Array_Complex on SAM_DUE](#1-ISR_8_PWMs_Array_Complex-on-SAM_DUE)
  * [2. ISR_8_PWMs_Array on SAM_DUE](#2-isr_8_pwms_array-on-SAM_DUE)
  * [3. ISR_8_PWMs_Array_Simple on SAM_DUE](#3-ISR_8_PWMs_Array_Simple-on-SAM_DUE)
  * [4. ISR_Modify_PWM on SAM_DUE](#4-ISR_Modify_PWM-on-SAM_DUE)
  * [5. ISR_Changing_PWM on SAM_DUE](#5-ISR_Changing_PWM-on-SAM_DUE)
* [Debug](#debug)
* [Troubleshooting](#troubleshooting)
* [Issues](#issues)
* [TO DO](#to-do)
* [DONE](#done)
* [Contributions and Thanks](#contributions-and-thanks)
* [Contributing](#contributing)
* [License](#license)
* [Copyright](#copyright)

---
---

### Important Change from v1.2.0

Please have a look at [HOWTO Fix `Multiple Definitions` Linker Error](#howto-fix-multiple-definitions-linker-error)

As more complex calculation and check inside ISR are introduced from v1.2.0, there are consequences as follows

- using min 30uS and max 8 PWM channels for v1.2.0
- using min 20uS and max 8 PWM channels for v1.2.1

```
// Don't change these numbers to make higher Timer freq. System can hang
#define HW_TIMER_INTERVAL_US        30L
#define HW_TIMER_INTERVAL_FREQ      50000L
```

You certainly can modify to use better values according to your board and use-case, just remember to test and reverse to conservative values if crash happens.


### Why do we need this [SAMDUE_Slow_PWM library](https://github.com/khoih-prog/SAMDUE_Slow_PWM)

### Features

This library enables you to use ISR-based PWM channels on **SAM_DUE** boards, using [`Arduino SAM core`](https://github.com/arduino/ArduinoCore-sam), to create and output PWM any GPIO pin. Because this library doesn't use the powerful purely hardware-controlled PWM with many limitations, the maximum PWM frequency is currently limited at **1000Hz**, which is still suitable for many real-life applications. Now you can also modify PWM settings on-the-fly.

---

This library enables you to use Interrupt from Hardware Timers on **SAM_DUE** boards to create and output PWM to pins. It now supports 16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer. PWM interval can be very long (uint64_t microsecs / millisecs). The most important feature is they're ISR-based PWM channels. Therefore, their executions are not blocked by bad-behaving functions or tasks. This important feature is absolutely necessary for mission-critical tasks. These hardware PWM channels, using interrupt, still work even if other functions are blocking. Moreover, they are much more precise (certainly depending on clock frequency accuracy) than other software PWM using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

As **Hardware Timers are rare, and very precious assets** of any board, this library now enables you to use up to **16 ISR-based synchronized PWM channels, while consuming only 1 Hardware Timer**. Timers' interval is very long (**ulong millisecs**).

Now with these new **16 ISR-based PWM-channels**, the maximum interval is **practically unlimited** (limited only by unsigned long miliseconds) while **the accuracy is nearly perfect** compared to software PWM channels. 

The most important feature is they're ISR-based PWM channels. Therefore, their executions are **not blocked by bad-behaving functions / tasks**. This important feature is absolutely necessary for mission-critical tasks. 

The [**ISR_8_PWMs_Array_Complex**](examples/ISR_8_PWMs_Array_Complex) example will demonstrate the nearly perfect accuracy, compared to software PWM, by printing the actual period / duty-cycle in `microsecs` of each of PWM-channels.

Being ISR-based PWM, their executions are not blocked by bad-behaving functions / tasks, such as connecting to WiFi, Internet or Blynk services. You can also have many `(up to 16)` PWM channels to use.

This non-being-blocked important feature is absolutely necessary for mission-critical tasks.

You'll see `software-based` SimpleTimer is blocked while system is connecting to WiFi / Internet / Blynk, as well as by blocking task 
in loop(), using delay() function as an example. The elapsed time then is very unaccurate

---

#### Why using ISR-based PWM is better

Imagine you have a system with a **mission-critical** function, measuring water level and control the sump pump or doing something much more important. You normally use a software timer to poll, or even place the function in loop(). But what if another function is **blocking** the loop() or setup().

So your function **might not be executed, and the result would be disastrous.**

You'd prefer to have your function called, no matter what happening with other functions (busy loop, bug, etc.).

The correct choice is to use a Hardware Timer with **Interrupt** to call your function.

These hardware PWM channels, using interrupt, still work even if other functions are blocking. Moreover, they are much more **precise** (certainly depending on clock frequency accuracy) than other software PWM channels using millis() or micros(). That's necessary if you need to measure some data requiring better accuracy.

Functions using normal software PWM channels, relying on loop() and calling millis(), won't work if the loop() or setup() is blocked by certain operation. For example, certain function is blocking while it's connecting to WiFi or some services.

The catch is **your function is now part of an ISR (Interrupt Service Routine), and must be lean / mean, and follow certain rules.** More to read on:

[**HOWTO Attach Interrupt**](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)

---

### Currently supported Boards

1. **SAM_DUE** boards**, using [`Arduino SAM core`](https://github.com/arduino/ArduinoCore-sam)

---

#### Important Notes about ISR

1. Inside the attached function, **delay() won’t work and the value returned by millis() will not increment.** Serial data received while in the function may be lost. You should declare as **volatile any variables that you modify within the attached function.**

2. Typically global variables are used to pass data between an ISR and the main program. To make sure variables shared between an ISR and the main program are updated correctly, declare them as volatile.

---
---

## Prerequisites

 1. [`Arduino IDE 1.8.19+` for Arduino](https://github.com/arduino/Arduino). [![GitHub release](https://img.shields.io/github/release/arduino/Arduino.svg)](https://github.com/arduino/Arduino/releases/latest)
 2. [`Arduino SAM core v1.6.12+`](https://github.com/arduino/ArduinoCore-sam)
 
 3. To use with certain example
   - [`SimpleTimer library`](https://github.com/jfturcot/SimpleTimer) to use with some examples.
   
---
---

## Installation

### Use Arduino Library Manager

The best and easiest way is to use `Arduino Library Manager`. Search for [**SAMDUE_Slow_PWM**](https://github.com/khoih-prog/SAMDUE_Slow_PWM), then select / install the latest version.
You can also use this link [![arduino-library-badge](https://www.ardu-badge.com/badge/SAMDUE_Slow_PWM.svg?)](https://www.ardu-badge.com/SAMDUE_Slow_PWM) for more detailed instructions.

### Manual Install

Another way to install is to:

1. Navigate to [**SAMDUE_Slow_PWM**](https://github.com/khoih-prog/SAMDUE_Slow_PWM) page.
2. Download the latest release `SAMDUE_Slow_PWM-master.zip`.
3. Extract the zip file to `SAMDUE_Slow_PWM-master` directory 
4. Copy whole `SAMDUE_Slow_PWM-master` folder to Arduino libraries' directory such as `~/Arduino/libraries/`.

### VS Code & PlatformIO

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO](https://platformio.org/platformio-ide)
3. Install [**SAMDUE_Slow_PWM** library](https://platformio.org/lib/show/12896/SAMDUE_Slow_PWM) by using [Library Manager](https://platformio.org/lib/show/12896/SAMDUE_Slow_PWM/installation). Search for **SAMDUE_Slow_PWM** in [Platform.io Author's Libraries](https://platformio.org/lib/search?query=author:%22Khoi%20Hoang%22)
4. Use included [platformio.ini](platformio/platformio.ini) file from examples to ensure that all dependent libraries will installed automatically. Please visit documentation for the other options and examples at [Project Configuration File](https://docs.platformio.org/page/projectconf.html)


---
---

### HOWTO Fix `Multiple Definitions` Linker Error

The current library implementation, using `xyz-Impl.h` instead of standard `xyz.cpp`, possibly creates certain `Multiple Definitions` Linker error in certain use cases.

You can include this `.hpp` file

```
// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "SAMDUE_Slow_PWM.hpp"     //https://github.com/khoih-prog/SAMDUE_Slow_PWM
```

in many files. But be sure to use the following `.h` file **in just 1 `.h`, `.cpp` or `.ino` file**, which must **not be included in any other file**, to avoid `Multiple Definitions` Linker Error

```
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "SAMDUE_Slow_PWM.h"           //https://github.com/khoih-prog/SAMDUE_Slow_PWM
```

Check the new [**multiFileProject** example](examples/multiFileProject) for a `HOWTO` demo.

Have a look at the discussion in [Different behaviour using the src_cpp or src_h lib #80](https://github.com/khoih-prog/ESPAsync_WiFiManager/discussions/80)

---
---


## Usage


Before using any Timer, you have to make sure the Timer has not been used by any other purpose.


#### 1. Init ISR_PWM

```
// Init SAMDUE_Slow_PWM, each can service 16 different ISR-based PWM channels
SAMDUE_Slow_PWM ISR_PWM;
```

#### 2. Set PWM Frequency, dutycycle, attach irqCallbackStartFunc and irqCallbackStopFunc functions

```
void irqCallbackStartFunc()
{

}

void irqCallbackStopFunc()
{

}

////////////////////////////////////////////////

uint16_t attachDueInterrupt(double microseconds, timerCallback callback, const char* TimerName)
{
  DueTimerInterrupt dueTimerInterrupt = DueTimer.getAvailable();
  
  dueTimerInterrupt.attachInterruptInterval(microseconds, callback);

  uint16_t timerNumber = dueTimerInterrupt.getTimerNumber();
  
  Serial.print(TimerName); Serial.print(F(" attached to Timer(")); Serial.print(timerNumber); Serial.println(F(")"));

  return timerNumber;
}

////////////////////////////////////////////////

void setup()
{
  ....
  
  // Interval in microsecs
  attachDueInterrupt(HW_TIMER_INTERVAL_US, TimerHandler, "ITimer");
  
  // You can use this with PWM_Freq in Hz
  ISR_PWM.setPWM(PWM_Pin, PWM_Freq, PWM_DutyCycle, irqCallbackStartFunc, irqCallbackStopFunc);
                   
  ....                 
}  
```

---
---

### Examples: 

 1. [ISR_8_PWMs_Array](examples/ISR_8_PWMs_Array)
 2. [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)
 3. [ISR_8_PWMs_Array_Simple](examples/ISR_8_PWMs_Array_Simple)
 4. [ISR_Changing_PWM](examples/ISR_Changing_PWM)
 5. [ISR_Modify_PWM](examples/ISR_Modify_PWM)
 6. [multiFileProject](examples/multiFileProject). **New**

 
---
---

### Example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex)

```
#if !( defined(ARDUINO_SAM_DUE) || defined(__SAM3X8E__) )
  #error This is designed only for Arduino SAM_DUE board! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "ESP32_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_      3

// Default is true, uncomment to false
//#define CHANGING_PWM_END_OF_CYCLE     false

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "SAMDUE_Slow_PWM.h"

#include <SimpleTimer.h>              // https://github.com/jfturcot/SimpleTimer

#define LED_OFF             HIGH
#define LED_ON              LOW

//#ifndef LED_BUILTIN
//  #define LED_BUILTIN       13
//#endif

#ifndef LED_BLUE
  #define LED_BLUE          2
#endif

#ifndef LED_RED
  #define LED_RED           3
#endif

#define USING_HW_TIMER_INTERVAL_MS        false   //true

// Don't change these numbers to make higher Timer freq. System can hang
#define HW_TIMER_INTERVAL_US        20L
#define HW_TIMER_INTERVAL_FREQ      50000L

volatile uint32_t startMicros = 0;

// Init SAMDUE_Slow_PWM, each can service 16 different ISR-based PWM channels
SAMDUE_Slow_PWM ISR_PWM;

//////////////////////////////////////////////////////

void TimerHandler()
{
  ISR_PWM.run();
}

/////////////////////////////////////////////////

#define PIN_22      22
#define PIN_23      23
#define PIN_24      24
#define PIN_25      25
#define PIN_26      26
#define PIN_27      27
#define PIN_28      28

// You can assign pins here. Be careful to select good pin to use or crash, e.g pin 0-1
uint32_t PWM_Pin[] =
{
  LED_BUILTIN, PIN_22, PIN_23,  PIN_24,  PIN_25,  PIN_26,  PIN_26,  PIN_28
};

#define NUMBER_ISR_PWMS         ( sizeof(PWM_Pin) / sizeof(uint32_t) )

typedef void (*irqCallback)  ();

//////////////////////////////////////////////////////

#define USE_COMPLEX_STRUCT      true

//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

typedef struct
{
  uint32_t      PWM_Pin;
  irqCallback   irqCallbackStartFunc;
  irqCallback   irqCallbackStopFunc;

  float         PWM_Freq;

  float         PWM_DutyCycle;

  uint32_t      deltaMicrosStart;
  uint32_t      previousMicrosStart;

  uint32_t      deltaMicrosStop;
  uint32_t      previousMicrosStop;

} ISR_PWM_Data;

// In nRF52, avoid doing something fancy in ISR, for example Serial.print()
// The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
// Or you can get this run-time error / crash

void doingSomethingStart(int index);

void doingSomethingStop(int index);

#else   // #if USE_COMPLEX_STRUCT

volatile unsigned long deltaMicrosStart    [] = { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStart [] = { 0, 0, 0, 0, 0, 0, 0, 0 };

volatile unsigned long deltaMicrosStop     [] = { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile unsigned long previousMicrosStop  [] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// You can assign any interval for any timer here, in Hz
float PWM_Freq[] =
{
  1.0f,  2.0f,  3.0f,  5.0f,  10.0f,  20.0f,  30.0f,  50.0f
};

// You can assign any duty-cycle for any PWM channel here, in %
float PWM_DutyCycle[] =
{
  5.0, 10.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0
};

void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  deltaMicrosStart[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStart[index] = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  // Count from start to stop PWM pulse
  deltaMicrosStop[index]    = currentMicros - previousMicrosStart[index];
  previousMicrosStop[index] = currentMicros;
}

#endif    // #if USE_COMPLEX_STRUCT

////////////////////////////////////
// Shared
////////////////////////////////////

void doingSomethingStart0()
{
  doingSomethingStart(0);
}

void doingSomethingStart1()
{
  doingSomethingStart(1);
}

void doingSomethingStart2()
{
  doingSomethingStart(2);
}

void doingSomethingStart3()
{
  doingSomethingStart(3);
}

void doingSomethingStart4()
{
  doingSomethingStart(4);
}

void doingSomethingStart5()
{
  doingSomethingStart(5);
}

void doingSomethingStart6()
{
  doingSomethingStart(6);
}

void doingSomethingStart7()
{
  doingSomethingStart(7);
}


//////////////////////////////////////////////////////

void doingSomethingStop0()
{
  doingSomethingStop(0);
}

void doingSomethingStop1()
{
  doingSomethingStop(1);
}

void doingSomethingStop2()
{
  doingSomethingStop(2);
}

void doingSomethingStop3()
{
  doingSomethingStop(3);
}

void doingSomethingStop4()
{
  doingSomethingStop(4);
}

void doingSomethingStop5()
{
  doingSomethingStop(5);
}

void doingSomethingStop6()
{
  doingSomethingStop(6);
}

void doingSomethingStop7()
{
  doingSomethingStop(7);
}


//////////////////////////////////////////////////////

#if USE_COMPLEX_STRUCT

ISR_PWM_Data curISR_PWM_Data[] =
{
  // pin, irqCallbackStartFunc, irqCallbackStopFunc, PWM_Freq, PWM_DutyCycle, deltaMicrosStart, previousMicrosStart, deltaMicrosStop, previousMicrosStop
  { LED_BUILTIN,  doingSomethingStart0,    doingSomethingStop0,   1.0,   5.0, 0, 0, 0, 0 },
  { PIN_22,       doingSomethingStart1,    doingSomethingStop1,   2.0,  10.0, 0, 0, 0, 0 },
  { PIN_23,       doingSomethingStart2,    doingSomethingStop2,   3.0,  20.0, 0, 0, 0, 0 },
  { PIN_24,       doingSomethingStart3,    doingSomethingStop3,   5.0,  25.0, 0, 0, 0, 0 },
  { PIN_25,       doingSomethingStart4,    doingSomethingStop4,  10.0,  30.0, 0, 0, 0, 0 },
  { PIN_26,       doingSomethingStart5,    doingSomethingStop5,  20.0,  35.0, 0, 0, 0, 0 },
  { PIN_27,       doingSomethingStart6,    doingSomethingStop6,  30.0,  40.0, 0, 0, 0, 0 },
  { PIN_28,       doingSomethingStart7,    doingSomethingStop7,  50.0,  45.0, 0, 0, 0, 0 },
};


void doingSomethingStart(int index)
{
  unsigned long currentMicros  = micros();

  curISR_PWM_Data[index].deltaMicrosStart    = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStart = currentMicros;
}

void doingSomethingStop(int index)
{
  unsigned long currentMicros  = micros();

  //curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStop;
  // Count from start to stop PWM pulse
  curISR_PWM_Data[index].deltaMicrosStop     = currentMicros - curISR_PWM_Data[index].previousMicrosStart;
  curISR_PWM_Data[index].previousMicrosStop  = currentMicros;
}

#else   // #if USE_COMPLEX_STRUCT

irqCallback irqCallbackStartFunc[] =
{
  doingSomethingStart0,  doingSomethingStart1,  doingSomethingStart2,  doingSomethingStart3,
  doingSomethingStart4,  doingSomethingStart5,  doingSomethingStart6,  doingSomethingStart7
};

irqCallback irqCallbackStopFunc[] =
{
  doingSomethingStop0,  doingSomethingStop1,  doingSomethingStop2,  doingSomethingStop3,
  doingSomethingStop4,  doingSomethingStop5,  doingSomethingStop6,  doingSomethingStop7
};

#endif    // #if USE_COMPLEX_STRUCT

//////////////////////////////////////////////////////

#define SIMPLE_TIMER_MS        2000L

// Init SimpleTimer
SimpleTimer simpleTimer;

// Here is software Timer, you can do somewhat fancy stuffs without many issues.
// But always avoid
// 1. Long delay() it just doing nothing and pain-without-gain wasting CPU power.Plan and design your code / strategy ahead
// 2. Very long "do", "while", "for" loops without predetermined exit time.
void simpleTimerDoingSomething2s()
{
  static unsigned long previousMicrosStart = startMicros;

  unsigned long currMicros = micros();

  Serial.print(F("SimpleTimer (us): ")); Serial.print(SIMPLE_TIMER_MS);
  Serial.print(F(", us : ")); Serial.print(currMicros);
  Serial.print(F(", Dus : ")); Serial.println(currMicros - previousMicrosStart);

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    Serial.print(F("PWM Channel : ")); Serial.print(i);
    Serial.print(F(", prog Period (us): "));

    Serial.print(1000000 / curISR_PWM_Data[i].PWM_Freq);

    Serial.print(F(", actual : ")); Serial.print((uint32_t) curISR_PWM_Data[i].deltaMicrosStart);

    Serial.print(F(", prog DutyCycle : "));

    Serial.print(curISR_PWM_Data[i].PWM_DutyCycle);

    Serial.print(F(", actual : ")); Serial.println((float) curISR_PWM_Data[i].deltaMicrosStop * 100.0f / curISR_PWM_Data[i].deltaMicrosStart);
    //Serial.print(F(", actual deltaMicrosStop : ")); Serial.println(curISR_PWM_Data[i].deltaMicrosStop);
    //Serial.print(F(", actual deltaMicrosStart : ")); Serial.println(curISR_PWM_Data[i].deltaMicrosStart);

#else

    Serial.print(F("PWM Channel : ")); Serial.print(i);

    Serial.print(1000 / PWM_Freq[i]);

    Serial.print(F(", prog. Period (us): ")); Serial.print(PWM_Period[i]);
    Serial.print(F(", actual : ")); Serial.print(deltaMicrosStart[i]);

    Serial.print(F(", prog DutyCycle : "));

    Serial.print(PWM_DutyCycle[i]);

    Serial.print(F(", actual : ")); Serial.println( (float) deltaMicrosStop[i] * 100.0f / deltaMicrosStart[i]);
    //Serial.print(F(", actual deltaMicrosStop : ")); Serial.println(deltaMicrosStop[i]);
    //Serial.print(F(", actual deltaMicrosStart : ")); Serial.println(deltaMicrosStart[i]);
#endif
  }

  previousMicrosStart = currMicros;
}

////////////////////////////////////////////////

uint16_t attachDueInterrupt(double microseconds, timerCallback callback, const char* TimerName)
{
  DueTimerInterrupt dueTimerInterrupt = DueTimer.getAvailable();
  
  dueTimerInterrupt.attachInterruptInterval(microseconds, callback);

  uint16_t timerNumber = dueTimerInterrupt.getTimerNumber();
  
  Serial.print(TimerName); Serial.print(F(" attached to Timer(")); Serial.print(timerNumber); Serial.println(F(")"));

  return timerNumber;
}

////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(2000);

  Serial.print(F("\nStarting ISR_8_PWMs_Array_Complex on ")); Serial.println(BOARD_NAME);
  Serial.println(SAMDUE_SLOW_PWM_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));
  Serial.print(F("Timer Frequency = ")); Serial.print(SystemCoreClock / 1000000); Serial.println(F(" MHz"));

  // Interval in microsecs
  attachDueInterrupt(HW_TIMER_INTERVAL_US, TimerHandler, "ITimer");

  startMicros = micros();

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_PWM

  for (uint16_t i = 0; i < NUMBER_ISR_PWMS; i++)
  {
#if USE_COMPLEX_STRUCT
    curISR_PWM_Data[i].previousMicrosStart = startMicros;
    //ISR_PWM.setInterval(curISR_PWM_Data[i].PWM_Period, curISR_PWM_Data[i].irqCallbackStartFunc);

    //void setPWM(uint32_t pin, float frequency, float dutycycle
    // , timer_callback_p StartCallback = nullptr, timer_callback_p StopCallback = nullptr)

    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(curISR_PWM_Data[i].PWM_Pin, curISR_PWM_Data[i].PWM_Freq, curISR_PWM_Data[i].PWM_DutyCycle,
                   curISR_PWM_Data[i].irqCallbackStartFunc, curISR_PWM_Data[i].irqCallbackStopFunc);

#else
    previousMicrosStart[i] = micros();

    // You can use this with PWM_Freq in Hz
    ISR_PWM.setPWM(PWM_Pin[i], PWM_Freq[i], PWM_DutyCycle[i], irqCallbackStartFunc[i], irqCallbackStopFunc[i]);

#endif
  }

  // You need this timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary.
  simpleTimer.setInterval(SIMPLE_TIMER_MS, simpleTimerDoingSomething2s);
}

#define BLOCKING_TIME_MS      10000L

void loop()
{
  // This unadvised blocking task is used to demonstrate the blocking effects onto the execution and accuracy to Software timer
  // You see the time elapse of ISR_PWM still accurate, whereas very unaccurate for Software Timer
  // The time elapse for 2000ms software timer now becomes 3000ms (BLOCKING_TIME_MS)
  // While that of ISR_PWM is still prefect.
  delay(BLOCKING_TIME_MS);

  // You need this Software timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary
  // You don't need to and never call ISR_PWM.run() here in the loop(). It's already handled by ISR timer.
  simpleTimer.run();
}
```
---
---

### Debug Terminal Output Samples

### 1. ISR_8_PWMs_Array_Complex on SAM_DUE

The following is the sample terminal output when running example [ISR_8_PWMs_Array_Complex](examples/ISR_8_PWMs_Array_Complex) to demonstrate how to use multiple PWM channels with complex callback functions, the accuracy of ISR Hardware PWM-channels, **especially when system is very busy**.  The ISR PWM-channels is **running exactly according to corresponding programmed periods and duty-cycles**


```
Starting ISR_8_PWMs_Array_Complex on SAM_DUE
SAMDUE_Slow_PWM v1.2.1
CPU Frequency = 84 MHz
Timer Frequency = 84 MHz
[PWM] Using Timer( 0 ) = TC0
[PWM] Channel = 0 , IRQ = TC0_IRQn
ITimer attached to Timer(0)
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 2009552
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 2015589
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 2021600
Channel : 3	    Period : 200000		OnTime : 50000	Start_Time : 2027591
Channel : 4	    Period : 100000		OnTime : 30000	Start_Time : 2033591
Channel : 5	    Period : 50000		OnTime : 17500	Start_Time : 2039592
Channel : 6	    Period : 33333		OnTime : 13333	Start_Time : 2045521
Channel : 7	    Period : 20000		OnTime : 9000	Start_Time : 2051429
SimpleTimer (us): 2000, us : 12057006, Dus : 10047524
PWM Channel : 0, prog Period (us): 1000000.00, actual : 1000000, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (us): 500000.00, actual : 500000, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (us): 333333.33, actual : 333339, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (us): 200000.00, actual : 199999, prog DutyCycle : 25.00, actual : 25.00
PWM Channel : 4, prog Period (us): 100000.00, actual : 99999, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 5, prog Period (us): 50000.00, actual : 50001, prog DutyCycle : 35.00, actual : 35.00
PWM Channel : 6, prog Period (us): 33333.33, actual : 33341, prog DutyCycle : 40.00, actual : 39.95
PWM Channel : 7, prog Period (us): 20000.00, actual : 20000, prog DutyCycle : 45.00, actual : 45.00
SimpleTimer (us): 2000, us : 22120008, Dus : 10063002
PWM Channel : 0, prog Period (us): 1000000.00, actual : 1000000, prog DutyCycle : 5.00, actual : 5.00
PWM Channel : 1, prog Period (us): 500000.00, actual : 500000, prog DutyCycle : 10.00, actual : 10.00
PWM Channel : 2, prog Period (us): 333333.33, actual : 333339, prog DutyCycle : 20.00, actual : 20.00
PWM Channel : 3, prog Period (us): 200000.00, actual : 200000, prog DutyCycle : 25.00, actual : 25.00
PWM Channel : 4, prog Period (us): 100000.00, actual : 100001, prog DutyCycle : 30.00, actual : 30.00
PWM Channel : 5, prog Period (us): 50000.00, actual : 49999, prog DutyCycle : 35.00, actual : 35.00
PWM Channel : 6, prog Period (us): 33333.33, actual : 33340, prog DutyCycle : 40.00, actual : 39.95
PWM Channel : 7, prog Period (us): 20000.00, actual : 20001, prog DutyCycle : 45.00, actual : 45.00
```

---

### 2. ISR_8_PWMs_Array on SAM_DUE

The following is the sample terminal output when running example [**ISR_8_PWMs_Array**](examples/ISR_8_PWMs_Array) on **SAM_DUE** to demonstrate how to use multiple PWM channels with simple callback functions.

```
Starting ISR_8_PWMs_Array on SAM_DUE
SAMDUE_Slow_PWM v1.2.1
CPU Frequency = 84 MHz
Timer Frequency = 84 MHz
[PWM] Using Timer( 0 ) = TC0
[PWM] Channel = 0 , IRQ = TC0_IRQn
ITimer attached to Timer(0)
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 2008858
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 2014903
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 2020913
Channel : 3	    Period : 200000		OnTime : 50000	Start_Time : 2026912
Channel : 4	    Period : 100000		OnTime : 30000	Start_Time : 2032913
Channel : 5	    Period : 50000		OnTime : 17500	Start_Time : 2038913
Channel : 6	    Period : 33333		OnTime : 13333	Start_Time : 2044832
Channel : 7	    Period : 20000		OnTime : 9000	Start_Time : 2050743
```

---

### 3. ISR_8_PWMs_Array_Simple on SAM_DUE

The following is the sample terminal output when running example [**ISR_8_PWMs_Array_Simple**](examples/ISR_8_PWMs_Array_Simple) on **SAM_DUE** to demonstrate how to use multiple PWM channels.

```
Starting ISR_8_PWMs_Array_Simple on SAM_DUE
SAMDUE_Slow_PWM v1.2.1
CPU Frequency = 84 MHz
Timer Frequency = 84 MHz
[PWM] Using Timer( 0 ) = TC0
[PWM] Channel = 0 , IRQ = TC0_IRQn
ITimer attached to Timer(0)
Channel : 0	    Period : 1000000		OnTime : 50000	Start_Time : 2009460
Channel : 1	    Period : 500000		OnTime : 50000	Start_Time : 2015503
Channel : 2	    Period : 333333		OnTime : 66666	Start_Time : 2021514
Channel : 3	    Period : 200000		OnTime : 50000	Start_Time : 2027505
Channel : 4	    Period : 100000		OnTime : 30000	Start_Time : 2033505
Channel : 5	    Period : 50000		OnTime : 17500	Start_Time : 2039514
Channel : 6	    Period : 33333		OnTime : 13333	Start_Time : 2045434
Channel : 7	    Period : 20000		OnTime : 9000	Start_Time : 2051343
```

---

### 4. ISR_Modify_PWM on SAM_DUE

The following is the sample terminal output when running example [ISR_Modify_PWM](examples/ISR_Modify_PWM) on **SAM_DUE** to demonstrate how to modify PWM settings on-the-fly without deleting the PWM channel

```
Starting ISR_Modify_PWM on SAM_DUE
SAMDUE_Slow_PWM v1.2.1
CPU Frequency = 84 MHz
Timer Frequency = 84 MHz
[PWM] Using Timer( 0 ) = TC0
[PWM] Channel = 0 , IRQ = TC0_IRQn
ITimer attached to Timer(0)
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 2012628
Channel : 0	New Period : 500000		OnTime : 450000	Start_Time : 12012634
Channel : 0	New Period : 1000000		OnTime : 500000	Start_Time : 22012634
Channel : 0	New Period : 500000		OnTime : 450000	Start_Time : 31512634
Channel : 0	New Period : 1000000		OnTime : 500000	Start_Time : 42012634
Channel : 0	New Period : 500000		OnTime : 450000	Start_Time : 51512634
```

---

### 5. ISR_Changing_PWM on SAM_DUE

The following is the sample terminal output when running example [ISR_Changing_PWM](examples/ISR_Changing_PWM) on **SAM_DUE** to demonstrate how to modify PWM settings on-the-fly by deleting the PWM channel and reinit the PWM channel

```
Starting ISR_Changing_PWM on SAM_DUE
SAMDUE_Slow_PWM v1.2.1
CPU Frequency = 84 MHz
Timer Frequency = 84 MHz
[PWM] Using Timer( 0 ) = TC0
[PWM] Channel = 0 , IRQ = TC0_IRQn
ITimer attached to Timer(0)
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 2012800
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 12018300
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 22018303
Using PWM Freq = 2.00, PWM DutyCycle = 90.00
Channel : 0	    Period : 500000		OnTime : 450000	Start_Time : 32018301
Using PWM Freq = 1.00, PWM DutyCycle = 50.00
Channel : 0	    Period : 1000000		OnTime : 500000	Start_Time : 42018304
```

---
---

### Debug

Debug is enabled by default on Serial.

You can also change the debugging level `_PWM_LOGLEVEL_` from 0 to 4

```cpp
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_     0
```

---

### Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.


---
---

### Issues

Submit issues to: [SAMDUE_Slow_PWM issues](https://github.com/khoih-prog/SAMDUE_Slow_PWM/issues)

---

## TO DO

1. Search for bug and improvement.
2. Similar features for remaining Arduino boards

---

## DONE

1. Basic hardware multi-channel PWM for **SAM_DUE**, etc. using [`Arduino SAM core`](https://github.com/arduino/ArduinoCore-sam)
2. Add Table of Contents
3. Add functions to modify PWM settings on-the-fly
4. Fix `multiple-definitions` linker error. Drop `src_cpp` and `src_h` directories
5. DutyCycle to be optionally updated at the end current PWM period instead of immediately.
6. Add examples [multiFileProject](examples/multiFileProject) to demo for multiple-file project
7. Improve accuracy by using `float`, instead of `uint32_t` for `dutycycle`
8. Optimize library code by using `reference-passing` instead of `value-passing`

---
---

### Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.


---

## Contributing

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

### License

- The library is licensed under [MIT](https://github.com/khoih-prog/SAMDUE_Slow_PWM/blob/main/LICENSE)

---

## Copyright

Copyright 2021- Khoi Hoang


