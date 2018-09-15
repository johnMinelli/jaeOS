#ifndef PCB_H
#define PCB_H


#include "/usr/include/uarm/uARMconst.h"
#include "/usr/include/uarm/uARMtypes.h"
#include "const.h"
#include "types.h"

void initPcbs_scan(int i, pcb_t **p);
void initPcbs();
void freePcb(pcb_t *p);
pcb_t *allocPcb();
void insertProcQ(pcb_t **head, pcb_t *p);
pcb_t *headProcQ(pcb_t *head);
pcb_t *removeProcQ(pcb_t **head);
pcb_t *outProcQ(pcb_t **head, pcb_t *p);
void forallProcQ(pcb_t *head, void fun(pcb_t *pcb, void *), void *arg);
pcb_t *lastSib(pcb_t *p);
void insertChild(pcb_t *parent, pcb_t *p);
pcb_t *removeChild(pcb_t *p);
pcb_t* outChild_scan(pcb_t **head, pcb_t* p);
pcb_t *outChild(pcb_t* p);


#endif //PCB_H
