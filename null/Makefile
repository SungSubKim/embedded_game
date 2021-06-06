CC=arm-linux-g++
CFLAGS=-g -Wall
TARGET=game
OBJS=main.o
CONSTS=level*.h font.h background.h gpio.h rpi_logo.h skku.h
MYLIB=ssd1306.h

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

main.o:
	$(CC) main.cpp -c -o main.o

clean:
	rm $(OBJS) $(TARGET)
