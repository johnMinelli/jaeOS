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

pcb_t *pcbfree_h;  //  testa  della  lista dei  PCB  che  sono  liberi o  inutilizzati.
pcb_t pcbFree_table[20]; // array  di PCB

void initPcbs() {
    pcbfree_h =(pcb_t*) malloc(sizeof(pcbFree_table));
}

void freePcb(pcb_t *p){
    if (p != NULL){
        p -> p_next=pcbfree_h;
        pcbfree_h=p;
    }
}

pcb_t *allocPcb(){
    if(pcbfree_h==NULL){
        return NULL;
    }else{
        pcb_t *temp = (pcb_t*) malloc(sizeof(pcb_t));
        temp = pcbfree_h;
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

void insertProcQ(pcb_t **head, pcb_t *p){
    if(p!= NULL){
        if(*head==NULL || (*head)-> priority <= p->priority){
            p -> p_next=*head;
            head=&p;
        }else{
            insertProcQ(&(*head)->p_next,p);
        }
    }
}

pcb_t *headProcQ(pcb_t *head){
    return  head;
}

pcb_t *removeProcQ(pcb_t **head){
    if(*head != NULL ){
        *head = (*head) -> p_next;
        return *head;
    }else{
        return NULL;
    }
}

pcb_t *outProcQ(pcb_t **head, pcb_t *p){
    if(!(*head==NULL || p==NULL)){
        if(*head==p){
            (*head)->p_next = p->p_next;
            //togli
            return p;
        }else if((*head)->p_next==p){
            (*head)->p_next -> p_next = p->p_next;
            //togli
            return p;
        }else{
            return outProcQ(&(*head)->p_next,p);
        }
    }
    return NULL;
}




int main() {
    printf("Hello, World!\n");
    return 0;
}