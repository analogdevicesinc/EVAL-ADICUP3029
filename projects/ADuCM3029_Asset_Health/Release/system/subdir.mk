################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/adi_initialize.c 

SRC_OBJS += \
./system/adi_initialize.o 

C_DEPS += \
./system/adi_initialize.d 


# Each subdirectory must supply rules for building sources it contributes
system/%.o: ../system/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D__ADUCM302x__ -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0\system" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/transport" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/BLE/radio" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/Application" -I"D:/WorkSpace/ImpactMaxPeak/ADuCM3029_Asset_Health_Core0/src/ADXL372" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"D:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Board_Support" -I"D:\WorkSpace\ImpactMaxPeak\ADuCM3029_Asset_Health_Core0/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


