#ifndef MOCK_CMSIS_OS2_H
#define MOCK_CMSIS_OS2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum {
    osPriorityNone          = 0,
    osPriorityIdle          = 1,
    osPriorityLow           = 8,
    osPriorityBelowNormal   = 16,
    osPriorityNormal        = 24,
    osPriorityAboveNormal   = 32,
    osPriorityHigh          = 40,
    osPriorityRealtime      = 48,
    osPriorityISR           = 56,
    osPriorityError         = -1,
    osPriorityReserved      = 0x7FFFFFFF
} osPriority_t;

typedef struct {
    const char                   *name;
    uint32_t                      attr_bits;
    void                         *cb_mem;
    uint32_t                      cb_size;
    void                         *stack_mem;
    uint32_t                      stack_size;
    osPriority_t                  priority;
    uint32_t                      tz_module;
    uint32_t                      reserved;
} osThreadAttr_t;

typedef void * osThreadId_t;

typedef enum {
    osOK                    =  0,
    osError                 = -1,
    osErrorTimeout          = -2,
    osErrorResource         = -3,
    osErrorParameter        = -4,
    osErrorNoMemory         = -5,
    osErrorISR              = -6,
    osStatusReserved        = 0x7FFFFFFF
} osStatus_t;

osStatus_t osDelay(uint32_t ticks);
uint32_t osKernelGetTickCount(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_CMSIS_OS2_H */
