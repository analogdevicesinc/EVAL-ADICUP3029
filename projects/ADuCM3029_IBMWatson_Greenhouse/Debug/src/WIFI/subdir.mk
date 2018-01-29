################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/WIFI/MQTTConnectClient.c \
../src/WIFI/MQTTConnectServer.c \
../src/WIFI/MQTTDeserializePublish.c \
../src/WIFI/MQTTFormat.c \
../src/WIFI/MQTTPacket.c \
../src/WIFI/MQTTSerializePublish.c \
../src/WIFI/MQTTSubscribeClient.c \
../src/WIFI/MQTTSubscribeServer.c \
../src/WIFI/MQTTUnsubscribeClient.c \
../src/WIFI/MQTTUnsubscribeServer.c \
../src/WIFI/adi_timestamp.c \
../src/WIFI/adi_uart.c \
../src/WIFI/adi_wifi.c \
../src/WIFI/adi_wifi_logevent.c \
../src/WIFI/adi_wifi_noos.c \
../src/WIFI/adi_wifi_transport.c 

SRC_OBJS += \
./src/WIFI/MQTTConnectClient.o \
./src/WIFI/MQTTConnectServer.o \
./src/WIFI/MQTTDeserializePublish.o \
./src/WIFI/MQTTFormat.o \
./src/WIFI/MQTTPacket.o \
./src/WIFI/MQTTSerializePublish.o \
./src/WIFI/MQTTSubscribeClient.o \
./src/WIFI/MQTTSubscribeServer.o \
./src/WIFI/MQTTUnsubscribeClient.o \
./src/WIFI/MQTTUnsubscribeServer.o \
./src/WIFI/adi_timestamp.o \
./src/WIFI/adi_uart.o \
./src/WIFI/adi_wifi.o \
./src/WIFI/adi_wifi_logevent.o \
./src/WIFI/adi_wifi_noos.o \
./src/WIFI/adi_wifi_transport.o 

C_DEPS += \
./src/WIFI/MQTTConnectClient.d \
./src/WIFI/MQTTConnectServer.d \
./src/WIFI/MQTTDeserializePublish.d \
./src/WIFI/MQTTFormat.d \
./src/WIFI/MQTTPacket.d \
./src/WIFI/MQTTSerializePublish.d \
./src/WIFI/MQTTSubscribeClient.d \
./src/WIFI/MQTTSubscribeServer.d \
./src/WIFI/MQTTUnsubscribeClient.d \
./src/WIFI/MQTTUnsubscribeServer.d \
./src/WIFI/adi_timestamp.d \
./src/WIFI/adi_uart.d \
./src/WIFI/adi_wifi.d \
./src/WIFI/adi_wifi_logevent.d \
./src/WIFI/adi_wifi_noos.d \
./src/WIFI/adi_wifi_transport.d 


# Each subdirectory must supply rules for building sources it contributes
src/WIFI/%.o: ../src/WIFI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson\system" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson\json" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


