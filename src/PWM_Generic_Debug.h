/****************************************************************************************************************************
  PWM_Generic_Debug.h
  For Arduino SAM_DUE boards
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/SAMDUE_Slow_PWM
  Licensed under MIT license

  Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
  unsigned long miliseconds), you just consume only one megaAVR-based timer and avoid conflicting with other cores' tasks.
  The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
  Therefore, their executions are not blocked by bad-behaving functions / tasks.
  This important feature is absolutely necessary for mission-critical tasks.

  Version: 1.2.2

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K.Hoang      29/09/2021 Initial coding for Arduino SAM_DUE
  1.1.0   K Hoang      10/11/2021 Add functions to modify PWM settings on-the-fly
  1.2.0   K Hoang      31/01/2022 Fix multiple-definitions linker error. Improve accuracy. Change DutyCycle update
  1.2.1   K Hoang      01/02/2022 Use float for DutyCycle and Freq, uint32_t for period. Optimize code
  1.2.2   K Hoang      04/03/2022 Fix `DutyCycle` and `New Period` display bugs. Display warning only when debug level > 3
 *****************************************************************************************************************************/

#pragma once

#ifndef PWM_GENERIC_DEBUG_H
#define PWM_GENERIC_DEBUG_H

#ifdef PWM_GENERIC_DEBUG_PORT
  #define PWM_DBG_PORT      PWM_GENERIC_DEBUG_PORT
#else
  #define PWM_DBG_PORT      Serial
#endif

// Change _PWM_LOGLEVEL_ to set tracing and logging verbosity
// 0: DISABLED: no logging
// 1: ERROR: errors
// 2: WARN: errors and warnings
// 3: INFO: errors, warnings and informational (default)
// 4: DEBUG: errors, warnings, informational and debug

#ifndef _PWM_LOGLEVEL_
  #define _PWM_LOGLEVEL_       1
#endif

///////////////////////////////////////

const char PWM_MARK[]  = "[PWM] ";
const char PWM_SPACE[] = " ";

#define PWM_PRINT        PWM_DBG_PORT.print
#define PWM_PRINTLN      PWM_DBG_PORT.println

#define PWM_PRINT_MARK   PWM_PRINT(PWM_MARK)
#define PWM_PRINT_SP     PWM_PRINT(PWM_SPACE)
#define PWM_PRINT_LINE   PWM_PRINT(PWM_LINE)

///////////////////////////////////////

#define PWM_LOGERROR(x)         if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGERROR0(x)        if(_PWM_LOGLEVEL_>0) { PWM_PRINT(x); }
#define PWM_LOGERRORLN0(x)      if(_PWM_LOGLEVEL_>0) { PWM_PRINTLN(x); }
#define PWM_LOGERROR1(x,y)      if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGERROR2(x,y,z)    if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGERROR3(x,y,z,w)  if(_PWM_LOGLEVEL_>0) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGWARN(x)          if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGWARN0(x)         if(_PWM_LOGLEVEL_>1) { PWM_PRINT(x); }
#define PWM_LOGWARNLN0(x)       if(_PWM_LOGLEVEL_>1) { PWM_PRINTLN(x); }
#define PWM_LOGWARN1(x,y)       if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGWARN2(x,y,z)     if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGWARN3(x,y,z,w)   if(_PWM_LOGLEVEL_>1) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGINFO(x)          if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGINFO0(x)         if(_PWM_LOGLEVEL_>2) { PWM_PRINT(x); }
#define PWM_LOGINFOLN0(x)       if(_PWM_LOGLEVEL_>2) { PWM_PRINTLN(x); }
#define PWM_LOGINFO1(x,y)       if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGINFO2(x,y,z)     if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGINFO3(x,y,z,w)   if(_PWM_LOGLEVEL_>2) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#define PWM_LOGDEBUG(x)         if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINTLN(x); }
#define PWM_LOGDEBUG0(x)        if(_PWM_LOGLEVEL_>3) { PWM_PRINT(x); }
#define PWM_LOGDEBUGLN0(x)      if(_PWM_LOGLEVEL_>3) { PWM_PRINTLN(x); }
#define PWM_LOGDEBUG1(x,y)      if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINTLN(y); }
#define PWM_LOGDEBUG2(x,y,z)    if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINTLN(z); }
#define PWM_LOGDEBUG3(x,y,z,w)  if(_PWM_LOGLEVEL_>3) { PWM_PRINT_MARK; PWM_PRINT(x); PWM_PRINT_SP; PWM_PRINT(y); PWM_PRINT_SP; PWM_PRINT(z); PWM_PRINT_SP; PWM_PRINTLN(w); }

///////////////////////////////////////

#endif    //PWM_GENERIC_DEBUG_H
