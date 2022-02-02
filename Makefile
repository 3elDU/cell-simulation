CC = gcc

LDFLAGS = -lSDL2 -lSDL2_image

SOURCES = src/*
OBJECTS = $(SOURCES:.c=.o)
TARGET = cells

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS) core

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

install:
	install -d ${DESTDIR}${PREFIX}/bin
	install -m 755 cells $(DESTDIR)$(PREFIX)/bin/cells
