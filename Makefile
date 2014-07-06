
MSP430=msp430g2553

CC=msp430-gcc
CFLAGS=-mmcu=$(MSP430) -Os
CURDIR=`pwd`
LDFLAGS=-mmcu=$(MSP430) -Wl,-L ./$(MSP430)/ -Wl,-T ./$(MSP430)/msp430.x
# -Wl,--verbose -Wl,-M # DEBUG FLAGS
#LDFLAGS=-mmcu=$(MSP430)
COMMONSRC=adc.c morse.c time.c flash.c onewire.c
COMMONOBJ=$(COMMONSRC:.c=.o)
COMMONH=eggs.h adc.h morse.h time.h flash.h onewire.h
NOTESSRC=eggs.c
NOTESOBJ=$(NOTESSRC:.c=.o)
NOTESEXE=$(NOTESSRC:.c=.bin)


all: $(NOTESEXE)

#msp430-gcc morse.h morse.c love.h love_note_2013_01_17.c sleep.h sleep.c -Os -Wl,-T ./msp430g2001/memory.x -Wl,-T ./msp430g2001/periph.x -Wl,-T ./msp430g2001/msp430.x -o love_note_2013_01_17

.o: $< $(COMMONH)
	$(CC) $(LDFLAGS) $(COMMONH) $< -c -o $@

%.bin : %.o $(COMMONOBJ)
	$(CC) $(LDFLAGS) $(COMMONOBJ) $< -o $@

clean:
	find . | grep "\.o$$" | xargs -I xxx rm -rvf xxx
	find . | grep "\.bin$$" | xargs -I xxx rm -rvf xxx
