################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/dma/adi_dma.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/gpio/adi_gpio.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/int/adi_int.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/int/adi_nvic.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/pwr/adi_pwr.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/drivers/spi/adi_spi.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/drivers/uart/adi_uart.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/wdt/adi_wdt.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/common.c 

SRC_OBJS += \
./RTE/Device/ADuCM3029/adi_dma.o \
./RTE/Device/ADuCM3029/adi_gpio.o \
./RTE/Device/ADuCM3029/adi_int.o \
./RTE/Device/ADuCM3029/adi_nvic.o \
./RTE/Device/ADuCM3029/adi_pwr.o \
./RTE/Device/ADuCM3029/adi_spi.o \
./RTE/Device/ADuCM3029/adi_uart.o \
./RTE/Device/ADuCM3029/adi_wdt.o \
./RTE/Device/ADuCM3029/common.o 

C_DEPS += \
./RTE/Device/ADuCM3029/adi_dma.d \
./RTE/Device/ADuCM3029/adi_gpio.d \
./RTE/Device/ADuCM3029/adi_int.d \
./RTE/Device/ADuCM3029/adi_nvic.d \
./RTE/Device/ADuCM3029/adi_pwr.d \
./RTE/Device/ADuCM3029/adi_spi.d \
./RTE/Device/ADuCM3029/adi_uart.d \
./RTE/Device/ADuCM3029/adi_wdt.d \
./RTE/Device/ADuCM3029/common.d 


# Each subdirectory must supply rules for building sources it contributes
RTE/Device/ADuCM3029/adi_dma.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/dma/adi_dma.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_gpio.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/gpio/adi_gpio.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_int.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/int/adi_int.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_nvic.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/int/adi_nvic.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_pwr.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/pwr/adi_pwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_spi.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/drivers/spi/adi_spi.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_uart.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/drivers/uart/adi_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_wdt.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/services/wdt/adi_wdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/common.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Source/common.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -UADI_DEBUG -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_demo_esp8266/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.4/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


