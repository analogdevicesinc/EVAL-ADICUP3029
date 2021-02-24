PLATFORM = aducm3029

ifeq '$(LOCAL_BUILD)' 'y'

include ../../local_variables.mk

endif

PROJECT			?= $(realpath .)
TARGET 			?= $(notdir $(realpath .))
NO-OS			?= $(realpath ../../no-OS)
BUILD_DIR_NAME		?= build
WORKSPACE		?= $(PROJECT)/$(BUILD_DIR_NAME)

INCLUDE			?= $(NO-OS)/include
DRIVERS 		?= $(NO-OS)/drivers
PLATFORM_DRIVERS	?= $(NO-OS)/drivers/platform/$(PLATFORM)

HARDWARE ?= $(wildcard pinmux_config.c)

# By default copy noos sources into the build directory of the project.
LINK_SRCS ?= n
