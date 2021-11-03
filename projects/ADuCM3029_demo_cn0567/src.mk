SRC_DIRS += $(PROJECT)/src
SRC_DIRS += $(NO-OS)/iio/iio_app

SRC_DIRS += $(PLATFORM_DRIVERS)
SRC_DIRS += $(INCLUDE)
SRC_DIRS += $(DRIVERS)/photo-electronic/adpd410x
SRC_DIRS += $(NO-OS)/drivers/gpio
SRC_DIRS += $(NO-OS)/drivers/spi
SRC_DIRS += $(NO-OS)/drivers/irq


SRCS += $(NO-OS)/util/list.c
SRCS += $(NO-OS)/util/util.c

IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.c
IGNORED_FILES += $(PLATFORM_DRIVERS)/uart_stdio.h

TINYIIOD=y
