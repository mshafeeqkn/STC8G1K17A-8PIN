# ==============================================================================
# Makefile for STC8G Dimmer Project
# ==============================================================================

CC      = sdcc

# Compiler Flags:
# -mmcs51: Target architecture is standard 8051
# --model-small: Use internal RAM for variables by default
# --xram-size/--code-size: Define memory boundaries for STC8G1K08
CFLAGS  = -mmcs51 --model-small --xram-size 1024 --code-size 8192

# Flashing Tool Configuration
STCGAL  = /Library/Frameworks/Python.framework/Versions/3.6/bin/stcgal
PORT    = /dev/cu.usbserial-1420
PLATFRM = stc8g
CLOCK_FREQ = 11059 # Target frequency in kHz (11.0592MHz)

TARGET  = main
SOURCES = main.c uart.c pwm.c nec.c

# Dynamically generate list of relocatable object files (.rel) from C sources
RELS    = $(SOURCES:.c=.rel)

# Default target
all: $(TARGET).hex

# Link Stage: Combine all .rel files into the final Intel Hex format (.ihx)
$(TARGET).ihx: $(RELS)
	$(CC) $(CFLAGS) $(RELS) -o $(TARGET).ihx

# Compile Stage: Pattern rule to compile each .c file into a .rel object file
%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Conversion Stage: Convert compiler .ihx output to standard .hex file format for flashing
$(TARGET).hex: $(TARGET).ihx
	packihx $(TARGET).ihx > $(TARGET).hex

# Flash Stage: Upload the compiled .hex file to the microcontroller
flash: $(TARGET).hex
	killall screen || true
	$(STCGAL) -P $(PLATFRM) -p $(PORT) -t $(CLOCK_FREQ) -b 115200 $(TARGET).hex

# Cleanup: Remove all compiler generated artifacts
clean:
	rm -f *.rel *.lst *.map *.mem *.rst *.sym *.asm *.lk \
	      $(TARGET).ihx $(TARGET).hex

# Mark targets that don't represent physical files
.PHONY: all flash clean
