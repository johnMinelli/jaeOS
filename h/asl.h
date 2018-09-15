#ifndef ASL_H
#define ASL_H


#include "/usr/include/uarm/uARMconst.h"
#include "/usr/include/uarm/uARMtypes.h"
#include "const.h"
#include "types.h"

void setSemd(semd_t **head,semd_t *s);
semd_t *getSemd(semd_t *head,int *key);
void insertPtoSemQ(pcb_t *head, pcb_t *p);
int insertBlocked(int *key, pcb_t *p);
pcb_t *headBlocked(int *key);
semd_t *removeSemd_scan(semd_t **head,int *key);
void removeSemd(int *key);
pcb_t* removeBlocked(int *key);
void outChildBlocked(pcb_t *p);
void forallBlocked(int *key, void fun(pcb_t *pcb, void *), void *arg);
void initASL_scan(int i, semd_t **s);
void initASL();


#endif //ASL_H
