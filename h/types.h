#ifndef TYPES_H
#define TYPES_H


#include "/usr/include/uarm/uARMtypes.h"

typedef unsigned int cpu_t;
typedef unsigned int memaddr;

typedef struct pcb_t {
    struct pcb_t *p_next;
    struct pcb_t *p_parent;
    struct pcb_t *p_first_child;
    struct pcb_t *p_sib;
    state_t p_s;
    int priority;
    int *p_semKey;
    //////////////
    state_t *oldSys;
    state_t *newSys;
    state_t *oldPgm;
    state_t *newPgm;
    state_t *oldTlb;
    state_t *newTlb;
    cpu_t time;
    cpu_t kernelTime;
    cpu_t userTime;
} pcb_t;


typedef struct semd_t {
    struct semd_t *s_next;
    int *s_key;
    struct pcb_t *s_procQ;
} semd_t;


#endif //TYPES_H
