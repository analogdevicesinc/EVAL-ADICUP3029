SRC_DIRS += $(PROJECT)/src
SRC_DIRS += $(NO-OS)/iio/iio_app

SRC_DIRS += $(PLATFORM_DRIVERS)
SRC_DIRS += $(INCLUDE)
SRC_DIRS += $(DRIVERS)/adc/ad7799
SRC_DIRS += $(NO-OS)/drivers/api

SRCS += $(NO-OS)/util/no_os_list.c
SRCS += $(NO-OS)/util/no_os_util.c

IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.c
IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.h

TINYIIOD=y
