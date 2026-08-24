# GDB Initialization Script for Renode + STM32H7
set pagination off
set confirm off
set disassembly-flavor intel

# Catch HardFaults and Asserts automatically
break HardFault_Handler
break Default_Handler

# Custom helper command to reset machine and restart execution
define rrestart
    monitor sysbus.cpu Reset
    load
    continue
end
document rrestart
    Reset the CPU in Renode, reload ELF symbols, and continue execution.
end

# Custom helper command to step past ISRs to next line of code
define nline
    advance +1
end
document nline
    Advance to next source line without instruction-stepping into SysTick/ISRs.
end
