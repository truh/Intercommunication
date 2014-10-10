MCU = TM4C123GH6PM

# TIVAWARE_PATH: path to tivaware folder
TIVAWARE_PATH = $(HOME)/opt/tivaware/
# OUTDIR: directory to use for output
OUTDIR = build

# Sources
VPATH = src

COMMON_SRC = 
MASTER_SRC = master.c
SLAVE_SRC = slave.c

INCLUDE = -Iinc

# LD_SCRIPT: linker script
LD_SCRIPT = $(MCU).ld

# define flags
CFLAGS = -g -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS +=-Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall
CFLAGS += -pedantic -DPART_$(MCU) -c -I$(TIVAWARE_PATH)
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
RM      = rm -f
MKDIR	= mkdir -p
#######################################

# list of object files, placed in the build directory regardless of source path
MASTER_OBJS = $(addprefix $(OUTDIR)/,$(notdir $(SOURCES:.c=.o)))

# list of object files, placed in the build directory regardless of source path
SLAVE_OBJS = $(addprefix $(OUTDIR)/,$(notdir $(SOURCES:.c=.o)))

# default: build bin
all: $(OUTDIR)/master.bin $(OUTDIR)/slave.bin

$(OUTDIR)/%.o: src/%.c | $(OUTDIR)
	$(CC) -o $@ $^ $(CFLAGS)

$(OUTDIR)/master.out: $(MASTER_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/slave.out: $(SLAVE_OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/master.bin: $(OUTDIR)/master.out
	$(OBJCOPY) -O binary $< $@

$(OUTDIR)/slave.bin: $(OUTDIR)/slave.out
	$(OBJCOPY) -O binary $< $@

# create the output directory
$(OUTDIR):
	$(MKDIR) $(OUTDIR)

clean:
	-$(RM) $(OUTDIR)/*

.PHONY: all clean
