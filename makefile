#  Guy Ginat 206922544, Ron Hadad 209260645
CC = gcc
CFLAGS = -m32 -Wall

TARGET = myELF

all: $(TARGET)

$(TARGET): myELF.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

