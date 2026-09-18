#include "main.h"
#include "string.h"

void send_log_message(const char *data);
int uart_driver(void);

extern const osThreadAttr_t logging_attributes;
void Logging_Task(void *argument);
