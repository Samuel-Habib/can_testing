#include "cmsis_os2.h"
#include "main.h"

extern const osThreadAttr_t watchdog_attributes;
void Watchdog_Task(void *argument);
