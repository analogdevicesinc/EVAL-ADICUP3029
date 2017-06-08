################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/MQTT/MQTTConnectClient.c \
../src/MQTT/MQTTConnectServer.c \
../src/MQTT/MQTTDeserializePublish.c \
../src/MQTT/MQTTFormat.c \
../src/MQTT/MQTTPacket.c \
../src/MQTT/MQTTSerializePublish.c \
../src/MQTT/MQTTSubscribeClient.c \
../src/MQTT/MQTTSubscribeServer.c \
../src/MQTT/MQTTUnsubscribeClient.c \
../src/MQTT/MQTTUnsubscribeServer.c 

SRC_OBJS += \
./src/MQTT/MQTTConnectClient.o \
./src/MQTT/MQTTConnectServer.o \
./src/MQTT/MQTTDeserializePublish.o \
./src/MQTT/MQTTFormat.o \
./src/MQTT/MQTTPacket.o \
./src/MQTT/MQTTSerializePublish.o \
./src/MQTT/MQTTSubscribeClient.o \
./src/MQTT/MQTTSubscribeServer.o \
./src/MQTT/MQTTUnsubscribeClient.o \
./src/MQTT/MQTTUnsubscribeServer.o 

C_DEPS += \
./src/MQTT/MQTTConnectClient.d \
./src/MQTT/MQTTConnectServer.d \
./src/MQTT/MQTTDeserializePublish.d \
./src/MQTT/MQTTFormat.d \
./src/MQTT/MQTTPacket.d \
./src/MQTT/MQTTSerializePublish.d \
./src/MQTT/MQTTSubscribeClient.d \
./src/MQTT/MQTTSubscribeServer.d \
./src/MQTT/MQTTUnsubscribeClient.d \
./src/MQTT/MQTTUnsubscribeServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/MQTT/%.o: ../src/MQTT/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\system" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\src" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266\src\MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266/RTE" -I"C:\Users\spopa\cces\demo_esp8266\ADuCM3029_demo_esp8266/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


