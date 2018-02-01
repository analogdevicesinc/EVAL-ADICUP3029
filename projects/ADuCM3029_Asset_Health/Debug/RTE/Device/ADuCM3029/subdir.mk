################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/dma/adi_dma.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/gpio/adi_gpio.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/pwr/adi_pwr.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/rtc/adi_rtc.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/spi/adi_spi.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/tmr/adi_tmr.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/uart/adi_uart.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/wdt/adi_wdt.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/xint/adi_xint.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/common.c \
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
RTE/Device/ADuCM3029/adi_dma.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/dma/adi_dma.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_gpio.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/gpio/adi_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_pwr.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/pwr/adi_pwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_rtc.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/rtc/adi_rtc.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_spi.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/spi/adi_spi.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_tmr.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/tmr/adi_tmr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_uart.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/uart/adi_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_wdt.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/wdt/adi_wdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_xint.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/drivers/xint/adi_xint.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/common.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Source/common.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/%.o: ../RTE/Device/ADuCM3029/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


