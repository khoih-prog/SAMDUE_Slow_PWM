/****************************************************************************************************************************
  SAMDUE_Slow_PWM_ISR.hpp
  For Arduino SAM_DUE boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/SAMDUE_Slow_PWM
  Licensed under MIT license
  
  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one megaAVR-based timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.2.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K.Hoang      29/09/2021 Initial coding for Arduino SAM_DUE
  1.1.0   K Hoang      10/11/2021 Add functions to modify PWM settings on-the-fly
  1.2.0   K Hoang      31/01/2022 Fix multiple-definitions linker error. Improve accuracy. Change DutyCycle update
*****************************************************************************************************************************/

#pragma once

#ifndef SAMDUE_SLOW_PWM_ISR_HPP
#define SAMDUE_SLOW_PWM_ISR_HPP

#if ( defined(ARDUINO_SAM_DUE) || defined(__SAM3X8E__) )
  #if !defined(BOARD_NAME)
    #define BOARD_NAME      "SAM_DUE"
  #endif
#else
  #error This is designed only for Arduino megaAVR board! Please check your Tools->Board setting.
#endif

#ifndef SAMDUE_SLOW_PWM_VERSION
  #define SAMDUE_SLOW_PWM_VERSION           F("SAMDUE_Slow_PWM v1.2.0")
  
  #define SAMDUE_SLOW_PWM_VERSION_MAJOR     1
  #define SAMDUE_SLOW_PWM_VERSION_MINOR     2
  #define SAMDUE_SLOW_PWM_VERSION_PATCH     0

  #define SAMDUE_SLOW_PWM_VERSION_INT       1002000
#endif

#ifndef _PWM_LOGLEVEL_
  #define _PWM_LOGLEVEL_        1
#endif

#include "PWM_Generic_Debug.h"

#include <stddef.h>

#include <inttypes.h>

#if defined(ARDUINO)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
#endif

#define SAMDUE_SLOW_PWM_ISR  SAMDUE_Slow_PWM

typedef void (*timer_callback)();
typedef void (*timer_callback_p)(void *);

#if !defined(CHANGING_PWM_END_OF_CYCLE)
  #warning Using default CHANGING_PWM_END_OF_CYCLE == true
  #define CHANGING_PWM_END_OF_CYCLE     true
#endif

#define INVALID_SAM_DUE_PIN         255

//////////////////////////////////////////////////////////////////

class SAMDUE_SLOW_PWM_ISR 
{

  public:
    // maximum number of PWM channels
#define MAX_NUMBER_CHANNELS        16

    // constructor
    SAMDUE_SLOW_PWM_ISR();

    void init();

    // this function must be called inside loop()
    void run();
    
    //////////////////////////////////////////////////////////////////
    // PWM
    // Return the channelNum if OK, -1 if error
    int setPWM(const uint32_t& pin, const double& frequency, const double& dutycycle, timer_callback StartCallback = nullptr, 
                timer_callback StopCallback = nullptr)
    {
      double period = 0.0;

      if ( ( frequency > 0.0 ) && ( frequency <= 1000.0 ) )
      {
        // period in us
        period = 1000000.0f / frequency;
      }
      else
      {       
        PWM_LOGERROR("Error: Invalid frequency, max is 1000Hz");
        
        return -1;
      }
      
      return setupPWMChannel(pin, period, dutycycle, (void *) StartCallback, (void *) StopCallback);   
    }

    // period in us
    // Return the channelNum if OK, -1 if error
    int setPWM_Period(const uint32_t& pin, const double& period, const double& dutycycle, 
                         timer_callback StartCallback = nullptr, timer_callback StopCallback = nullptr)  
    {     
      return setupPWMChannel(pin, period, dutycycle, (void *) StartCallback, (void *) StopCallback);      
    } 
    
    //////////////////////////////////////////////////////////////////
    
    // low level function to modify a PWM channel
    // returns the true on success or false on failure
    bool modifyPWMChannel(const uint8_t& channelNum, const uint32_t& pin, const double& frequency, const double& dutycycle)
    {
      double period = 0.0;
      
      if ( ( frequency > 0.0 ) && ( frequency <= 1000.0 ) )
      {
        // period in us
        period = 1000000.0f / frequency;
      }
      else
      {       
        PWM_LOGERROR("Error: Invalid frequency, max is 1000Hz");
        return false;
      }
      
      return modifyPWMChannel_Period(channelNum, pin, period, dutycycle);
    }
    
    //////////////////////////////////////////////////////////////////
    
    //period in us
    bool modifyPWMChannel_Period(const uint8_t& channelNum, const uint32_t& pin, const double& period, const double& dutycycle);
    
    //////////////////////////////////////////////////////////////////

    // destroy the specified PWM channel
    void deleteChannel(const uint8_t& channelNum);

    // restart the specified PWM channel
    void restartChannel(const uint8_t& channelNum);

    // returns true if the specified PWM channel is enabled
    bool isEnabled(const uint8_t& channelNum);

    // enables the specified PWM channel
    void enable(const uint8_t& channelNum);

    // disables the specified PWM channel
    void disable(const uint8_t& channelNum);

    // enables all PWM channels
    void enableAll();

    // disables all PWM channels
    void disableAll();

    // enables the specified PWM channel if it's currently disabled, and vice-versa
    void toggle(const uint8_t& channelNum);

    // returns the number of used PWM channels
    int getnumChannels();

    // returns the number of available PWM channels
    uint8_t getNumAvailablePWMChannels() 
    {
      if (numChannels <= 0)
        return MAX_NUMBER_CHANNELS;
      else 
        return MAX_NUMBER_CHANNELS - numChannels;
    };

  private:

    // low level function to initialize and enable a new PWM channel
    // returns the PWM channel number (channelNum) on success or
    // -1 on failure (f == NULL) or no free PWM channels 
    int setupPWMChannel(const uint32_t& pin, const double& period, const double& dutycycle, 
                           void* cbStartFunc = nullptr, void* cbStopFunc = nullptr);

    // find the first available slot
    int findFirstFreeSlot();

    typedef struct 
    {
      ///////////////////////////////////
      
      
      ///////////////////////////////////
      
      uint32_t      prevTime;           // value returned by the micros() or millis() function in the previous run() call
      double        period;             // period value, in us / ms
      uint32_t      onTime;             // onTime value, ( period * dutyCycle / 100 ) us  / ms
      
      void*         callbackStart;      // pointer to the callback function when PWM pulse starts (HIGH)
      void*         callbackStop;       // pointer to the callback function when PWM pulse stops (LOW)
      
      ////////////////////////////////////////////////////////////
      
      uint32_t      pin;                // PWM pin
      bool          pinHigh;            // true if PWM pin is HIGH
      ////////////////////////////////////////////////////////////
      
      bool          enabled;            // true if enabled
      
      // New from v1.2.0     
      double        newPeriod;          // period value, in us / ms
      double        newDutyCycle;       // from 0.00 to 100.00, double precision
      //////
    } PWM_t;

    volatile PWM_t SAM_DUE_PWM[MAX_NUMBER_CHANNELS];

    // actual number of PWM channels in use (-1 means uninitialized)
    volatile int numChannels;
};

#endif    // SAMDUE_SLOW_PWM_ISR_HPP


