CC      = sdcc
# Added -c to pattern rule, but keeping general flags here
CFLAGS  = -mmcs51 --model-small --xram-size 1024 --code-size 8192
STCGAL  = /Library/Frameworks/Python.framework/Versions/3.6/bin/stcgal
PORT    = /dev/cu.usbserial-1420
PLATFRM = stc8g
# Frequency in kHz (32000 = 32MHz)
# CLOCK_FREQ = 11059
CLOCK_FREQ = 32000

TARGET  = main
SOURCES = main.c

# This automatically creates a list of .rel files from your .c files
RELS    = $(SOURCES:.c=.rel)

all: $(TARGET).hex

# 1. Link the .rel files into the final .ihx
$(TARGET).ihx: $(RELS)
	$(CC) $(CFLAGS) $(RELS) -o $(TARGET).ihx

# 2. Pattern rule: Compile each .c file into a .rel file
%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 3. Convert IHX -> HEX
$(TARGET).hex: $(TARGET).ihx
	packihx $(TARGET).ihx > $(TARGET).hex

flash: $(TARGET).hex
	$(STCGAL) -P $(PLATFRM) -p $(PORT) -t $(CLOCK_FREQ) -b 115200 $(TARGET).hex

clean:
	rm -f *.rel *.lst *.map *.mem *.rst *.sym *.asm *.lk \
	      $(TARGET).ihx $(TARGET).hex

.PHONY: all flash clean
# stcgal -P stc8g -p /dev/cu.usbserial-1420 -t 11059 -b 115200 main.hex
