# Makefile
#
# suitable for buildroot
#

SOURCES=bienen.c bw-global.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=bienen

CC = $(CROSS_COMPILE)gcc
LDFLAGS += -lconfig -lrt -lm -lpq
CFLAGS += -g3 -O0

#!!!!!!!!!
# nur fuer sandbox build
# CC = $(CROSS_COMPILE)gcc
# LDFLAGS += -L/opt/arm-br-bw-cortexa8-glibc/arm-buildroot-linux-gnueabihf/sysroot/usr/lib/
# CFLAGS += -I/opt/arm-br-bw-cortexa8-glibc/arm-buildroot-linux-gnueabihf/sysroot/usr/include
#!!!!!!!!!

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

all: $(SOURCES) $(EXECUTABLE) bw-global.h

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(EXECUTABLE) *.o

install: $(EXECUTABLE) 	# $(EXECUTABLE2)
	mkdir -p $(DESTDIR)/$(BINDIR)
	install -m 0755 $< $(DESTDIR)/$(BINDIR)

copy:
	scp $(EXECUTABLE) root@192.168.0.118:/opt/bienen/bin/

