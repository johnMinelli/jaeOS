#include "../h/init.h"



/*    GLOBAL STUFF      */
 int processCounter;
 int softBlockCounter;
 pcb_t* currentProcess;
 pcb_t* readyQueue;

 int deviceSemaphore[MAX_DEVICES];
 cpu_t timeLeft;
 cpu_t startTOD;
 cpu_t kernelStart;


/* * * * * * * * * * * * */




/* Inizializzazione:
 * aree NEW per interrupt e trap nel frame riservato dalla ROM
 * strutture dati (PCB e ASL)
 * variabili globali di sistema
 * i semafori che vi serviranno nel nucleo
 * il PCB start
 * chiamata allo scheduler
 * */
int main() {
    /* init NEW area */ 
    //0x0000 7000 - 8000 è un area riservata al kernel dove metto gli stati del processore.
	//sono aree new che mi fanno fare il salto all'handler dell'eccezione in caso di *evento* */
    state_t *newArea;

    newArea = (state_t *) TLB_NEWAREA;
    newArea->pc = (unsigned int)tlbHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = STATUS_NULL | STATUS_ID | STATUS_TIMER_ID | STATUS_SYS_MODE;
    newArea->CP15_Control = STATUS_NULL;

    newArea = (state_t *) SYSBK_NEWAREA;
    newArea->pc = (unsigned int)sysCallHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = STATUS_NULL | STATUS_ID | STATUS_TIMER_ID | STATUS_SYS_MODE;
    newArea->CP15_Control = STATUS_NULL;

    newArea = (state_t *) PGMTRAP_NEWAREA;
    newArea->pc = (unsigned int) pgmTrapHandler;
    newArea->sp = RAM_TOP;
    newArea->cpsr = STATUS_NULL | STATUS_ID | STATUS_TIMER_ID | STATUS_SYS_MODE;
    newArea->CP15_Control = STATUS_NULL;

    newArea = (state_t *) INT_NEWAREA;
    newArea->pc = (unsigned int) interruptHandler;
    newArea->sp = RAM_TOP;
    newArea->cpsr = STATUS_NULL | STATUS_ID | STATUS_TIMER_ID | STATUS_SYS_MODE;
    newArea->CP15_Control = STATUS_NULL;


    /*init strutture dati PCB and ASL*/
    initPcbs();
    initASL();

    /*init variabili globali*/
    processCounter = 0;
    softBlockCounter = 0;
    readyQueue = NULL;
    currentProcess = NULL;

    /*init semafori*/
    //Il nucleo mantiene i semafori per tutti i device(doppi per i teminali) più uno per lo pseudo-clock timer.
    //All of these semaphores need to be initialized to zero.
    for (int i = 0; i < MAX_DEVICES; ++i) {
        deviceSemaphore[i] = 0;
    }

    /*Alloco un PCB e riempite i campi*/
    //lo pusho nella readyqueue e poi chiamo lo scheduler -> ci penserà lui ad eseguirlo
    pcb_t *start = allocPcb();
    start->priority = 0;                  
    start->p_s.sp = (RAM_TOP - FRAMESIZE); //ne basterebbero 0x2c0 io ne dedico 0x1000
    start->p_s.pc = (unsigned int) test;
    start->p_s.cpsr = STATUS_NULL | STATUS_SYS_MODE; //interrupt bits 0 -> abilitati
    start->p_s.CP15_Control = STATUS_NULL;
    insertProcQ(&readyQueue, start);
    processCounter++;
    /*  timing   */
    timeLeft = INTERVALTIME;
    startTOD=getTODLO();
    setTIMER(QUANTUM);
    scheduler();
    /*          */
    PANIC();
    return 0;


}

//aux for copy state
void copyState(state_t* src, state_t* dest) {

    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->a4 = src->a4;
    dest->v1 = src->v1;
    dest->v2 = src->v2;
    dest->v3 = src->v3;
    dest->v4 = src->v4;
    dest->v5 = src->v5;
    dest->v6 = src->v6;
    dest->sl = src->sl;
    dest->fp = src->fp;
    dest->ip = src->ip;
    dest->sp = src->sp;
    dest->lr = src->lr;
    dest->pc = src->pc;
    dest->cpsr = src->cpsr;
    dest->CP15_Control = src->CP15_Control;
    dest->CP15_EntryHi = src->CP15_EntryHi;
    dest->CP15_Cause = src->CP15_Cause;
    dest->TOD_Hi = src->TOD_Hi;
    dest->TOD_Low = src->TOD_Low;
}