CC      = sdcc
CFLAGS  = -mmcs51 --model-small --code-size 8192
TARGET  = main
STCGAL  = /Library/Frameworks/Python.framework/Versions/3.6/bin/stcgal
PORT    = /dev/cu.usbserial-1420
PLATFRM = stc8g
CLOCK_FREQ = 11059 # Target frequency in kHz (11.0592MHz)

all: $(TARGET).hex

$(TARGET).ihx: $(TARGET).c
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET).ihx

$(TARGET).hex: $(TARGET).ihx
	packihx $(TARGET).ihx > $(TARGET).hex

flash: $(TARGET).hex
	$(STCGAL) -P $(PLATFRM) -p $(PORT) -t $(CLOCK_FREQ) -b 115200 $(TARGET).hex

clean:
	rm -f *.rel *.lst *.map *.mem *.rst *.sym *.asm *.lk *.ihx *.hex
