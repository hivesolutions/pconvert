-include conanbuildinfo.mak

CC=gcc
CPP=g++
LD=$(CC)
CP=cp
RM=rm
SYS=posix
OPTIMIZATION=-O3 -finline-functions -Winline
CFLAGS=$(OPTIMIZATION) -c -Wall
LDFLAGS=-L/usr/local/lib
CPFLAGS=-rf
RMFLAGS=-rf
LIBS=-lm -lpng
PREFIX=/usr
SRC_DIR=src/pconvert
SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pconvert

# updates the internal variables with the values comming
# from the conan package manager
CFLAGS+=$(CONAN_CFLAGS)
CFLAGS+=$(addprefix -I, $(CONAN_INCLUDE_DIRS))
CFLAGS+=$(addprefix -D, $(CONAN_DEFINES))
LDFLAGS+=$(addprefix -L, $(CONAN_LIB_DIRS))
LIBS+=$(addprefix -l, $(CONAN_LIBS))

ifeq ($(SYS),darwin)
  CC=clang
  CPP=clang
  LDFLAGS+=-framework Security
endif

ifeq ($(DEBUG),1)
  CFLAGS+=-g
  LDFLAGS+=-g
endif

ifeq ($(EXTENSION),1)
  CFLAGS+=-DPCONVERT_EXTENSION -I/usr/local/include -I/usr/include/python$(PYTHON_VERSION) -I/usr/local/include/python$(PYTHON_VERSION) $(shell python-config --includes)
  LIBS+=-lpython
  PYTHON_VERSION=2.7
endif

ifeq ($(OPENCL),1)
  CFLAGS+=-DPCONVERT_OPENCL
  ifeq ($(SYS),darwin)
    LDFLAGS+=-framework OpenCL
  else
    LIBS+=-lopencl
  endif
endif

all: $(SOURCES) $(EXECUTABLE)

install: all
	$(CP) $(EXECUTABLE) $(PREFIX)/bin

clean:
	$(RM) $(RMFLAGS) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
