# ru-tel - Solar PDU & Telemetry Firmware

Power Distribution Unit (PDU) and Telemetry system running on the **STM32H753** microcontroller with **FreeRTOS**.

## Setup

All required build tools, GCC ARM cross-compiler, OpenOCD, Renode simulator, and language servers can be installed automatically using `depend.sh`:

```bash
./depend.sh
```

`depend.sh` will also set up USB udev rules (`/etc/udev/rules.d/60-openocd.rules`) for non-root flashing access via ST-Link and CMSIS-DAP debuggers.

---

## Build

To clean, configure, and build the ELF binary in one step:

```bash
./commands.sh
```

### Manual CMake Build
```bash
# Generate build system using the GCC ARM toolchain (run once)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -B build

# Compile binary
cmake --build build
```

The output binary will be generated at `build/ru-tel.elf`.

---

## Flashing with OpenOCD

The CMake configuration includes custom targets (`flash` and `openocd-flash`) to compile and flash the firmware directly to the target hardware via OpenOCD in one step.

### 1. Using CMake Flash Target
Once configured, run:

```bash
cmake --build build --target flash
```

Or using the alias:
```bash
cmake --build build --target openocd-flash
```


> **Note:** Executing the `flash` target automatically triggers a build of `ru-tel.elf` if any source files have changed prior to flashing.

### 2. Customizing Interface / Target Configuration
By default, OpenOCD uses `interface/stlink.cfg` and `target/stm32h7x.cfg`. You can customize these configuration paths during CMake generation:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
      -DOPENOCD_INTERFACE=interface/stlink-v2-1.cfg \
      -DOPENOCD_TARGET=target/stm32h7x.cfg \
      -B build
```
You can also invoke OpenOCD directly from the command line:

```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/ru-tel.elf verify reset exit"
```

---

## 🧪 Simulation & VHIL Testing (Renode)

To test and debug the firmware in a Virtual Hardware-in-the-Loop (VHIL) environment without hardware attached:

```bash
./run_vhil.sh
```

Available flags for `run_vhil.sh`:
- `./run_vhil.sh --normal` : Standard mode (Renode + GDB auto-continue)
- `./run_vhil.sh --step`   : Halts GDB at startup for step-by-step debugging
- `./run_vhil.sh --no-gdb` : Runs Renode simulation without attaching GDB

---

##  Profiling & Utilities

- **SEGGER SystemView Setup:** `./SEGGER_SYSVIEW_SETUP.sh --linux`
- **Trace Viewer:** `python3 trace_viewer.py`
- **Power peripheral mock script:** `pwr_mock.py`
