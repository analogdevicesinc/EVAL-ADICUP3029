################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/dma/adi_dma.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/pwr/adi_pwr.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/tmr/adi_tmr.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/uart/adi_uart.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/wdt/adi_wdt.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/common.c \
../RTE/Device/ADuCM3029/startup_ADuCM3029.c \
../RTE/Device/ADuCM3029/system_ADuCM3029.c 

S_UPPER_SRCS += \
../RTE/Device/ADuCM3029/reset_ADuCM3029.S 

SRC_OBJS += \
./RTE/Device/ADuCM3029/adi_dma.o \
./RTE/Device/ADuCM3029/adi_pwr.o \
./RTE/Device/ADuCM3029/adi_tmr.o \
./RTE/Device/ADuCM3029/adi_uart.o \
./RTE/Device/ADuCM3029/adi_wdt.o \
./RTE/Device/ADuCM3029/common.o \
./RTE/Device/ADuCM3029/reset_ADuCM3029.o \
./RTE/Device/ADuCM3029/startup_ADuCM3029.o \
./RTE/Device/ADuCM3029/system_ADuCM3029.o 

C_DEPS += \
./RTE/Device/ADuCM3029/adi_dma.d \
./RTE/Device/ADuCM3029/adi_pwr.d \
./RTE/Device/ADuCM3029/adi_tmr.d \
./RTE/Device/ADuCM3029/adi_uart.d \
./RTE/Device/ADuCM3029/adi_wdt.d \
./RTE/Device/ADuCM3029/common.d \
./RTE/Device/ADuCM3029/startup_ADuCM3029.d \
./RTE/Device/ADuCM3029/system_ADuCM3029.d 


# Each subdirectory must supply rules for building sources it contributes
RTE/Device/ADuCM3029/adi_dma.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/dma/adi_dma.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_pwr.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/pwr/adi_pwr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_tmr.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/tmr/adi_tmr.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_uart.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/uart/adi_uart.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/adi_wdt.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/drivers/wdt/adi_wdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/common.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Source/common.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/%.o: ../RTE/Device/ADuCM3029/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded Assembler'
	arm-none-eabi-gcc -c -x assembler-with-cpp -g -gdwarf-2 -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\ADrimbar\git\EVAL-ADICUP3029\projects\ADuCM3029_demo_cli\system" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/ARM/CMSIS/5.2.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/dma" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/pwr" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/tmr" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/uart" -I"C:/Analog Devices/CrossCore Embedded Studio 2.7.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/wdt" -I"C:\Users\ADrimbar\git\EVAL-ADICUP3029\projects\ADuCM3029_demo_cli/RTE" -I"C:\Users\ADrimbar\git\EVAL-ADICUP3029\projects\ADuCM3029_demo_cli/RTE/Device/ADuCM3029" -mcpu=cortex-m3 -mthumb -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Device/ADuCM3029/%.o: ../RTE/Device/ADuCM3029/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-ace88a7f676e681455857b8b214a14d7.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


