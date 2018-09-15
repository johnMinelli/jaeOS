#include "../h/init.h"


void tlbHandler(){
    passUpOrDie(TLB);
}
void pgmTrapHandler(){
    passUpOrDie(PGMTRAP);
}

void sysCallHandler(){
    //ripristino old state pre-syscall 
    state_t* oldState = (state_t*) SYSBK_OLDAREA;
    copyState(oldState, &(currentProcess->p_s));
	
	//a seconda della syscall che ha causato l'interruzione agisco di conseguenza
    if(oldState->a1 >= CREATEPROCESS && oldState->a1 <= WAITCHLD){
        //If the state was in system mode
        if((getSTATUS() & STATUS_SYS_MODE) == STATUS_SYS_MODE){
            //gestisco le 10 systemcall di sistema e quindi anche il tempo
			currentProcess->userTime += (oldState->TOD_Low - startTOD);
			kernelStart = oldState->TOD_Low;
            switch(oldState->a1) {
                case CREATEPROCESS:
                    sys1();
					break;
                case TERMINATEPROCESS: {
                    pcb_t *target = currentProcess->p_s.a2;
                    sys2(target);
                }break;
                case SEMP:
                    sys3();
                    break;
                case SEMV:
                    sys4();
                    break;
                case SPECHDL:
                    sys5();
                    break;
                case GETTIME:
					sys6();
                    break;
                case WAITCLOCK:
                    sys7();
					break;
                case IODEVOP: 
					sys8();
					break;
                case GETPIDS:
                    sys9();
					break;
                case WAITCHLD:
					sys10();
					break;
            }
            scheduler();
        }else{
            //error - syscall admin chiamate in usrmode
            state_t* oldPGMState = (state_t*) PGMTRAP_OLDAREA;
            copyState(oldState, oldPGMState);
            oldPGMState->CP15_Cause = EXC_RESERVEDINSTR;
            pgmTrapHandler();
        }
    }else{
        //Syscall 10+
        passUpOrDie(SYSBK);
    }
}







/* CREATEPROCESS */
void sys1() {
    pcb_t *newProc = allocPcb();
    if (!newProc) {
        //error - no free pcbs
        currentProcess->p_s.a1 = SCFAIL;
    } else {
        currentProcess->p_s.a1 = SCSUCC;

        processCounter++;
		//il secondo argomento contiene lo stato del nuovo processo
		copyState(currentProcess->p_s.a2, &(newProc->p_s));
		*(pcb_t**)currentProcess->p_s.a4 = newProc;
        newProc->time = getTODLO();
		newProc->priority = currentProcess->p_s.a3;
        newProc->p_s.TOD_Low = getTODLO();
        //aggiorno la pcb tree
        insertChild(currentProcess, newProc);
        insertProcQ(&readyQueue, newProc);
        
    }
}

/* TERMINATEPROCESS */
void sys2(pcb_t* target){
    if (target && target!=currentProcess) {
        //termino processo target(e i child ricorsivamente) e ritorno 0 o -1 in caso di FAIL
        pcb_t* parentSem = target->p_parent;
        outChild(target);
        terminateProc_scan(target);
        currentProcess->p_s.a1 = SCSUCC;
        //eventually sblocco il padre
        pcb_t* temp = removeBlocked((int*)parentSem);
        if(temp != NULL) {
            temp->p_semKey = NULL;
            temp->p_s.TOD_Low = getTODLO();
            insertProcQ(&(readyQueue), temp);
        }
    } else {
		//termino currentProcess(e i child ricorsivamente) e richiamo lo scheduler con currentProcess = NULL
        int* parentSem = (int*) currentProcess->p_parent;
        outChild(currentProcess);
        terminateProc_scan(currentProcess);
        currentProcess = NULL;
        //eventually sblocco il padre
        pcb_t* temp = removeBlocked(parentSem);
        if(temp != NULL) {
            temp->p_semKey = NULL;
            temp->p_s.TOD_Low = getTODLO();
            insertProcQ(&(readyQueue), temp);
        }
    }
}

/* SEMP */
void sys3(){
    int *semAddress = (int *) currentProcess->p_s.a2;
    //normale routine di P
    *semAddress = *semAddress - 1;
    if (*semAddress < 0) {
        //blocco il processo chiamante
		currentProcess->kernelTime += getTODLO() - kernelStart;
        insertBlocked(semAddress,currentProcess); //e se ritorna -1 (ovvero non ci sono semafori liberi)?
        currentProcess = NULL;
    }
}

/* SEMV */
void sys4(){
    int *semAddress = (int *) currentProcess->p_s.a2;
    //normale routine di V
    *semAddress = *semAddress + 1;
    if (*semAddress <= 0) {
        //sblocco il processo
        pcb_t *temp = removeBlocked(semAddress);
        if(temp != NULL) {
            temp->p_semKey = NULL;
            temp->p_s.TOD_Low = getTODLO();
            //lo metto nella readyQueue
            insertProcQ(&(readyQueue), temp);
        }
    }
}

/* SPECHDL */
void sys5(){
	//setto le aree specifiche di gestione delle trap/interrupt/syscall
    switch(currentProcess->p_s.a2) {
        case SYSBK:
		{
            if(!currentProcess->newSys) {
                currentProcess->newSys = (state_t*)currentProcess->p_s.a4;
                currentProcess->oldSys = (state_t*)currentProcess->p_s.a3;
                currentProcess->p_s.a1 = 0;
            }else{
                currentProcess->p_s.a1 = -1;
            }
		}break;
        case PGMTRAP:
		{
			if(!currentProcess->newPgm) {
                currentProcess->newPgm = (state_t*)currentProcess->p_s.a4;
                currentProcess->oldPgm = (state_t*)currentProcess->p_s.a3;
                currentProcess->p_s.a1 = 0;
            }else{
                currentProcess->p_s.a1 = -1;
            }
		}break;
        case TLB:
		{
			if(!currentProcess-> newTlb) {
                currentProcess->newTlb = (state_t*)currentProcess->p_s.a4;
                currentProcess->oldTlb = (state_t*)currentProcess->p_s.a3;
                currentProcess->p_s.a1 = 0;
            }else{
                currentProcess->p_s.a1 = -1;
            }
		}break;
    }
}

/* GETTIME */
void sys6(){
	currentProcess->kernelTime += (getTODLO() - kernelStart);
	*(unsigned int *)(currentProcess->p_s.a2) = currentProcess->userTime;
	*(unsigned int *)(currentProcess->p_s.a3) = currentProcess->kernelTime;
	*(unsigned int *)(currentProcess->p_s.a4) = getTODLO()-currentProcess->time;
	kernelStart = getTODLO();
}

/* WAITCLOCK */
void sys7(){
	//blocco il processo sull prossimo tick
	deviceSemaphore[MAX_DEVICES - 1]--;
	
	if (deviceSemaphore[MAX_DEVICES - 1] < 0) {
		//soft block it
		currentProcess->kernelTime += getTODLO() - kernelStart;
		insertBlocked(&(deviceSemaphore[MAX_DEVICES - 1]), currentProcess);
		currentProcess = NULL;
		softBlockCounter++;
	}else{
		//error
		PANIC();
	}
}

/* IODEVOP */
void sys8(){
	/* identifico il semaforo corrispondente al device */
		//(MACRO) semDev = DEVPERINT*(oldState->a2 - DISKINT) + oldState->a3;
	/* determino device e interrupt line */
		//(MACRO) devAddrBase = 0x0000.0040 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10)
		//(MACRO) DEV_REG_ADDR(line, dev) (DEV_REG_START + ((line) - DEV_IL_START) * N_DEV_PER_IL * DEV_REG_SIZE + (dev) * DEV_REG_SIZE)
	int line = (currentProcess->p_s.a3 - DEV_REG_START) /
			   (N_DEV_PER_IL * DEV_REG_SIZE); //già decrementata di DEV_IL_START(3)
	int dev = (currentProcess->p_s.a3 - DEV_REG_START - line * N_DEV_PER_IL * DEV_REG_SIZE) / DEV_REG_SIZE;
	int semDev = (N_DEV_PER_IL * line) + dev;
	//If is a terminal && is a transmit terminal
	if ((EXT_IL_INDEX(IL_TERMINAL) == line) && (currentProcess->p_s.a3 % DEV_REG_SIZE) >= 0x08) {
		semDev = semDev + N_DEV_PER_IL;
	}
	//gestisco la routine come una semp
	deviceSemaphore[semDev] = deviceSemaphore[semDev] - 1;
	/* Blocco il processo sul semaforo fino all'interrupt */
	currentProcess->kernelTime += getTODLO() - kernelStart;
	insertBlocked(&(deviceSemaphore[semDev]), currentProcess);
	softBlockCounter++;
	if (deviceSemaphore[semDev] >= -1) { //device ready to accept command
		*(unsigned int *) currentProcess->p_s.a3 = (unsigned int) currentProcess->p_s.a2;
	}
	currentProcess = NULL;
}

/* GETPIDS */
void sys9(){
	if(currentProcess) {
		*(pcb_t**)(currentProcess->p_s.a2) = currentProcess;
		if (currentProcess->p_parent) {
			*(pcb_t**)(currentProcess->p_s.a3) = currentProcess->p_parent;
		}
	}	
}

/* WAITCHLD */
void sys10(){
	if(currentProcess->p_first_child){
		//gestisco la routine come semp
		currentProcess->kernelTime += getTODLO() - kernelStart;   //non aggiorno il valore del semaforo tanto ne blocco uno solo
		insertBlocked((int *) currentProcess, currentProcess); //e se ritorna -1 (ovvero non ci sono semafori liberi)?
		currentProcess = NULL;
	}
}








void passUpOrDie(int type){
    switch (type){
        case TLB:
		{
            if(currentProcess->oldTlb){
                state_t* oldState = (state_t*) TLB_OLDAREA;
				currentProcess->userTime += (oldState->TOD_Low - startTOD);
				kernelStart = oldState->TOD_Low;
                copyState(oldState,  currentProcess->oldTlb);
                //passUp
                copyState(currentProcess->newTlb,  &(currentProcess->p_s));
                scheduler();
            }else{
                sys2(NULL);
            }
		}break;
        case SYSBK:
		{
            if(currentProcess->oldSys){
                state_t* oldState = (state_t*) SYSBK_OLDAREA;
				currentProcess->userTime += (oldState->TOD_Low - startTOD);
				kernelStart = oldState->TOD_Low;
                copyState(oldState,  currentProcess->oldSys);
                //passUp
                copyState(currentProcess->newSys,  &(currentProcess->p_s));
                scheduler();
            }else{
                sys2(NULL);
            }
		}break;
        case PGMTRAP:
		{
            if(currentProcess->oldPgm){
                state_t* oldState = (state_t*) PGMTRAP_OLDAREA;
				currentProcess->userTime += (oldState->TOD_Low - startTOD);
				kernelStart = oldState->TOD_Low;
                copyState(oldState,  currentProcess->oldPgm);
                //passUp
                copyState(currentProcess->newPgm,  &(currentProcess->p_s));
                scheduler();
            }else{
                sys2(NULL);
            }
		}break;
        default:
            sys2(NULL);
    }
	scheduler();
}


/* aux TERMINATEPROCESS */ 
//il target viene rimosso dai figli del padre
//per ogni figlio del target:
  //     - se è soft blocked     o    in readyQueue   gestisco la coda dei next
  //     - faccio la free della memoria

void terminateProc_scan(pcb_t* target){

    pcb_t* target_child = removeChild(target);

    while(target_child){
        terminateProc_scan(target_child);
		target_child = removeChild(target);
    }
	//controllo se è fermo su un semaforo
    if(!target->p_semKey){
        //non è nel semaforo quindi è ready
        outProcQ(&(readyQueue), target);
    }else{
		//sblocco dal semaforo
        outChildBlocked(target);
        if((target->p_semKey >= &(deviceSemaphore[0])) && (target->p_semKey <= &(deviceSemaphore[MAX_DEVICES-1]))){
            softBlockCounter--;
        }
        *(target->p_semKey) = *(target->p_semKey) + 1;
    }
    freePcb(target);
    processCounter--;
    return;
}