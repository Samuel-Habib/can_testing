#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H

#define RTT_USE_ASM                             (0)

#define SEGGER_RTT_MAX_NUM_UP_BUFFERS           2
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS         2
#define BUFFER_SIZE_UP                          4096
#define BUFFER_SIZE_DOWN                        16

#define SEGGER_RTT_MODE_DEFAULT                 SEGGER_RTT_MODE_NO_BLOCK_SKIP

#endif
