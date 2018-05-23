################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Sensors/adi_cn0410.cpp 

SRC_OBJS += \
./Sensors/adi_cn0410.o 

CPP_DEPS += \
./Sensors/adi_cn0410.d 


# Each subdirectory must supply rules for building sources it contributes
Sensors/%.o: ../Sensors/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C++ Compiler'
	arm-none-eabi-g++ -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410\system" -I"" -I"" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.1.0/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADI-WifiSoftware/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADI-WifiSoftware/1.0.0/Include/communication" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADI-WifiSoftware/1.0.0/Include/communication/wifi" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADI-WifiSoftware/1.0.0/Include/communication/wifi/transport" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADI-WifiSoftware/1.0.0/Source/communication/wifi/MQTT" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/dma" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/gpio" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/pwr" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/rtc" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/spi" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/3.1.0/Include/drivers/wdt" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410/RTE" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410/RTE/Device/ADuCM3029" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410/RTE/Wi-Fi" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410\Sensors" -I"C:\Users\mcaprior\cces\2.6.0\ADuCM3029_demo_cn0410\include" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


