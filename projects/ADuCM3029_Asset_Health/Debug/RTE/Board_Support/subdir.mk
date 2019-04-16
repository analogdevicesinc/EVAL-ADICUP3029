################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/radio/adi_ble_logevent.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/framework/noos/adi_ble_noos.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/radio/adi_ble_radio.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/transport/adi_ble_transport.c \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/common/adi_timestamp.c 

SRC_OBJS += \
./RTE/Board_Support/adi_ble_logevent.o \
./RTE/Board_Support/adi_ble_noos.o \
./RTE/Board_Support/adi_ble_radio.o \
./RTE/Board_Support/adi_ble_transport.o \
./RTE/Board_Support/adi_timestamp.o 

C_DEPS += \
./RTE/Board_Support/adi_ble_logevent.d \
./RTE/Board_Support/adi_ble_noos.d \
./RTE/Board_Support/adi_ble_radio.d \
./RTE/Board_Support/adi_ble_transport.d \
./RTE/Board_Support/adi_timestamp.d 


# Each subdirectory must supply rules for building sources it contributes
RTE/Board_Support/adi_ble_logevent.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/radio/adi_ble_logevent.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-4ae8cf73e6fed7c05a06e2774f02d67d.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Board_Support/adi_ble_noos.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/framework/noos/adi_ble_noos.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-4ae8cf73e6fed7c05a06e2774f02d67d.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Board_Support/adi_ble_radio.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/radio/adi_ble_radio.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-4ae8cf73e6fed7c05a06e2774f02d67d.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Board_Support/adi_ble_transport.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/ble/transport/adi_ble_transport.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-4ae8cf73e6fed7c05a06e2774f02d67d.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

RTE/Board_Support/adi_timestamp.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.8.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Source/common/adi_timestamp.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-4ae8cf73e6fed7c05a06e2774f02d67d.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


