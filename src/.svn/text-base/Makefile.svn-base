# linuxaldl Makefile

CC = gcc
CFLAGS = -g -W -Wall -Wno-unused `pkg-config --cflags gtk+-2.0`

V = @

all: linuxaldl

sts_serial.o: sts_serial.c
	@echo + cc sts_serial.c
	$(V)$(CC) $(CFLAGS) -c sts_serial.c

linuxaldl.o: linuxaldl.c
	@echo + cc linuxaldl.c
	$(V)$(CC) $(CFLAGS) -c linuxaldl.c

linuxaldl_gui.o: linuxaldl_gui.c
	@echo + cc linuxaldl_gui.c
	$(V)$(CC) $(CFLAGS) -c linuxaldl_gui.c

linuxaldl: linuxaldl.o linuxaldl_gui.o sts_serial.o
	@echo + link main
	$(V)$(CC) $(CFLAGS) -lpopt `pkg-config --libs gtk+-2.0` -o ../bin/$@ linuxaldl.o linuxaldl_gui.o sts_serial.o

clean:
	@echo + clean
	$(V)rm -rf *.o ../bin/linuxaldl
