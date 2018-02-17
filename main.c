#include <stdio.h>
#include <stdlib.h>

#define MAXPROC 20
#define MAXSEMD MAXPROC
#define ASHDSIZE 8



typedef struct pcb_t {
    struct pcb_t *p_next;
    struct pcb_t *p_parent;
    struct pcb_t *p_first_child;
    struct pcb_t *p_sib;
    //state_t p_s;
    int priority;
    int *p_semKey;
} pcb_t;

typedef struct semd_t {
    struct semd_t *s_next;
    int *s_key;
    struct pcb_t *s_procQ;
} semd_t;


pcb_t *pcbfree_h;  //  testa  della  lista dei  PCB  che  sono  liberi o  inutilizzati.
pcb_t pcbFree_table[MAXPROC]; // array  di PCB
semd_t *semdFree_h;
semd_t semd_table[MAXSEMD];
semd_t *semdhash[ASHDSIZE];

void initPcbs() {
    pcbfree_h =(pcb_t*) malloc(sizeof(pcbFree_table));
}

//free di un processo      ERROREEEEEE
void freePcb(pcb_t *p){
    if (p != NULL){             //<----- inserimento in coda e non in testa!!!!
        p -> p_next=pcbfree_h;
        pcbfree_h=p;
    }
}

//alloca un processo prendendo da quelli liberi pcbfree
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
        //priority ??
        temp -> p_semKey = NULL;
        return temp;
    }
}

//inserisce p nella coda di PCB head secondo priority
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

//restituisce il primo PCB della coda di PCB head
pcb_t *headProcQ(pcb_t *head){
    return  head;
}

//rimuove il primo PCB dalla coda di PCB head
pcb_t *removeProcQ(pcb_t **head){
    if(*head != NULL ){
        pcb_t *temp = *head;
        *head = (*head) -> p_next;
        return temp;
    }
    return NULL;
}

//rimuove il processo p dalla coda di PCB head
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

void forallProcQ(pcb_t *head, void fun(pcb_t *pcb, void *), void *arg) {
    if (head != NULL) {
        fun(head, arg);
        forallProcQ(head->p_next, fun, arg);
    }
}

//___________________________________TREE__________________________________________________________________

//inserisce p tra i figli di parent
void insertChild(pcb_t *parent, pcb_t *p){
    p ->p_parent = parent;
    p ->p_sib = parent->p_first_child;
    parent ->p_first_child = p;
}

//rimuove il primo figlio di p
pcb_t *removeChild(pcb_t *p){
    if(p ->p_first_child != NULL){
        pcb_t *temp = p ->p_first_child;
        p ->p_first_child = p ->p_first_child ->p_sib;
        //devo rimuovere i riferimenti di temp?
        return temp;
    }
    return NULL;
}

//rimuove (p) un PCB dalla lista dei figli di suo padre
pcb_t *outChild(pcb_t* p){
    if(p!=NULL && p->p_parent != NULL) {
        return outChild_scan(&(p->p_parent->p_first_child), p);
    }
    return NULL;
}

pcb_t *outChild_scan(pcb_t **head, pcb_t* p){
    if(!(*head==NULL || p==NULL)){
        if(*head==p){
            (*head) = p->p_sib;
            return p;
        }else{
            return outChild_scan(&(*head)->p_sib,p);
        }
    }
    return NULL;
}

//___________________________________SEMAPHORES__________________________________________________________________


//ausiliarie
void setSemd(semd_t **head,semd_t *s) {
    if(*head==NULL){
        *head=s;
    }else{
        setSemd(&(*head)->s_next,s);
    }
}

semd_t *getSemd(semd_t *head,int *key) {
    if (head != NULL) {
        if (head->s_key == key) {
            return head;
        }
        return getSemd(head->s_next, key);
    }
    return NULL;
}

void insertPtoSemQ(pcb_t *head, pcb_t *p){
    if(p!= NULL){
        if(head->p_next==NULL){
            head -> p_next = p;
        }else{
            insertPtoSemQ(head->p_next,p);
        }
    }
}

//inserisce un PCB nella coda dei bloccati del semaforo con key
int insertBlocked(int *key, pcb_t *p) {
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;
    semd_t *find = getSemd(semdhash[n],key);    //((int)(&sem[70])/(int)sizeof(int))%8
    if(find != NULL) {
        p->p_semKey = key;
        insertPtoSemQ(find->s_procQ, p);
        return 0;
    }else {
        if (semdFree_h != NULL) {/////
            //tolgo dai liberi
            semd_t *temp = semdFree_h;
            semdFree_h = semdFree_h->s_next;
            //inizializzo semaforo
            temp->s_next = NULL;
            temp->s_key = key;
            temp->s_procQ = p;  // per ins il proc in coda invece di ---->  insertPtoSemQ(&(temp), p);
            //setto il processo
            p->p_semKey = key;
            //inserisco il semaforo negli usati
            setSemd(&semdhash[n],temp);
            return 0;
        }
        return -1;
    }
}

//restituisce la testa della coda dei PCB bloccati dal semaforo con key
pcb_t *headBlocked(int *key) {
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;
    semd_t *find = getSemd(semdhash[n],key);
    if(find != NULL) {
        return find->s_procQ;
    }
    return NULL;
}

//ausiliarie per free-are un semaforo con coda vuota
void removeSemd(int *key){
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;
    setSemd(&semdFree_h,removeSemd_scan(&semdhash[n],key)); //mette tra i liberi quello che rimuove
}


semd_t *removeSemd_scan(semd_t **head,int *key) {
    if (*head != NULL) {
        if((*head) ->s_key == key){
            semd_t *out = *head;
            (*head) = (*head) ->s_next;
            return out;
        }else{
            return removeSemd_scan(&(*head) -> s_next, key);
        }
    }
    return NULL;
}

//rimuove il PCB in testa alla coda dei bloccati del semaforo che ha key
pcb_t* removeBlocked(int *key){
    pcb_t *q = headBlocked(key); //coda dei processi del semaforo con chiave key
    if(q != NULL){
        pcb_t *out = removeProcQ(&q); //null se non rimuove niente(impossibile) altrimenti puntatore al proc rimosso dalla coda dei bloccati
        if(q != NULL){
            //coda vuota allora il semaforo é inutilizzato
            removeSemd(key);  //<-- se volete si può togliere un metodo lasciando solo  _scan
        }
        return out;
    }
    return NULL;
}

//rimuove il PCB puntato da p dalla coda del semaforo su cui è bloccato
void outChildBlocked(pcb_t *p){
    if(p != NULL){
        pcb_t *q = headBlocked(p -> p_semKey);//null se non trova il semaforo (impossibile)
        outProcQ(&q,p);
        if(q != NULL){
            //coda vuota allora il semaforo é inutilizzato
            removeSemd(p -> p_semKey);  //<-- se volete si può togliere un metodo lasciando solo  _scan
        }
    }

}

void forallBlocked(int *key, void *fun(pcb_t *pcb, void *), void *arg) {
    pcb_t *temp = headBlocked(key);
    if(temp != NULL){
        forallProcQ(temp, fun, arg);
    }
}

void initASL()    //<--  non sono certo della alloc io






        int main() {
                if(-1){

                printf("Hello, World!\n");
                return 0;
            }
}

