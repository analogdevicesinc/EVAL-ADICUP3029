################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Communication.c \
../src/Timer.c \
../src/adxl372.c \
../src/main.c \
../src/platform_drivers.c 

SRC_OBJS += \
./src/Communication.o \
./src/Timer.o \
./src/adxl372.o \
./src/main.o \
./src/platform_drivers.o 

C_DEPS += \
./src/Communication.d \
./src/Timer.d \
./src/adxl372.d \
./src/main.d \
./src/platform_drivers.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -Og -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -DEVAL_ADICUP3029 -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-f977b94f5e7a240c7b29279bab25467a.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


