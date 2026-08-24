# pwr_mock.py
# Stateful mock for STM32H7 PWR register to bypass clock/voltage config hangs in simulation.

if 'registers' not in globals():
    # Initialize the register state.
    # Start with PWR_CR1 (0x04) and PWR_D3CR (0x18) having VOSRDY/ACTVOSRDY (bit 13) set.
    registers = {
        0x04: 0x2000,
        0x18: 0x2000
    }

if request.IsInit:
    pass
elif request.IsWrite:
    # Retain the written value
    registers[request.Offset] = request.Value
elif request.IsRead:
    # Read the value from our register dictionary (default to 0 if not yet written)
    val = registers.get(request.Offset, 0)
    # Always ensure VOSRDY / ACTVOSRDY (bit 13) is set to bypass wait-ready loops
    if request.Offset in (0x04, 0x18):
        val |= 0x2000
    request.Value = val
