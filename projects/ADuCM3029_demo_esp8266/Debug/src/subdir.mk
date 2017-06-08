################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADXL362.c \
../src/Communication.c \
../src/ESP8266.c \
../src/PinMux.c \
../src/Timer.c \
../src/main.c 

SRC_OBJS += \
./src/ADXL362.o \
./src/Communication.o \
./src/ESP8266.o \
./src/PinMux.o \
./src/Timer.o \
./src/main.o 

C_DEPS += \
./src/ADXL362.d \
./src/Communication.d \
./src/ESP8266.d \
./src/PinMux.d \
./src/Timer.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\system" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\src" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


