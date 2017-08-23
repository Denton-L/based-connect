CC=gcc
CCFLAGS=-Wall
LDFLAGS=-lbluetooth
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXEC=based-connect

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CCFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(EXEC)
