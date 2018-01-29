################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platforms/senseability/trace.c 

SRC_OBJS += \
./platforms/senseability/trace.o 

C_DEPS += \
./platforms/senseability/trace.d 


# Each subdirectory must supply rules for building sources it contributes
platforms/senseability/%.o: ../platforms/senseability/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\system" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\skeleton" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\default" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


