################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BLE/transport/adi_transport.c 

SRC_OBJS += \
./src/BLE/transport/adi_transport.o 

C_DEPS += \
./src/BLE/transport/adi_transport.d 


# Each subdirectory must supply rules for building sources it contributes
src/BLE/transport/%.o: ../src/BLE/transport/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health\system" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/BLE/transport" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/BLE/radio" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/ADXL372" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.1/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.1/ARM/packs/AnalogDevices/ADuCM302x_IoT_DFP/1.0.0/Include" -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health/RTE" -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


