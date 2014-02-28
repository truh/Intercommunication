# Tiva Makefile
# #####################################
#
# Part of the uCtools project
# uctools.github.com
#
#######################################
# user configuration:
#######################################
# TARGET: name of the output file
TARGET = main
# MCU: part number to build for
MCU = TM4C123GH6PM
# OUTDIR: directory to use for output
OUTDIR = build
# TIVAWARE_PATH: path to tivaware folder
TIVAWARE_PATH = $(HOME)/opt/tivaware

# SOURCES: list of input source sources
SOURCEDIR = src/buttons_isr
SOURCES = $(wildcard $(SOURCEDIR)/*.c)

SOURCES += $(TIVAWARE_PATH)/driverlib/gpio.c
SOURCES += $(TIVAWARE_PATH)/utils/uartstdio.c
SOURCES += $(TIVAWARE_PATH)/examples/boards/ek-tm4c123gxl/drivers/buttons.c

# INCLUDES: list of includes, by default, use Includes directory
INCLUDES = -Iinclude -I$(TIVAWARE_PATH)

# LD_SCRIPT: linker script
LD_SCRIPT = $(MCU).ld

# define flags
CFLAGS = -g -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS +=-Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall
CFLAGS += -pedantic -DPART_$(MCU) -c $(INCLUDES)
CFLAGS += -DTARGET_IS_BLIZZARD_RA1
LDFLAGS = -T $(LD_SCRIPT) --entry ResetISR --gc-sections

#######################################
# end of user configuration
#######################################
#
#######################################
# binaries
#######################################
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
FLASH	= lm4flash
RM      = rm -rf
MKDIR	= mkdir -p
#######################################

# list of object files, placed in the build directory regardless of source path
OBJECTS = $(addprefix $(OUTDIR)/,$(notdir $(SOURCES:.c=.o)))

# default: build bin
all: $(OUTDIR)/$(TARGET).bin

$(OBJECTS): $(SOURCES) | $(OUTDIR)
	$(CC) -o $@ $(filter %$(subst .o,.c,$(@F)), $(SOURCES)) $(CFLAGS)

$(OUTDIR)/$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/$(TARGET).bin: $(OUTDIR)/$(TARGET)
	$(OBJCOPY) -O binary $< $@

# create the output directory
$(OUTDIR):
	$(MKDIR) $(OUTDIR)

program: $(OUTDIR)/$(TARGET).bin
	$(FLASH) $(OUTDIR)/$(TARGET).bin

clean:
	-$(RM) $(OUTDIR)/*

.PHONY: all clean
