#include "../h/init.h"


void interruptHandler() {
    unsigned int cause;

    state_t* oldInt = (state_t *) INT_OLDAREA;

    cause = oldInt->CP15_Cause;

    //se l'interruzione si è verificata mentre c'era un processo running devo gestirne il tempo
    if(currentProcess != NULL) {
		currentProcess->userTime += (oldInt->TOD_Low - startTOD);
		kernelStart = oldInt->TOD_Low;
        //ripristino lo stato dall'old interrupt area
		oldInt->pc = oldInt->pc - 4;
        copyState(oldInt, &(currentProcess->p_s));
    }

    //Determino la linea del device ha causato l'interrupt
    if(CAUSE_IP_GET(cause,IL_TIMER)) {
        if((timeLeft <= 0) || timeLeft < QUANTUM){		//pseudoclock tick > devo svegliare TUTTI i processi in WAIT sul tick (dalla syscall)
            pcb_t* temp = removeBlocked(&(deviceSemaphore[MAX_DEVICES-1]));
            while(temp != NULL) {
                temp->p_semKey = NULL;
                softBlockCounter--;		
				temp->p_s.TOD_Low = getTODLO();
                insertProcQ(&(readyQueue), temp);
                temp = removeBlocked(&(deviceSemaphore[MAX_DEVICES-1]));
            }
            //reset del semaforo
            deviceSemaphore[MAX_DEVICES-1] = 0;
            //reset dell'intervallo
            timeLeft = INTERVALTIME;
        }else { 										//il processo ha finito il suo quanto di tempo > lo stoppo e rimetto in readyqueue
            if (currentProcess != NULL) {
                currentProcess->kernelTime += getTODLO() - kernelStart;
				currentProcess->p_s.TOD_Low = getTODLO();
                insertProcQ(&(readyQueue),currentProcess);
                currentProcess = NULL;
            }
        }
        //ritorno allo scheduler con o senza currentProcess
    }
    else if(CAUSE_IP_GET(cause,IL_DISK) ) {
        deviceHandler(IL_DISK);
    }else if(CAUSE_IP_GET(cause,IL_TAPE)) {
        deviceHandler(IL_TAPE);
    }else if(CAUSE_IP_GET(cause,IL_ETHERNET)) {
        deviceHandler(IL_ETHERNET);
    }else if(CAUSE_IP_GET(cause,IL_PRINTER)) {
        deviceHandler(IL_PRINTER);
    }else if(CAUSE_IP_GET(cause,IL_TERMINAL)) {
        deviceHandler(IL_TERMINAL);
    }else {
        PANIC();
    }
    scheduler();
}

/* funzione per sbloccare i processi in wait sul device che ha causato l'interrupt*/
void deviceHandler(int line) {
    unsigned int* bitmap = (unsigned int*)CDEV_BITMAP_ADDR(line);
    int dev = getDeviceFromBitmap(bitmap);
	/* individuato il device sblocco i processi sul semaforo corrispondente */
	/* !!Attenzione se è un terminale dobbiamo distinguere per ricezione o trasmissione */
    int semDev = (EXT_IL_INDEX(line) * N_DEV_PER_IL) + dev;
    if (line == IL_TERMINAL){
		unsigned int* termRcvStatus = DEV_REG_ADDR(line,dev)+(2*WS);
		if ((*termRcvStatus & DEV_TERM_STATUS) == DEV_TTRS_S_CHARTRSM) {
			semDev = semDev + N_DEV_PER_IL;
		}
    }	
    deviceSemaphore[semDev] = deviceSemaphore[semDev] + 1;
    if(deviceSemaphore[semDev] <= 0){		
        pcb_t* temp = removeBlocked(&(deviceSemaphore[semDev]));
        if(temp != NULL){
            temp->p_semKey = NULL;
            softBlockCounter--;
            temp->p_s.a1 = *(unsigned int *)(temp->p_s.a3 - WS);
            temp->p_s.TOD_Low = getTODLO();
            insertProcQ(&(readyQueue), temp);
        }
        pcb_t* next = headBlocked(&deviceSemaphore[semDev]);
        *(unsigned int*) temp->p_s.a3 = DEV_C_ACK;
    }
}

/* aux*/
//data la bitmap della linea del device che ha causato l'interrupt, risalgo al singolo device 
int getDeviceFromBitmap(int *line) {
    int activeBit = 0x00000001;
    int i;
    for (i = 0; i < 8; i++) {
        if ((*line & activeBit) == activeBit) break;
        activeBit = activeBit << 1;
    }
    return i;
}