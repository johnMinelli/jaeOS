CC = arm-none-eabi-gcc
LD =  arm-none-eabi-ld
UARM = /usr/include/uarm/
SOURCE = src/
LIB = $(UARM)crtso.o $(UARM)libuarm.o 
OBJ = test.o phase1.o
ELF = $(UARM)ldscripts/elf32ltsarm.h.uarmcore.x    
FLAGs = -mcpu=arm7tdmi -I $(UARM) -I ./h/

all: test 

test: $(OBJ)
	$(LD) -T $(ELF) -o test $(LIB) $(OBJ)
test.o: test.c
	$(CC) $(FLAGs) -c test.c
phase1.o: $(SOURCE)phase1.c
	$(CC) $(FLAGs) -c $(SOURCE)phase1.c 
