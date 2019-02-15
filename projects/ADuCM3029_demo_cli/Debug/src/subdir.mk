################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ADuCM3029_demo_cli.c \
../src/Cli.c \
../src/Communication.c \
../src/Timer.c 

SRC_OBJS += \
./src/ADuCM3029_demo_cli.o \
./src/Cli.o \
./src/Communication.o \
./src/Timer.o 

C_DEPS += \
./src/ADuCM3029_demo_cli.d \
./src/Cli.d \
./src/Communication.d \
./src/Timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -ffunction-sections -fdata-sections -DCORE0 -D_DEBUG -D_RTE_ -D__ADUCM3029__ -D__SILICON_REVISION__=0x100 @includes-afa06b5f1ce396f0c89a226ccea14b75.txt -Wall -c -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


