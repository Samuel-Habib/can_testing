#ifndef MOCK_FREERTOS_H
#define MOCK_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "projdefs.h"
#include "portmacro.h"

typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t)(xTimeInMs))

#include "task.h"

#ifdef __cplusplus
}
#endif

#endif /* MOCK_FREERTOS_H */
