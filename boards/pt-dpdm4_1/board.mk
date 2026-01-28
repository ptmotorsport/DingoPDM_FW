# List of all the board related files.
BOARDSRC = ./boards/pt-dpdm4_1/board.c

# Required include directories
BOARDINC = ./boards/pt-dpdm4_1

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)

include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f4xx.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk