CC=gcc
CP=cp
RM=rm
CFLAGS=-c -Wall -I/usr/include/python$(PYTHON_VERSION)
LDFLAGS=-framework OpenCL
CPFLAGS=-rf
RMFLAGS=-rf
LIBS=-lm -lpng
PREFIX=/usr
SRC_DIR=src/pconvert
SOURCES=$(SRC_DIR)/stdafx.c $(SRC_DIR)/pconvert.c $(SRC_DIR)/structs.c $(SRC_DIR)/util.c $(SRC_DIR)/opencl.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pconvert
PYTHON_VERSION=2.7

all: $(SOURCES) $(EXECUTABLE)

install: all
	$(CP) $(EXECUTABLE) $(PREFIX)/bin

clean:
	$(RM) $(RMFLAGS) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
