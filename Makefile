CC = gcc
CCFLAGS = -Wall -MMD -O
LDFLAGS = -lbluetooth
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
DEPENDS = $(OBJECTS:.o=.d)
EXEC = based-connect

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CCFLAGS)

-include $(DEPENDS)

.PHONY: clean

clean:
	rm -f *.o *.d $(EXEC)
