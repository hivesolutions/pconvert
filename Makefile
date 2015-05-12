CC=gcc
RM=rm
CFLAGS=-c -Wall
LDFLAGS=-lpng
RMFLAGS=-rf
SRC_DIR=src/pconvert
SOURCES=$(SRC_DIR)/stdafx.c $(SRC_DIR)/pconvert.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pconvert

all: $(SOURCES) $(EXECUTABLE)

clean:
	$(RM) $(RMFLAGS) $(OBJECTS) 

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
