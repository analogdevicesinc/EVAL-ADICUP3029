################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/dma/adi_dma.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/gpio/adi_gpio.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/pwr/adi_pwr.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/rtc/adi_rtc.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/spi/adi_spi.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/tmr/adi_tmr.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/uart/adi_uart.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/wdt/adi_wdt.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/xint/adi_xint.c \
D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/common.c \
../RTE/Device/ADuCM3029/startup_ADuCM3029.c \
../RTE/Device/ADuCM3029/system_ADuCM3029.c 

SRC_OBJS += \
./RTE/Device/ADuCM3029/adi_dma.o \
./RTE/Device/ADuCM3029/adi_gpio.o \
./RTE/Device/ADuCM3029/adi_pwr.o \
./RTE/Device/ADuCM3029/adi_rtc.o \
./RTE/Device/ADuCM3029/adi_spi.o \
./RTE/Device/ADuCM3029/adi_tmr.o \
./RTE/Device/ADuCM3029/adi_uart.o \
./RTE/Device/ADuCM3029/adi_wdt.o \
./RTE/Device/ADuCM3029/adi_xint.o \
./RTE/Device/ADuCM3029/common.o \
./RTE/Device/ADuCM3029/startup_ADuCM3029.o \
./RTE/Device/ADuCM3029/system_ADuCM3029.o 

C_DEPS += \
./RTE/Device/ADuCM3029/adi_dma.d \
./RTE/Device/ADuCM3029/adi_gpio.d \
./RTE/Device/ADuCM3029/adi_pwr.d \
./RTE/Device/ADuCM3029/adi_rtc.d \
./RTE/Device/ADuCM3029/adi_spi.d \
./RTE/Device/ADuCM3029/adi_tmr.d \
./RTE/Device/ADuCM3029/adi_uart.d \
./RTE/Device/ADuCM3029/adi_wdt.d \
./RTE/Device/ADuCM3029/adi_xint.d \
./RTE/Device/ADuCM3029/common.d \
./RTE/Device/ADuCM3029/startup_ADuCM3029.d \
./RTE/Device/ADuCM3029/system_ADuCM3029.d 


# Each subdirectory must supply rules for building sources it contributes
RTE/Device/ADuCM3029/adi_dma.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/dma/adi_dma.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_gpio.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/gpio/adi_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_pwr.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/pwr/adi_pwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_rtc.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/rtc/adi_rtc.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_spi.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/spi/adi_spi.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_tmr.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/tmr/adi_tmr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_uart.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/uart/adi_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_wdt.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/wdt/adi_wdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_xint.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/xint/adi_xint.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/common.o: D:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/common.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/%.o: ../RTE/Device/ADuCM3029/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


