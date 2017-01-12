################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADuCM3029_test_project.c \
../src/Communication.c \
../src/Test.c \
../src/Timer.c 

SRC_OBJS += \
./src/ADuCM3029_test_project.o \
./src/Communication.o \
./src/Test.o \
./src/Timer.o 

C_DEPS += \
./src/ADuCM3029_test_project.d \
./src/Communication.d \
./src/Test.d \
./src/Timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -O2 -ffunction-sections -fdata-sections -DCORE0 -DNDEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_test_project\system" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/ssldd_config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/drivers/config" -I"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/ARM/CMSIS/4.5.0/CMSIS/Include" -I"C:/Users/spopa/cces/2.5.0/ADuCM3029_test_project/system/services/config" -I"C:/Analog Devices/CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Include" -I"C:\Users\spopa\cces\2.5.0\ADuCM3029_test_project/RTE" -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


