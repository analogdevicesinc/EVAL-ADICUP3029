################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Source/drivers/crc/adi_crc.c 

SRC_OBJS += \
./system/drivers/crc/adi_crc.o 

C_DEPS += \
./system/drivers/crc/adi_crc.d 


# Each subdirectory must supply rules for building sources it contributes
system/drivers/crc/adi_crc.o: C:/Analog\ Devices/CrossCore\ Embedded\ Studio\ 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Source/drivers/crc/adi_crc.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DADI_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_test_project\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/drivers/config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/services/config" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_test_project/RTE" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


