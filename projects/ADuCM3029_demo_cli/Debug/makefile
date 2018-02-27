################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := ${ComSpec}

-include ../makefile.init

RM := clean

# All of the sources participating in the build are defined here
-include sources.mk
-include system/pinmux/GeneratedSources/subdir.mk
-include system/subdir.mk
-include src/subdir.mk
-include RTE/Device/ADuCM3029/subdir.mk
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
all: ADuCM3029_demo_cli

# Tool invocations
ADuCM3029_demo_cli: $(USER_OBJS) $(SRC_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: CrossCore GCC ARM Embedded C Linker'
	arm-none-eabi-gcc -TC:\Users\ADrimbar\git\EVAL-ADICUP3029\projects\ADuCM3029_demo_cli/RTE/Device/ADuCM3029/ADuCM3029.ld -Wl,--gc-sections -mcpu=cortex-m3 -mthumb -specs=nosys.specs -o  "ADuCM3029_demo_cli" @input-file.txt -lm
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) ADuCM3029_demo_cli
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
