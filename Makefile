CC=gcc
CP=cp
RM=rm
CFLAGS=-c -Wall
LDFLAGS=
CPFLAGS=-rf
RMFLAGS=-rf
LIBS=-lpng
PREFIX=/usr
SRC_DIR=src/pconvert
SOURCES=$(SRC_DIR)/stdafx.c $(SRC_DIR)/pconvert.c $(SRC_DIR)/util.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pconvert

all: $(SOURCES) $(EXECUTABLE)

install: all
	$(CP) $(EXECUTABLE) $(PREFIX)/bin

clean:
	$(RM) $(RMFLAGS) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
