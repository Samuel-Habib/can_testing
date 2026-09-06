#include "cmsis_os2.h"
#include "main.h"
#include "math.h"
#include "projdefs.h"
#include "stm32h7xx_hal_gpio.h"
#include <limits.h>
#include <stdio.h>

extern const osThreadAttr_t battery_attributes;
void Battery_Task(void *argument);
bool run_battery_task(volatile unsigned int *curr_sensor_readings);
void reset_battery_current_test(void);
void battery_test_debug(void);
