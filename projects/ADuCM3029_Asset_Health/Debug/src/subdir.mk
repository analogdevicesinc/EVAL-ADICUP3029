################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Communication.c \
../src/Timer.c \
../src/main.c 

SRC_OBJS += \
./src/Communication.o \
./src/Timer.o \
./src/main.o 

C_DEPS += \
./src/Communication.d \
./src/Timer.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D__ADUCM302x__ -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health\system" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/transport" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/BLE/radio" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/ADXL372" -I"D:/Work/GitClones/EVAL-ADICUP3029/projects/ADuCM3029_Asset_Health/src/Application" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/ARM/CMSIS/5.0.1/CMSIS/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/2.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include" -I"C:/Analog Devices/CrossCore Embedded Studio 2.6.0/ARM/packs/AnalogDevices/EVAL-ADICUP3029_BSP/1.0.0/Include/ble" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Board_Support" -I"D:\Work\GitClones\EVAL-ADICUP3029\projects\ADuCM3029_Asset_Health/RTE/Device/ADuCM3029" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


