PLATFORM = aducm3029

PROJECT			?= $(realpath .)
TARGET 			?= $(notdir $(realpath .))
NO-OS			?= $(realpath ../../no-OS)
WORKSPACE		?= $(realpath ..)

INCLUDE			?= $(NO-OS)/include
DRIVERS 		?= $(NO-OS)/drivers
PLATFORM_DRIVERS	?= $(NO-OS)/drivers/platform/$(PLATFORM)

ifeq '$(LOCAL_BUILD)' 'y'

include ../../local_variables.mk

endif

# By default copy noos sources into the build directory of the project.
LINK_SRCS ?= n
