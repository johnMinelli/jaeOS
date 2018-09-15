#ifndef INIT_H
#define INIT_H


#include "/usr/include/uarm/libuarm.h"
#include "/usr/include/uarm/arch.h"
#include "/usr/include/uarm/uARMconst.h"
#include "/usr/include/uarm/uARMtypes.h"
#include "const.h"
#include "types.h"
#include "pcb.h"
#include "asl.h"

extern int processCounter;
extern int softBlockCounter;
extern pcb_t* currentProcess;
extern pcb_t* readyQueue;

extern int deviceSemaphore[MAX_DEVICES];
extern cpu_t timeLeft;
extern cpu_t startTOD;
extern cpu_t kernelStart;

extern void test();
extern void copyState(state_t* src,state_t* dest);
extern void tlbHandler();
extern void pgmTrapHandler();
extern void sysCallHandler();
extern void interruptHandler();
extern void timingHandler();
extern void scheduler();


#endif //INIT_H


