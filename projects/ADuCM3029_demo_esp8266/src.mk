SRC_DIRS += $(PROJECT)/app_src
SRC_DIRS += $(PLATFORM_DRIVERS)
SRC_DIRS += $(INCLUDE)
SRC_DIRS += $(NO-OS)/util
SRC_DIRS += $(NO-OS)/network
SRC_DIRS += $(DRIVERS)/accel/adxl362
SRC_DIRS += $(NO-OS)/drivers/gpio
SRC_DIRS += $(NO-OS)/drivers/spi
SRC_DIRS += $(NO-OS)/drivers/irq
DISABLE_SECURE_SOCKET=y

LIBRARIES += mqtt
