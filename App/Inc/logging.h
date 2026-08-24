#include "main.h"

signed int log_module(char data[]);
int uart_driver(void);

extern const osThreadAttr_t logging_attributes;
void Logging_Task(void *argument);
