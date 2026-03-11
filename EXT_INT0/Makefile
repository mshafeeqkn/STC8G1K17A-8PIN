CC      = sdcc
CFLAGS  = -mmcs51 --model-small --xram-size 1024 --code-size 8192
STCGAL  = /Library/Frameworks/Python.framework/Versions/3.6/bin/stcgal
PORT    = /dev/cu.usbserial-1410      # change to your adapter port
PLATFRM = stc8g
CLOCK_FREQ = 32000

TARGET  = main
SOURCES = main.c

all: $(TARGET).hex

# Compile + link to IHX
$(TARGET).ihx: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET).ihx

# Convert IHX -> HEX using packihx (ships with SDCC)
$(TARGET).hex: $(TARGET).ihx
	packihx $(TARGET).ihx > $(TARGET).hex

flash: $(TARGET).hex
	$(STCGAL) -P $(PLATFRM) -p $(PORT) -t $(CLOCK_FREQ) -b 9600 $(TARGET).hex

clean:
	rm -f *.rel *.lst *.map *.mem *.rst *.sym *.asm *.lk \
	      $(TARGET).ihx $(TARGET).hex

.PHONY: all flash clean
