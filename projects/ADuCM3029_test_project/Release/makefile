################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := ${ComSpec}

-include ../makefile.init

RM := clean

# All of the sources participating in the build are defined here
-include sources.mk
-include system/services/wdt/subdir.mk
-include system/services/tmr/subdir.mk
-include system/services/rtc/subdir.mk
-include system/services/pwr/subdir.mk
-include system/services/int/subdir.mk
-include system/services/gpio/subdir.mk
-include system/services/dma/subdir.mk
-include system/pinmux/GeneratedSources/subdir.mk
-include system/drivers/uart/subdir.mk
-include system/drivers/sport/subdir.mk
-include system/drivers/spi/subdir.mk
-include system/drivers/i2c/subdir.mk
-include system/drivers/flash/subdir.mk
-include system/drivers/crypto/subdir.mk
-include system/drivers/crc/subdir.mk
-include system/drivers/beep/subdir.mk
-include system/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
OBJS := $(SRC_OBJS)
# Quote makefile variable : OBJS 
OBJS := $(subst \ ,@, $(OBJS))
OBJS := $(foreach var, $(OBJS), "$(subst @, ,$(var))")
# Quote makefile variable : C_DEPS 
C_DEPS := $(subst \ ,@, $(C_DEPS))
C_DEPS := $(foreach var, $(C_DEPS), "$(subst @, ,$(var))")

# All Target
all: ADuCM3029_test_project

# Tool invocations
ADuCM3029_test_project: $(USER_OBJS) $(SRC_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Linker'
	arm-none-eabi-gcc -TADuCM3029.ld -L"C:\Analog Devices\CrossCore Embedded Studio 2.5.0/ARM/packs/AnalogDevices/ADuCM302x_DFP/1.0.3/Source/GCC" -Wl,--gc-sections -mcpu=cortex-m3 -mthumb -specs=rdimon.specs -o  "ADuCM3029_test_project" @input-file.txt $(USER_OBJS) -lm
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) ADuCM3029_test_project
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
