CC = arm-none-eabi-gcc
LD =  arm-none-eabi-ld
UARM = /usr/include/uarm/
SOURCE = src/
LIB = $(UARM)crtso.o $(UARM)libuarm.o 
OBJ = init.o interrupts.o scheduler.o exceptions.o asl.o pcb.o p2test.o
ELF = $(UARM)ldscripts/elf32ltsarm.h.uarmcore.x    
FLAGs = -mcpu=arm7tdmi -I $(UARM) -I ./h/ -std=gnu99

all: test clean

test: $(OBJ)
	$(LD) -T $(ELF) -o test $(LIB) $(OBJ)
p2test.o: $(SOURCE)p2test.c
	$(CC) $(FLAGs) -c $(SOURCE)p2test.c
init.o: $(SOURCE)init.c
	$(CC) $(FLAGs) -c $(SOURCE)init.c 
interrupts.o: $(SOURCE)interrupts.c
	$(CC) $(FLAGs) -c $(SOURCE)interrupts.c 
scheduler.o: $(SOURCE)scheduler.c
	$(CC) $(FLAGs) -c $(SOURCE)scheduler.c 
exceptions.o: $(SOURCE)exceptions.c
	$(CC) $(FLAGs) -c $(SOURCE)exceptions.c 
asl.o: $(SOURCE)asl.c
	$(CC) $(FLAGs) -c $(SOURCE)asl.c 
pcb.o: $(SOURCE)pcb.c
	$(CC) $(FLAGs) -c $(SOURCE)pcb.c
clean:
	rm -f *.o
clean-all:
	rm -f *.o test 