################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Communication.c \
../src/Timer.c 

CPP_SRCS += \
../src/ADuCM3029_Greenhouse_Local.cpp \
../src/sensors_data.cpp 

SRC_OBJS += \
./src/ADuCM3029_Greenhouse_Local.o \
./src/Communication.o \
./src/Timer.o \
./src/sensors_data.o 

C_DEPS += \
./src/Communication.d \
./src/Timer.d 

CPP_DEPS += \
./src/ADuCM3029_Greenhouse_Local.d \
./src/sensors_data.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C++ Compiler'
	arm-none-eabi-g++ -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\system" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\UART" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\system" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_Local_Greenhouse\UART" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


