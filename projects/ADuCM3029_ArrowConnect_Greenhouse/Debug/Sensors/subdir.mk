################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Sensors/adi_ad7124.cpp \
../Sensors/adi_ad7798.cpp \
../Sensors/adi_cn0397.cpp \
../Sensors/adi_cn0398.cpp \
../Sensors/adi_cn0410.cpp 

SRC_OBJS += \
./Sensors/adi_ad7124.o \
./Sensors/adi_ad7798.o \
./Sensors/adi_cn0397.o \
./Sensors/adi_cn0398.o \
./Sensors/adi_cn0410.o 

CPP_DEPS += \
./Sensors/adi_ad7124.d \
./Sensors/adi_ad7798.d \
./Sensors/adi_cn0397.d \
./Sensors/adi_cn0398.d \
./Sensors/adi_cn0410.d 


# Each subdirectory must supply rules for building sources it contributes
Sensors/%.o: ../Sensors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C++ Compiler'
	arm-none-eabi-g++ -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DDEBUG -DHTTP_DEBUG -DDEBUG_WOLFSSL -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\system" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\adc" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor\base_sensor" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include" -I"C:\Analog Devices\CrossCore Embedded Studio 2.6.0\ARM\packs\AnalogDevices\ADI-SensorSoftware\1.1.0\Include\sensor" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\skeleton" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\default" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029" -I"" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\platforms\ADuCM3029\json" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL\wolfssl" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\Wi-Fi_Driver" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\include" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL\wolfssl\wolfcrypt" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_ArrowConnect\src\wolfSSL" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


