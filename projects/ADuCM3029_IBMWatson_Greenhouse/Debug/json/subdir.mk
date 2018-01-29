################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../json/json.c \
../json/telemetry.c 

SRC_OBJS += \
./json/json.o \
./json/telemetry.o 

C_DEPS += \
./json/json.d \
./json/telemetry.d 


# Each subdirectory must supply rules for building sources it contributes
json/%.o: ../json/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\ARM\CMSIS\5.0.1\CMSIS\Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\WIFI" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


