################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../WIFI/MQTTConnectClient.c \
../WIFI/MQTTConnectServer.c \
../WIFI/MQTTDeserializePublish.c \
../WIFI/MQTTFormat.c \
../WIFI/MQTTPacket.c \
../WIFI/MQTTSerializePublish.c \
../WIFI/MQTTSubscribeClient.c \
../WIFI/MQTTSubscribeServer.c \
../WIFI/MQTTUnsubscribeClient.c \
../WIFI/MQTTUnsubscribeServer.c \
../WIFI/adi_timestamp.c \
../WIFI/adi_uart.c \
../WIFI/adi_wifi.c \
../WIFI/adi_wifi_logevent.c \
../WIFI/adi_wifi_noos.c \
../WIFI/adi_wifi_transport.c 

SRC_OBJS += \
./WIFI/MQTTConnectClient.o \
./WIFI/MQTTConnectServer.o \
./WIFI/MQTTDeserializePublish.o \
./WIFI/MQTTFormat.o \
./WIFI/MQTTPacket.o \
./WIFI/MQTTSerializePublish.o \
./WIFI/MQTTSubscribeClient.o \
./WIFI/MQTTSubscribeServer.o \
./WIFI/MQTTUnsubscribeClient.o \
./WIFI/MQTTUnsubscribeServer.o \
./WIFI/adi_timestamp.o \
./WIFI/adi_uart.o \
./WIFI/adi_wifi.o \
./WIFI/adi_wifi_logevent.o \
./WIFI/adi_wifi_noos.o \
./WIFI/adi_wifi_transport.o 

C_DEPS += \
./WIFI/MQTTConnectClient.d \
./WIFI/MQTTConnectServer.d \
./WIFI/MQTTDeserializePublish.d \
./WIFI/MQTTFormat.d \
./WIFI/MQTTPacket.d \
./WIFI/MQTTSerializePublish.d \
./WIFI/MQTTSubscribeClient.d \
./WIFI/MQTTSubscribeServer.d \
./WIFI/MQTTUnsubscribeClient.d \
./WIFI/MQTTUnsubscribeServer.d \
./WIFI/adi_timestamp.d \
./WIFI/adi_uart.d \
./WIFI/adi_wifi.d \
./WIFI/adi_wifi_logevent.d \
./WIFI/adi_wifi_noos.d \
./WIFI/adi_wifi_transport.d 


# Each subdirectory must supply rules for building sources it contributes
WIFI/%.o: ../WIFI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\ARM\CMSIS\5.0.1\CMSIS\Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_IBMWatson_Greenhouse\WIFI" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


