################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Wi-Fi_Driver/MQTTConnectClient.c \
../Wi-Fi_Driver/MQTTConnectServer.c \
../Wi-Fi_Driver/MQTTDeserializePublish.c \
../Wi-Fi_Driver/MQTTFormat.c \
../Wi-Fi_Driver/MQTTPacket.c \
../Wi-Fi_Driver/MQTTSerializePublish.c \
../Wi-Fi_Driver/MQTTSubscribeClient.c \
../Wi-Fi_Driver/MQTTSubscribeServer.c \
../Wi-Fi_Driver/MQTTUnsubscribeClient.c \
../Wi-Fi_Driver/MQTTUnsubscribeServer.c \
../Wi-Fi_Driver/adi_timestamp.c \
../Wi-Fi_Driver/adi_uart.c \
../Wi-Fi_Driver/adi_wifi.c \
../Wi-Fi_Driver/adi_wifi_logevent.c \
../Wi-Fi_Driver/adi_wifi_noos.c \
../Wi-Fi_Driver/adi_wifi_transport.c 

SRC_OBJS += \
./Wi-Fi_Driver/MQTTConnectClient.o \
./Wi-Fi_Driver/MQTTConnectServer.o \
./Wi-Fi_Driver/MQTTDeserializePublish.o \
./Wi-Fi_Driver/MQTTFormat.o \
./Wi-Fi_Driver/MQTTPacket.o \
./Wi-Fi_Driver/MQTTSerializePublish.o \
./Wi-Fi_Driver/MQTTSubscribeClient.o \
./Wi-Fi_Driver/MQTTSubscribeServer.o \
./Wi-Fi_Driver/MQTTUnsubscribeClient.o \
./Wi-Fi_Driver/MQTTUnsubscribeServer.o \
./Wi-Fi_Driver/adi_timestamp.o \
./Wi-Fi_Driver/adi_uart.o \
./Wi-Fi_Driver/adi_wifi.o \
./Wi-Fi_Driver/adi_wifi_logevent.o \
./Wi-Fi_Driver/adi_wifi_noos.o \
./Wi-Fi_Driver/adi_wifi_transport.o 

C_DEPS += \
./Wi-Fi_Driver/MQTTConnectClient.d \
./Wi-Fi_Driver/MQTTConnectServer.d \
./Wi-Fi_Driver/MQTTDeserializePublish.d \
./Wi-Fi_Driver/MQTTFormat.d \
./Wi-Fi_Driver/MQTTPacket.d \
./Wi-Fi_Driver/MQTTSerializePublish.d \
./Wi-Fi_Driver/MQTTSubscribeClient.d \
./Wi-Fi_Driver/MQTTSubscribeServer.d \
./Wi-Fi_Driver/MQTTUnsubscribeClient.d \
./Wi-Fi_Driver/MQTTUnsubscribeServer.d \
./Wi-Fi_Driver/adi_timestamp.d \
./Wi-Fi_Driver/adi_uart.d \
./Wi-Fi_Driver/adi_wifi.d \
./Wi-Fi_Driver/adi_wifi_logevent.d \
./Wi-Fi_Driver/adi_wifi_noos.d \
./Wi-Fi_Driver/adi_wifi_transport.d 


# Each subdirectory must supply rules for building sources it contributes
Wi-Fi_Driver/%.o: ../Wi-Fi_Driver/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DDEBUG -DHTTP_DEBUG -DDEBUG_WOLFSSL -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\adc" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\base_sensor" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\skeleton" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\default" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029" -I"" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL\wolfssl" -I/ADuCM3029_ArrowConnect_Greenhouse -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Wi-Fi_Driver" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


