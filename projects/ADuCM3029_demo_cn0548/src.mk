PLATFORM = aducm3029

SRC_DIRS += $(PROJECT)/src
SRC_DIRS += $(NO-OS)/iio/iio_app

SRC_DIRS += $(PLATFORM_DRIVERS)
SRC_DIRS += $(INCLUDE)
SRC_DIRS += $(DRIVERS)/adc/ad7799

SRCS += $(NO-OS)/util/list.c
SRCS += $(NO-OS)/util/util.c

IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.c
IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.h

TINYIIOD=y