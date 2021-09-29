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

  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K.Hoang      29/09/2021 Initial coding for Arduino SAM_DUE
*****************************************************************************************************************************/

#pragma once

#ifndef SAMDUE_SLOW_PWM_ISR_HPP
#define SAMDUE_SLOW_PWM_ISR_HPP

#include <string.h>

/////////////////////////////////////////////////// 


uint32_t timeNow()
{
  return ( (uint32_t) micros() );
}
 
/////////////////////////////////////////////////// 

SAMDUE_SLOW_PWM_ISR::SAMDUE_SLOW_PWM_ISR()
  : numChannels (-1)
{
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::init() 
{
  uint32_t currentTime = timeNow();
   
  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    memset((void*) &SAM_DUE_PWM[channelNum], 0, sizeof (PWM_t));
    SAM_DUE_PWM[channelNum].prevTime = currentTime;
    SAM_DUE_PWM[channelNum].pin      = INVALID_SAM_DUE_PIN;
  }
  
  numChannels = 0;
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::run() 
{    
  uint32_t currentTime = timeNow();
  
  noInterrupts();

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {   
    // If enabled => check
    // start period / dutyCycle => digitalWrite HIGH
    // end dutyCycle =>  digitalWrite LOW
    if (SAM_DUE_PWM[channelNum].enabled) 
    {
      if ( (uint32_t) (currentTime - SAM_DUE_PWM[channelNum].prevTime) <= SAM_DUE_PWM[channelNum].onTime ) 
      {              
        if (!SAM_DUE_PWM[channelNum].pinHigh)
        {
          digitalWrite(SAM_DUE_PWM[channelNum].pin, HIGH);
          SAM_DUE_PWM[channelNum].pinHigh = true;
          
          // callbackStart
          if (SAM_DUE_PWM[channelNum].callbackStart != nullptr)
          {
            (*(timer_callback) SAM_DUE_PWM[channelNum].callbackStart)();
          }
        }
      }
      else if ( (uint32_t) (currentTime - SAM_DUE_PWM[channelNum].prevTime) < SAM_DUE_PWM[channelNum].period ) 
      {
        if (SAM_DUE_PWM[channelNum].pinHigh)
        {
          digitalWrite(SAM_DUE_PWM[channelNum].pin, LOW);
          SAM_DUE_PWM[channelNum].pinHigh = false;
          
          // callback when PWM pulse stops (LOW)
          if (SAM_DUE_PWM[channelNum].callbackStop != nullptr)
          {
            (*(timer_callback) SAM_DUE_PWM[channelNum].callbackStop)();
          }
        }
      }
      else 
      //else if ( (uint32_t) (currentTime - SAM_DUE_PWM[channelNum].prevTime) >= SAM_DUE_PWM[channelNum].period )   
      {
        SAM_DUE_PWM[channelNum].prevTime = currentTime;
      }      
    }
  }
  
  interrupts();
}


///////////////////////////////////////////////////

// find the first available slot
// return -1 if none found
int SAMDUE_SLOW_PWM_ISR::findFirstFreeSlot() 
{
  // all slots are used
  if (numChannels >= MAX_NUMBER_CHANNELS) 
  {
    return -1;
  }

  // return the first slot with no callbackStart (i.e. free)
  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if ( (SAM_DUE_PWM[channelNum].period == 0) && !SAM_DUE_PWM[channelNum].enabled )
    {
      return channelNum;
    }
  }

  // no free slots found
  return -1;
}

///////////////////////////////////////////////////

int SAMDUE_SLOW_PWM_ISR::setupPWMChannel(uint32_t pin, uint32_t period, uint32_t dutycycle, void* cbStartFunc, void* cbStopFunc)
{
  int channelNum;
  
  // Invalid input, such as period = 0, etc
  if ( (period == 0) || (dutycycle > 100) )
  {
    PWM_LOGERROR(F("Error: Invalid period or dutycycle"));
    return -1;
  }

  if (numChannels < 0) 
  {
    init();
  }
 
  channelNum = findFirstFreeSlot();
  
  if (channelNum < 0) 
  {
    return -1;
  }

  SAM_DUE_PWM[channelNum].pin           = pin;
  SAM_DUE_PWM[channelNum].period        = period;
  SAM_DUE_PWM[channelNum].onTime        = ( period * dutycycle ) / 100;
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  SAM_DUE_PWM[channelNum].pinHigh       = true;
  
  SAM_DUE_PWM[channelNum].callbackStart = cbStartFunc;
  SAM_DUE_PWM[channelNum].callbackStop  = cbStopFunc;
  
  PWM_LOGINFO0(F("Channel : ")); PWM_LOGINFO0(channelNum); 
  PWM_LOGINFO0(F("\tPeriod : ")); PWM_LOGINFO0(SAM_DUE_PWM[channelNum].period);
  PWM_LOGINFO0(F("\t\tOnTime : ")); PWM_LOGINFO0(SAM_DUE_PWM[channelNum].onTime);
  PWM_LOGINFO0(F("\tStart_Time : ")); PWM_LOGINFOLN0(SAM_DUE_PWM[channelNum].prevTime);
 
  numChannels++;
  
  SAM_DUE_PWM[channelNum].enabled      = true;
  
  return channelNum;
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::deleteChannel(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  // nothing to delete if no timers are in use
  if (numChannels == 0) 
  {
    return;
  }

  // don't decrease the number of timers if the specified slot is already empty (zero period, invalid)
  if ( (SAM_DUE_PWM[channelNum].pin != INVALID_SAM_DUE_PIN) && (SAM_DUE_PWM[channelNum].period != 0) )
  {
    memset((void*) &SAM_DUE_PWM[channelNum], 0, sizeof (PWM_t));
    
    SAM_DUE_PWM[channelNum].pin = INVALID_SAM_DUE_PIN;
    
    // update number of timers
    numChannels--;
  }
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::restartChannel(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  SAM_DUE_PWM[channelNum].prevTime = timeNow();
}

///////////////////////////////////////////////////

bool SAMDUE_SLOW_PWM_ISR::isEnabled(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return false;
  }

  return SAM_DUE_PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::enable(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  SAM_DUE_PWM[channelNum].enabled = true;
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::disable(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  SAM_DUE_PWM[channelNum].enabled = false;
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::enableAll() 
{
  // Enable all timers with a callbackStart assigned (used)

  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (SAM_DUE_PWM[channelNum].period != 0)
    {
      SAM_DUE_PWM[channelNum].enabled = true;
    }
  }
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::disableAll() 
{
  // Disable all timers with a callbackStart assigned (used)
  for (uint8_t channelNum = 0; channelNum < MAX_NUMBER_CHANNELS; channelNum++) 
  {
    if (SAM_DUE_PWM[channelNum].period != 0)
    {
      SAM_DUE_PWM[channelNum].enabled = false;
    }
  }
}

///////////////////////////////////////////////////

void SAMDUE_SLOW_PWM_ISR::toggle(unsigned channelNum) 
{
  if (channelNum >= MAX_NUMBER_CHANNELS) 
  {
    return;
  }

  SAM_DUE_PWM[channelNum].enabled = !SAM_DUE_PWM[channelNum].enabled;
}

///////////////////////////////////////////////////

unsigned SAMDUE_SLOW_PWM_ISR::getnumChannels() 
{
  return numChannels;
}

#endif    // SAMDUE_SLOW_PWM_ISR_HPP

