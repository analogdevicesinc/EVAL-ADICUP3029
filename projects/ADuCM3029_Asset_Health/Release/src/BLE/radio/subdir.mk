################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BLE/radio/adi_ble_logevent.c \
../src/BLE/radio/adi_ble_radio.c \
../src/BLE/radio/adi_ble_radio_helper.c 

SRC_OBJS += \
./src/BLE/radio/adi_ble_logevent.o \
./src/BLE/radio/adi_ble_radio.o \
./src/BLE/radio/adi_ble_radio_helper.o 

C_DEPS += \
./src/BLE/radio/adi_ble_logevent.d \
./src/BLE/radio/adi_ble_radio.d \
./src/BLE/radio/adi_ble_radio_helper.d 


# Each subdirectory must supply rules for building sources it contributes
src/BLE/radio/%.o: ../src/BLE/radio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -D__ADUCM302x__ -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health\system" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.1/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.1/ARM/packs/AnalogDevices/ADuCM302x_IoT_DFP/1.0.0/Include" -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health/RTE" -I"C:\Work\ADICUP3029\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/BLE/transport" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/BLE/radio" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/Application" -I"C:/Work/ADICUP3029/ADuCM3029_Asset_Health/src/ADXL372" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


