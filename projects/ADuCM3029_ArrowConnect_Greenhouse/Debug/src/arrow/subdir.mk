################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/arrow/account.c \
../src/arrow/device.c \
../src/arrow/device_command.c \
../src/arrow/events.c \
../src/arrow/find_by.c \
../src/arrow/gateway.c \
../src/arrow/gateway_api.c \
../src/arrow/gateway_payload_sign.c \
../src/arrow/mem.c \
../src/arrow/mqtt.c \
../src/arrow/node.c \
../src/arrow/node_type.c \
../src/arrow/request.c \
../src/arrow/routine.c \
../src/arrow/sign.c \
../src/arrow/software_release.c \
../src/arrow/software_update.c \
../src/arrow/state.c \
../src/arrow/sys_weak.c \
../src/arrow/telemetry_api.c \
../src/arrow/testsuite.c \
../src/arrow/utf8.c 

SRC_OBJS += \
./src/arrow/account.o \
./src/arrow/device.o \
./src/arrow/device_command.o \
./src/arrow/events.o \
./src/arrow/find_by.o \
./src/arrow/gateway.o \
./src/arrow/gateway_api.o \
./src/arrow/gateway_payload_sign.o \
./src/arrow/mem.o \
./src/arrow/mqtt.o \
./src/arrow/node.o \
./src/arrow/node_type.o \
./src/arrow/request.o \
./src/arrow/routine.o \
./src/arrow/sign.o \
./src/arrow/software_release.o \
./src/arrow/software_update.o \
./src/arrow/state.o \
./src/arrow/sys_weak.o \
./src/arrow/telemetry_api.o \
./src/arrow/testsuite.o \
./src/arrow/utf8.o 

C_DEPS += \
./src/arrow/account.d \
./src/arrow/device.d \
./src/arrow/device_command.d \
./src/arrow/events.d \
./src/arrow/find_by.d \
./src/arrow/gateway.d \
./src/arrow/gateway_api.d \
./src/arrow/gateway_payload_sign.d \
./src/arrow/mem.d \
./src/arrow/mqtt.d \
./src/arrow/node.d \
./src/arrow/node_type.d \
./src/arrow/request.d \
./src/arrow/routine.d \
./src/arrow/sign.d \
./src/arrow/software_release.d \
./src/arrow/software_update.d \
./src/arrow/state.d \
./src/arrow/sys_weak.d \
./src/arrow/telemetry_api.d \
./src/arrow/testsuite.d \
./src/arrow/utf8.d 


# Each subdirectory must supply rules for building sources it contributes
src/arrow/%.o: ../src/arrow/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DDEBUG -DHTTP_DEBUG -DDEBUG_WOLFSSL -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\adc" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\base_sensor" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\skeleton" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\default" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029" -I"" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL\wolfssl" -I/ADuCM3029_ArrowConnect_Greenhouse -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Wi-Fi_Driver" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


