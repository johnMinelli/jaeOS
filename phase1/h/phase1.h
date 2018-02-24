#ifndef PHASE1
#define PHASE1


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


#endif