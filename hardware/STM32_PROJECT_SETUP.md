# STM32 project completion

The repository does not identify the exact STM32 MCU, development board, or
CubeMX version. Therefore HAL, `main.h`, linker scripts, startup code, and a
real `.ioc` file must not be fabricated.

## Required hardware inputs

Record these values before generating the project:

- Exact MCU part number and package.
- Board revision and clock source.
- UART used for K210 and UART used for BLE.
- SPI pins/CS/DRDY for ADS1256.
- I2C address and pins for RM3100.
- DMA streams, interrupt priorities, and FreeRTOS heap strategy.
- GPIO names for TX gate, damping SSR, RGB LED, and battery ADC.

## Generation steps

1. Open STM32CubeMX matching the chosen MCU and create the project.
2. Enable HAL, CMSIS-RTOS2/FreeRTOS, required UART DMA and peripheral pins.
3. Generate `Core/Inc/main.h`, HAL sources, startup code, linker script, and `.ioc`.
4. Copy the generated project files into this repository without overwriting
   the protocol library.
5. Add `protocol/` and the application source directories to the generated IDE
   project.
6. Run the host checks first, then build the generated target, then test on
   hardware with a logic analyzer.

`Hadeed.ioc.template` is a checklist placeholder, not a buildable CubeMX file.
