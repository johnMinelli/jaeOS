//
// Created by Gio on 26/07/2018.
//

#include "../h/asl.h"
#include "../h/pcb.h"


semd_t *semdFree_h;
semd_t semd_table[MAXSEMD];
semd_t *semdhash[ASHDSIZE];

/*________________________________________________SEMD________________________________________________*/

/*
 *  Funzione ausiliaria che inserisce il semaforo s in fondo alla lista di semafori di cui head è
 *  la testa.
 */
void setSemd(semd_t **head,semd_t *s) {
    if(*head==NULL){
        *head=s;
    }else{
        setSemd(&((*head)->s_next),s);
    }
}

/*
 *  Funzione ausiliaria che restituisce il semaforo con la chiave indicata, cercandolo nella lista di
 *  semafori di cui head è la testa. Tale lista è contenuta all'interno di una riga PREDETERMINATA
 *  dell'hashtable di semafori (semdhash)
 */
semd_t *getSemd(semd_t *head,int *key) {
    if (head != NULL) {
        if (head->s_key == key) {
            return head;
        }
        return getSemd(head->s_next, key);
    }
    return NULL;
}

/*
 *  Funzione ausiliaria per inserire il PCB puntato da p in fondo alla coda dei processi bloccati
 *  [a differenza di 'insertProcQ()' qui non tengo conto della priority]
 */
void insertPtoSemQ(pcb_t *head, pcb_t *p){
    if(p!= NULL){
        if(head->p_next==NULL){
            head -> p_next = p;
        }else{
            insertPtoSemQ(head->p_next,p);
        }
    }
}

/*
 * Viene inserito il PCB puntato da p nella coda dei processi bloccati associata al semaforo con
 * chiave  key.(1)
 * (2)SE il semaforo con chiave key non esiste -> ne alloco ed inizializzo uno attingendo dalla semdFree
 * (3)SE la semdFree è vuota -> restituisce -1
 */
int insertBlocked(int *key, pcb_t *p) {
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;     //determino la riga dell'hashtable
    semd_t *find = getSemd(semdhash[n],key);
	//p->p_next = NULL;
    if(find != NULL) {  //(1)
        p->p_semKey = key;
        insertPtoSemQ(find->s_procQ, p);
        return 0;
    }else {
        if (semdFree_h != NULL) {   //(2)

            semd_t *temp = semdFree_h;
            semdFree_h = semdFree_h->s_next;

            temp->s_next = NULL;
            temp->s_key = key;
            temp->s_procQ = p;

            p->p_semKey = key;

            setSemd(&semdhash[n],temp);
            return 0;
        }
        return -1;  //(3)
    }
}

/*
 *  Rstituisce un puntatore al primo PCB bloccato dal semaforo con chiave key
 */
pcb_t *headBlocked(int *key) {
    if(key != NULL){
        int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;     //determino la riga dell'hashtable
        semd_t *find = getSemd(semdhash[n],key);
        if(find != NULL) {
            return find->s_procQ;
        }
        return NULL;
    }
    return NULL;
}

/*
 *  Funzione ausiliaria ricorsiva che restituisce il semaforo con la chiave indicata, cercandolo e
 *  rimuovendolo dalla lista di semafori di cui head è la testa. Tale lista è contenuta all'interno
 *  di una riga PREDETERMINATA dell'hashtable di semafori (semdhash)
 */
semd_t *removeSemd_scan(semd_t **head,int *key) {
    if (*head != NULL) {
        if((*head) ->s_key == key){
            semd_t *out = *head;
            (*head) = (*head) ->s_next;
            out -> s_next = NULL;
            return out;
        }else{
            return removeSemd_scan(&(*head) -> s_next, key);
        }
    }
    return NULL;
}
// Funzione ausiliaria per liberare un semaforo
void removeSemd(int *key){
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;         //determino la riga dell'hashtable
    setSemd(&semdFree_h,removeSemd_scan(&semdhash[n],key)); //inserisce tra i liberi il semd rimosso dall'hashtable
}

/*
 *  Rimuove e restituisce il primo PCB dalla coda dei processi bloccati dal semaforo con chiave key
 *  SE la coda dei processi bloccati dal semaforo diventa vuota, libero il semaforo
 *  [fuori dall'hashtable, dentro semdFree]
 */
pcb_t* removeBlocked(int *key){
    //pcb_t *q = headBlocked(key);    //puntatore alla testa della coda dei PCB bloccati dal semaforo con chiave key
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;
    semd_t *find = getSemd(semdhash[n],key);
    if(find != NULL) {
        if(find->s_procQ != NULL){
            pcb_t *out = removeProcQ(&find->s_procQ);
            if(find->s_procQ == NULL){  //coda vuota -> il semaforo é inutilizzato
                removeSemd(key);
            }
            return out;
        }
    }
    return NULL;
}

/*
 *  Rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato
 *  SE la coda dei processi bloccati dal semaforo diventa vuota, libero il semaforo
 */
void outChildBlocked(pcb_t *p){
    if(p != NULL){
        pcb_t *q = headBlocked(p -> p_semKey);  //puntatore alla testa della coda dei PCB bloccati
        outProcQ(&q,p);
        if(q == NULL){  //coda vuota -> il semaforo é inutilizzato
            removeSemd(p -> p_semKey);
        }
		p->p_semKey = NULL;
    }

}

/*
 *  Richiama la funzione [fun] per ogni processo bloccato dal semaforo con chiave key
 */
void forallBlocked(int *key, void fun(pcb_t *pcb, void *), void *arg) {
    pcb_t *temp = headBlocked(key);
    if(temp != NULL){
        forallProcQ(temp, fun, arg);
    }
}

/*
 *  Funzione ricorsiva per inizializzare la lista dei semdFree in modo da contenere tutti gli
 *  elementi della semdTable
 */
void initASL_scan(int i, semd_t **s){
    if (i<MAXSEMD) {
        *s = &semd_table[i++];
        (*s) -> s_next = NULL;
        (*s) -> s_key = NULL;
        (*s) -> s_procQ = NULL;
        initASL_scan(i,&(*s)->s_next);
    }
}

void initASL() {
    initASL_scan(0, &semdFree_h);
}
