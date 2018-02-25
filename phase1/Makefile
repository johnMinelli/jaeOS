#Compiler
CC = arm-none-eabi-gcc
#Linker
LC =  arm-none-eabi-ld

UARM_PATH = /usr/include/uarm/
SOURCE_PATH = src/
LIB = $(UARM_PATH)crtso.o $(UARM_PATH)libuarm.o
OBJ = test.o phase1.o
ELF = $(UARM_PATH)ldscripts/elf32ltsarm.h.uarmcore.x
COMP_FLAGS = -mcpu=arm7tdmi -I $(UARM_PATH) -I ./h/

all: test clean

test: $(OBJ)
	$(LC) -T $(ELF) -o test $(LIB) $(OBJ)
test.o: test.c
	$(CC) $(COMP_FLAGS) -c test.c
phase1.o: $(SOURCE_PATH)phase1.c
	$(CC) $(COMP_FLAGS) -c $(SOURCE_PATH)phase1.c

clean:
	rm -f *.o
