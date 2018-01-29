################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/wolfSSL/src/crl.c \
../src/wolfSSL/src/internal.c \
../src/wolfSSL/src/io.c \
../src/wolfSSL/src/keys.c \
../src/wolfSSL/src/ocsp.c \
../src/wolfSSL/src/sniffer.c \
../src/wolfSSL/src/ssl.c \
../src/wolfSSL/src/tls.c 

SRC_OBJS += \
./src/wolfSSL/src/crl.o \
./src/wolfSSL/src/internal.o \
./src/wolfSSL/src/io.o \
./src/wolfSSL/src/keys.o \
./src/wolfSSL/src/ocsp.o \
./src/wolfSSL/src/sniffer.o \
./src/wolfSSL/src/ssl.o \
./src/wolfSSL/src/tls.o 

C_DEPS += \
./src/wolfSSL/src/crl.d \
./src/wolfSSL/src/internal.d \
./src/wolfSSL/src/io.d \
./src/wolfSSL/src/keys.d \
./src/wolfSSL/src/ocsp.d \
./src/wolfSSL/src/sniffer.d \
./src/wolfSSL/src/ssl.d \
./src/wolfSSL/src/tls.d 


# Each subdirectory must supply rules for building sources it contributes
src/wolfSSL/src/%.o: ../src/wolfSSL/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DDEBUG -DHTTP_DEBUG -DDEBUG_WOLFSSL -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\adc" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\base_sensor" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\skeleton" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\default" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029" -I"" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL\wolfssl" -I/ADuCM3029_ArrowConnect_Greenhouse -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Wi-Fi_Driver" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


