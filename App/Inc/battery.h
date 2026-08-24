#include "cmsis_os2.h"
#include "main.h"
#include "math.h"
#include <limits.h>

extern const osThreadAttr_t battery_attributes;
void Battery_Task(void *argument);
