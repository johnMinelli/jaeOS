//
// Created by Gio on 26/07/2018.
//

#include "../h/pcb.h"


pcb_t *pcbfree_h;
pcb_t pcbFree_table[MAXPROC];


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
        //temp->p_s = NULL;
        temp -> priority = 0;
        temp -> p_semKey = NULL;
        temp->kernelTime = 0;
        temp->userTime = 0;
        temp->time = 0;
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
		temp -> p_next = NULL;
		temp -> priority = temp->priority & MASK;
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
			p->p_next = NULL;
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
