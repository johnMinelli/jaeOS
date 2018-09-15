#include "../h/init.h"


/*
	se currentProcess è settato 
		|aggiorna il tempo e lo rimette in esecuzione sul processore
	altrimenti
		|se eseistono processi in readyqueue
		|	|aggiorno il tempo e lo metto in esecuzione
		|altrimenti
		|	|se ci sono processi softblocked da aspettare
		|	|	|li aspetto in WAIT()
		|	|altrimenti 
		|	|	|PANIC()

 **********************************************************************/
void scheduler(){
    if(!currentProcess){
        /*new job*/
        pcb_t *newProc = NULL;
        aging();
        newProc = removeProcQ(&(readyQueue));
        if(!newProc){
            /*non ci sono new process*/
            if(processCounter == 0) HALT();
            else if(processCounter > 0){
                if(softBlockCounter == 0)
					PANIC();
                else if(softBlockCounter > 0){
                    //non ha senso settare il timer ad un quanto perchè la situazione si ripresenterebbe
                    //calcolo e setto il timer al prossimo psudo clock tick
                    timeLeft -= (getTODLO() - startTOD);
					if(timeLeft < 0 || timeLeft > INTERVALTIME) { //se va in complemento
						timeLeft = 0;
					}
					setTIMER(timeLeft);
					//abilito interrupts
					setSTATUS(STATUS_ALL_INT_ENABLE(getSTATUS()));
                    WAIT();
                }
            }
        }else{
            /*Setto currentProcess al new job*/
            currentProcess = newProc;
            /*Store starting TOD*/
            if(currentProcess->time == 0){
                currentProcess->time = getTODLO();
            }
            //Calcolo il tempo rimanente al prossimo psudo clock tick
            timeLeft -= (getTODLO() - startTOD);
			if(timeLeft < 0 || timeLeft > INTERVALTIME) { //se va in complemento
				timeLeft = 0;
			}
            //Se c'è meno di un quanto rimenente sul clock
            if(timeLeft < QUANTUM){
                //Setto il new job timer al tempo rimanente
                setTIMER(timeLeft);
            }else {
                //Setto il new job timer ad un quanto pieno
                setTIMER(QUANTUM);
            }


        }
    }else{     /* rimetto in esecuzione il currentProcess */
		currentProcess->kernelTime += getTODLO() - kernelStart;
		timeLeft -= (getTODLO() - startTOD);
		if(timeLeft < 0 || timeLeft > INTERVALTIME) { //se va in complemento
			timeLeft = 0;
		}
		//Se c'è meno di un quanto rimenente sul clock
		if(timeLeft < QUANTUM){ // ovvero precedentemente era stato fatto setTIMER(timeLeft)
			setTIMER(timeLeft);
		}else {	//ovvero precedentemente era stato fatto setTIMER(QUANTUM)
			setTIMER(QUANTUM); //resetto ad un quanto pieno o riprendo a QUANTUM-tempo passato (ovvero usrTime)?
		}
    }
    startTOD = getTODLO();
    /*Esecuzione sul processore*/
    LDST(&(currentProcess->p_s));
}

	
	
	
/*	Funzione per aumentare la priorità dei processi fino a MAXPRIO che stanno "invecchiando" nella readyqueue */
void aging(){
	cpu_t now;
    pcb_t* next;
    pcb_t* temp = NULL;
	
	unsigned int n = MAXPRIO;
    int bits = 0;
    while(n) {
		bits++;
        n >>= 1;
    }
    now = getTODLO();
    next = readyQueue;
    temp = NULL;
    while(next){
        if(now-next->p_s.TOD_Low>=INTERVALAGE && next->priority < MAXPRIO){
            temp = next;
            next = next->p_next;
            outProcQ(&(readyQueue), temp);
			temp->p_next = NULL;
            while(now-temp->p_s.TOD_Low>=INTERVALAGE && temp->priority < MAXPRIO){
				//la priorità aggiuntiva viene messa nei bit più a sx  [bits_prio_aging|bits_prio_default]
                temp->priority = (1<<bits)+temp->priority;
                temp->userTime += INTERVALAGE;
                temp->p_s.TOD_Low += INTERVALAGE;
            }
            insertProcQ(&(readyQueue),temp);
        }else{
            next = next->p_next;
        }
    }
}