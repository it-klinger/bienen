# Makefile

SOURCES=spike.c
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=spike

LDFLAGS += -lm
CFLAGS += -g3 -O0

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(EXECUTABLE) *.o

install: $(EXECUTABLE)
	mkdir -p $(DESTDIR)/$(BINDIR)
	install -m 0755 $< $(DESTDIR)/$(BINDIR)


copy:
	scp $(EXECUTABLE) root@192.168.0.103:/usr/bin/

