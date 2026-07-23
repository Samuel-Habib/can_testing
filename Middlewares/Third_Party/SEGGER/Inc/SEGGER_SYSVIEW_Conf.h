#ifndef SEGGER_SYSVIEW_CONF_H
#define SEGGER_SYSVIEW_CONF_H

#include "FreeRTOS.h"
#include "stm32h7xx.h"

#define SEGGER_SYSVIEW_CORE                     SEGGER_SYSVIEW_CORE_CM3
#define SEGGER_SYSVIEW_GET_INTERRUPT_ID()      ((__get_IPSR()) & 0x1FF)
#define SEGGER_SYSVIEW_GET_TIMESTAMP()         (DWT->CYCCNT)
#define SEGGER_SYSVIEW_TIMESTAMP_BITS          32

#define SEGGER_SYSVIEW_MAX_ARGUMENTS            4
#define SEGGER_SYSVIEW_MAX_COMM_BYTES          1024

#endif
