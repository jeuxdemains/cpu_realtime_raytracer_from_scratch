TARGET=$(shell basename $(CURDIR))

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

CC=g++
CCFLAGS=-Wall -Ofast $(shell pkg-config --cflags-only-I sdl2)
LDFLAGS= $(shell pkg-config --libs sdl2)

all: $(TARGET)
.PHONY: all

$(TARGET): $(OBJECTS)
	@echo LD $@
	@$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp %.h
	@echo CC $*.cpp
	@$(CC) $(CCFLAGS) -c $<

%.o: %.cpp
	@echo CC $*.cpp
	@$(CC) $(CCFLAGS) -c $<

clean:
	@rm -f $(OBJECTS) $(TARGET)
.PHONY: clean
