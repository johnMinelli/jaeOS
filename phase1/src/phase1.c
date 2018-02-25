#include <uARMconst.h>
#include <uARMtypes.h>

#include "../h/phase1.h"
#include "../h/constants.h"


pcb_t *pcbfree_h;
pcb_t pcbFree_table[MAXPROC];
semd_t *semdFree_h;
semd_t semd_table[MAXSEMD];
semd_t *semdhash[ASHDSIZE];

/*________________________________________________PCB_________________________________________________*/

/*
 * Funzione ricorsiva per inizializzare la pcbFree_h in modo da contenere tutti gli elementi della
 * pcbFree_table
 */
void initPcbs_scan(int i, pcb_t **p){
    if (i<MAXPROC) {
        *p = &pcbFree_table[i++];
        (*p) -> p_next = NULL;
        (*p) -> p_parent = NULL;
        (*p) -> p_first_child = NULL;
        (*p) -> p_sib = NULL;
        (*p) -> priority = 0;
        (*p) -> p_semKey = NULL;
        initPcbs_scan(i,&(*p)->p_next);
    }
}
void initPcbs() {
    initPcbs_scan(0, &pcbfree_h);
}

/*
 * Inserisce il PCB puntato da p nella lista dei PCB liberi. Inseriamo in testa alla coda dato il
 * minor peso computazionale della funzione
 */
void freePcb(pcb_t *p){
    if (p != NULL){
        p -> p_next=pcbfree_h;
        pcbfree_h=p;
    }
}

/*
 * Alloca un processo rimuovendo un elemento dalla pcbFree ed inizializzando tutti i campi (NULL/0)
 * e restituisce l’elemento rimosso. Nel caso non ci siano elementi in pcbFree restituisco NULL
 */
pcb_t *allocPcb(){
    if(pcbfree_h==NULL){
        return NULL;
    }else{
        pcb_t *temp = pcbfree_h;
        pcbfree_h = temp -> p_next;
        temp -> p_next = NULL;
        temp -> p_parent = NULL;
        temp -> p_first_child = NULL;
        temp -> p_sib = NULL;
        //stato ??
        temp -> priority = 0;
        temp -> p_semKey = NULL;
        return temp;
    }
}

/*
 * Inserisce l’elemento puntato da p nella coda dei processi puntata da head tenendo conto della priority
 * [testa=priority++++       coda=priority----]
 */
void insertProcQ(pcb_t **head, pcb_t *p){
    if(p!= NULL){
        if(*head==NULL || (*head)-> priority <= p->priority){
            p -> p_next=*head;
            *head=p;
        }else{
            insertProcQ(&(*head)->p_next,p);
        }
    }
}

//Restituisce il primo PCB della coda di PCB (quindi la testa)
pcb_t *headProcQ(pcb_t *head){
    return  head;
}

/*
 * Rimuove il primo elemento dalla coda di PCB e ritorna lo stesso elemento
 */
pcb_t *removeProcQ(pcb_t **head){
    if(*head != NULL ){
        pcb_t *temp = *head;
        *head = (*head) -> p_next;
        return temp;
    }
    return NULL;
}

/*
 * Ricerca e rimuove il PCB puntato da p dalla coda dei PCB
 */
pcb_t *outProcQ(pcb_t **head, pcb_t *p){
    if(!(*head==NULL || p==NULL)){
        if(*head==p){
            (*head) = p->p_next;
            return p;
        }else{
            return outProcQ(&(*head)->p_next,p);
        }
    }
    return NULL;
}

//Richiama la funzione [fun] per ogni elemento della lista di PCB
void forallProcQ(pcb_t *head, void fun(pcb_t *pcb, void *), void *arg) {
    if (head != NULL) {
        fun(head, arg);
        forallProcQ(head->p_next, fun, arg);
    }
}

/*________________________________________________TREE________________________________________________*/

/*
 *  Funzione ausiliaria per ottenere l'ultimo elemento nella lista di figli un PCB
 * (quindi l'ultimo fratello del PCB puntato da p)
 */
pcb_t *lastSib(pcb_t *p){
    if(p->p_sib != NULL){
        return lastSib(p->p_sib);
    }
    return p;
}

/*
 *  Inserisce il PCB puntato da p in fondo alla lista dei figli del PCB puntato da parent.
 */
void insertChild(pcb_t *parent, pcb_t *p){
    p -> p_parent = parent;
    if(parent -> p_first_child == NULL){
        parent -> p_first_child = p;
    }else{
        lastSib(parent -> p_first_child) -> p_sib = p;
    }
}

/*
 *  Rimuove il primo figlio del PCB puntato da p
 */
pcb_t *removeChild(pcb_t *p){
    if(p ->p_first_child != NULL){
        pcb_t *temp = p ->p_first_child;
        p ->p_first_child = p ->p_first_child ->p_sib;
        temp ->p_parent = NULL;
        temp ->p_sib = NULL;
        return temp;
    }
    return NULL;
}

/*
 *  Funzione ricorsiva per rimuovere il PCB puntato da p dalla lista dei figli del PCB puntato da parent
 */
pcb_t* outChild_scan(pcb_t **head, pcb_t* p){
    if(!(*head==NULL || p==NULL)){
        if(*head==p){
            (*head) = p->p_sib;
            p ->p_parent = NULL;
            p ->p_sib = NULL;
            return p;
        }else{
            return outChild_scan(&(*head)->p_sib,p);
        }
    }
    return NULL;
}

pcb_t *outChild(pcb_t* p){
    if(p!=NULL && p->p_parent != NULL) {
        return outChild_scan(&(p->p_parent->p_first_child), p);
    }
    return NULL;
}

/*________________________________________________SEMD________________________________________________*/

/*
 *  Funzione ausiliaria che inserisce il semaforo s in fondo alla lista di semafori di cui head è
 *  la testa.
 */
void setSemd(semd_t **head,semd_t *s) {
    if(*head==NULL){
        *head=s;
    }else{
        setSemd(&(*head)->s_next,s);
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
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;     //determino la riga dell'hashtable
    semd_t *find = getSemd(semdhash[n],key);
    if(find != NULL) {
        return find->s_procQ;
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
    pcb_t *q = headBlocked(key);    //puntatore alla testa della coda dei PCB bloccati dal semaforo con chiave key
    if(q != NULL){
        pcb_t *out = removeProcQ(&q);
        if(q == NULL){  //coda vuota -> il semaforo é inutilizzato
            removeSemd(key);
        }
        return out;
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
    }

}


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
