#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXPROC 20
#define MAXSEMD MAXPROC
#define ASHDSIZE 8
#define MAX_PCB_PRIORITY		10
#define MIN_PCB_PRIORITY		0
#define DEFAULT_PCB_PRIORITY		5



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

void freePcb(pcb_t *p){
    if (p != NULL){             //<----- inserimento in coda o in testa??
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

//restituisce l'ultimo fratello di p
pcb_t *lastSib(pcb_t *p){
    if(p->p_sib != NULL){
        return lastSib(p->p_sib);
    }
    return p;
}

//inserisce p tra i figli di parent
void insertChild(pcb_t *parent, pcb_t *p){
    p -> p_parent = parent;
    if(parent -> p_first_child == NULL){
        parent -> p_first_child = p;
    }else{
        lastSib(parent -> p_first_child) -> p_sib = p;
    }
}

//rimuove il primo figlio di p
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

//rimuove (p) un PCB dalla lista dei figli di suo padre
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

void removeSemd(int *key){
    int n = ((int)(key)/(int)sizeof(int))%ASHDSIZE;
    setSemd(&semdFree_h,removeSemd_scan(&semdhash[n],key)); //mette tra i liberi quello che rimuove
}


//rimuove il PCB in testa alla coda dei bloccati del semaforo che ha key
pcb_t* removeBlocked(int *key){
    pcb_t *q = headBlocked(key); //coda dei processi del semaforo con chiave key
    if(q != NULL){
        pcb_t *out = removeProcQ(&q); //null se non rimuove niente(impossibile) altrimenti puntatore al proc rimosso dalla coda dei bloccati
        if(q == NULL){
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

void forallBlocked(int *key, void fun(pcb_t *pcb, void *), void *arg) {
    pcb_t *temp = headBlocked(key);
    if(temp != NULL){
        forallProcQ(temp, fun, arg);
    }
}

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



        int my_counter_process;
        int onesem;
        pcb_t	*procp[MAXPROC], *p, *q, *maxproc, *minproc, *proc;
        int  sem[MAXSEMD];
        pcb_t* head;


/* This function places the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */

 void addokbuf(char *strp){
    printf(strp);
}

/* This function places the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void adderrbuf(char *strp) {

    printf(strp);
}

void increment_counter(struct pcb_t *pcb, void* pt)
{
    int *counter=pt;
    (*counter)++;
}

/******************************************************************************
 * Main Test File
 ******************************************************************************/

int main() {

    int i;

    head = NULL;

    addokbuf("Strange Operating System test\n");

    initPcbs();
    addokbuf("Control Blocks initialized\n");
    if(MAXPROC == 20)
        addokbuf("No more than 20 proc\n");

    /* controlla la allocPcb */
    for (i = 0; i < MAXPROC; i++) {
        if ((procp[i] = allocPcb()) == NULL)
            adderrbuf("allocPcb(): unexpected NULL\n");
    }

    /* it should not allocate more than MAXPROC elements */
    if (allocPcb() != NULL) {
        adderrbuf("ERROR: allocPcb(): allocated more than MAXPROC entries\n");
    }
    addokbuf("AllocPcb test OK\n");
    addokbuf("List initialized correctly\n");

    /* Return the last 10 entries back to free list */
    for (i = 10; i < MAXPROC; i++)
        freePcb(procp[i]);

    addokbuf("10 entries were added to the free PCB list\n");

    addokbuf("We are going to test the insertProcQ function...\n");

    /* insert 10 elements using different priorities. one max, one min prio
    all the others default prio */
    for (i = 0; i < 10; i++) {
        q = allocPcb();
        if ( q == NULL)
            adderrbuf("ERROR: allocPcb(): unexpected NULL while insert\n");
        switch (i) {
            case 5:
                q->priority=DEFAULT_PCB_PRIORITY;
                proc=q;
                break;
            case 1:
                q->priority=MAX_PCB_PRIORITY;
                maxproc = q;
                break;
            case 2:
                q->priority=MIN_PCB_PRIORITY;
                minproc=q;
                break;
            default:
                q->priority=DEFAULT_PCB_PRIORITY;
                break;
        }
        /* insert the process in the queue */
        insertProcQ(&head, q);
    }

    addokbuf("Test insertProcQ: OK. Inserted 10 elements \n");

    addokbuf("Checking if the first element has max priority\n");
    /* Check outProcQ and headProcQ */
    if (headProcQ(head) != maxproc)
        adderrbuf("ERROR: headProcQ(qa) failed\n");
    addokbuf("Done\n");

    /* remove an element */
    addokbuf("Removing an element. Testing outProcQ\n");
    q=outProcQ(&head, proc);
    if ((q == NULL) || (q != proc))
        adderrbuf("ERROR: outProcQ(&qa, proc) failed to remove the entry\n");
    freePcb(q); /* return the element to the free list */
    addokbuf("Done\n");

    /* Removing the first element from ProcQ */
    addokbuf("Removing the first element. Testing removeProcQ\n");
    q = removeProcQ(&head);
    if (q == NULL || q != maxproc)
        adderrbuf("ERROR: removeProcQ(&qa, midproc) failed to remove the elements in the right order\n");
    freePcb(q);
    addokbuf("Done\n");

    /* we inserted 10 elements and removed 2. Now let us remove the
    remaining elements and check if the last one has the min prio */
    addokbuf("Testing removeProcQ ...   \n");
    for (i = 0; i < 7; i++) {
        if ((q = removeProcQ(&head)) == NULL)
            adderrbuf("removeProcQ(&qa): unexpected NULL\n");
        freePcb(q);
    }

    /* keeping track of the last element */
    q=headProcQ(head);
    addokbuf("Applying a function to all the elements of the list. Testing forallProcQ\n");
    /* there should be just one element inside the list. i'm going to remove by using the forallProcQ instruction */
    my_counter_process = 0;
    forallProcQ(head, increment_counter , &my_counter_process);
    if(my_counter_process != 1)
        adderrbuf("ERROR: forallProcQ(): failed on last entry\n");
    my_counter_process = 0;
    removeProcQ(&head);
    if (q != minproc)
        adderrbuf("ERROR: headProcQ(): failed on last entry\n");
    freePcb(q);
    addokbuf("Done\n");

    addokbuf("Let's see if now the queue is empty\n");
    addokbuf("Test insertProcQ(), removeProcQ(), headProcQ(), forallProcQ(), outProcQ(): OK\n");
    addokbuf("Test process queues module: OK\n");

    addokbuf("Testing process trees...\n");

    /*check that it has no child*/
    q = removeChild(procp[2]);
    if (q!=NULL )
        adderrbuf("ERROR: emptyChild: unexpected FALSE\n");
    /*if the test is passed, then the node has no child at all*/

    /* make procp[1],procp[2],procp[3], procp[7] children of procp[0] */
    addokbuf("Inserting...\n");
    insertChild(procp[0], procp[1]);
    insertChild(procp[0], procp[2]);
    insertChild(procp[0], procp[3]);
    insertChild(procp[0], procp[7]);
    addokbuf("Inserted 4 children of pcb0\n");

    /* make procp[8],procp[9] children of procp[7] */
    insertChild(procp[7], procp[8]);
    insertChild(procp[7], procp[9]);
    addokbuf("Inserted 2 children of pcb7\n");

    /*checking if node 7 has a child. if it doesn't have, ERRRO*/
    q=removeChild(procp[7]);
    addokbuf("Removed the first child of 7\n");
    if(q==NULL)
        adderrbuf("ERROR: emptyChild: node 7 has no child!\n");
    if(q!=procp[8])
        adderrbuf("ERROR: removeChild(procp[0] failed\n");
    /*otherwise, we will re-insert*/
    insertChild(procp[7], q);

    addokbuf("Re-inserted son in 7, now removing in 0\n");

    /*check the same with node 0*/
    q=removeChild(procp[0]);
    addokbuf("Removed the first child of 0\n");
    if(q==NULL)
        adderrbuf("ERROR: emptyChild: node 0 has no child!\n");
    if(q!=procp[1])
        adderrbuf("ERROR:  removeChild(procp[0] failed\n");
    /*otherwise, we will re-insert*/
    insertChild(procp[0], q);

    addokbuf("Check outchild function\n");

    /* Check outChild */
    q = outChild(procp[1]);
    addokbuf("Outchild executed on 1\n");
    if (q == NULL || q != procp[1])
        adderrbuf("ERROR: outChild(procp[1]) failed\n");
    q = outChild(procp[8]);
    addokbuf("Outchild executed on 8\n");
    if (q == NULL || q != procp[8])
        adderrbuf("ERROR: outChild(procp[8]) failed\n");

    /* Check removeChild */
    q = removeChild(procp[0]);
    addokbuf("Removechild executed on 0\n");
    if (q==procp[1])
        adderrbuf("q==pp1");
    if (q == NULL || q != procp[2])
        adderrbuf("ERROR: removeChild(procp[0])  1 failed\n");

    q = removeChild(procp[7]);
    addokbuf("Removechild executed on 7\n");
    if (q == NULL || q != procp[9])
        adderrbuf("ERROR: removeChild(procp[7])  2 failed\n");

    q = removeChild(procp[0]);
    addokbuf("Removechild executed on 0\n");
    if (q == NULL || q != procp[3])
        adderrbuf("ERROR: removeChild(procp[0]) 2 failed\n");

    q = removeChild(procp[0]);
    addokbuf("Removechild executed on 0\n");
    if (q == NULL || q != procp[7])
        adderrbuf("ERROR: removeChild(procp[0]) 3 failed\n");

    q=removeChild(procp[0]);
    addokbuf("Removechild executed on 0\n");
    if ( q!=NULL )
        adderrbuf("ERROR: removeChild(procp[0]): it had already a child! it shouldn't! ");

    addokbuf("Test: insertChild(), removeChild() OK\n");
    addokbuf("Testing process tree module OK\n");


    freePcb(procp[0]);
    freePcb(procp[1]);
    freePcb(procp[2]);
    freePcb(procp[3]);
    freePcb(procp[4]);
    freePcb(procp[5]);
    freePcb(procp[6]);
    freePcb(procp[7]);
    freePcb(procp[8]);
    freePcb(procp[9]);


    addokbuf("Now starting with test of semaphores\n");

    /* check ASL */
    initASL();
    addokbuf("Initializing active semaphore list\n");

    /* check removeBlocked and insertBlocked */
    addokbuf("Test insertBlocked(): test #1 started\n");
    for (i = 10; i < MAXSEMD; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("ERROR: insertBlocked() test#1: unexpected TRUE\n");
    }

    addokbuf("Test insertBlocked(): test #2 started\n");
    for (i = 0; i < 8; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[i], procp[i]))
            adderrbuf("ERROR:insertBlocked() test #2: unexpected TRUE\n");
    }

    for (i = 8; i < 10; i++) {
        procp[i] = allocPcb();
        if (insertBlocked(&sem[1], procp[i]))
            adderrbuf("ERROR:insertBlocked() test #3: unexpected TRUE\n");
    }


    addokbuf("Counting the number of elements inside semaphores\n");
    /*****************************************************************************************/
    /*counting the processes inside a semaphore*/
    /*first i reset to 0*/
    my_counter_process = 0;
    /*then i count*/
    forallBlocked(&sem[1], increment_counter, &my_counter_process);
    if(my_counter_process != 3)
        adderrbuf("ERROR forallBlocked, increment_counter. expected 3\n");
    else
        addokbuf("I have found 3 elements in sem 1; correct\n");

    /*counting the processes inside a semaphore*/
    /*first i reset to 0*/
    my_counter_process = 0;
    /*then i count*/
    forallBlocked(&sem[2], increment_counter, &my_counter_process);
    if(my_counter_process != 1)
        adderrbuf("ERROR forallBlocked, increment_counter. expected 1\n");
    else
        addokbuf("I have found 1 elements in sem 2; correct\n");

    /*adjust*/
    p = removeBlocked(&sem[1]);
    if (insertBlocked(&sem[8],p))
        adderrbuf("ERROR: removeBlocked(): fails to return to free list\n");
    p = removeBlocked(&sem[1]);
    if (insertBlocked(&sem[9],p))
        adderrbuf("ERROR: removeBlocked(): fails to return to free list\n");
    /*now all the semaphores are used*/

    /*****************************************************************************************/

    addokbuf("Check if the semaphores are returned to the free list\n");
    /* check if semaphore descriptors are returned to the free list */
    p = removeBlocked(&sem[11]);
    if (insertBlocked(&sem[11],p))
        adderrbuf("ERROR: removeBlocked(): fails to return to free list\n");

    if (insertBlocked(&onesem, procp[9]) == false)
        adderrbuf("ERROR: insertBlocked(): inserted more than MAXSEMD\n");

    addokbuf("Test removeBlocked(): test started\n");
    for (i = 10; i< MAXPROC; i++) {
        q = removeBlocked(&sem[i]);
        if (q == NULL)
            adderrbuf("ERROR: removeBlocked(): wouldn't remove\n");
        if (q != procp[i]) /* different from what expected!*/
            adderrbuf("ERROR: removeBlocked(): removed wrong element\n");
    }

    addokbuf("Test blocked done\n");

    if (removeBlocked(&sem[11]) != NULL)
        adderrbuf("ERROR: removeBlocked(): removed nonexistent blocked proc\n");

    addokbuf("Test insertBlocked() and removeBlocked() ok\n");


#if 0
    /* Creating a 2-layer tree */
	insertChild(procp[0], procp[1]);
	insertChild(procp[0], procp[2]);
	insertChild(procp[2], procp[3]);
	insertChild(procp[3], procp[4]);

	/* Testing outChildBlocked */
	outChildBlocked(procp[2]);
	addokbuf("I called the function outChildBlocked on procp[2]\n");

	/*i'm expecting that the child 2 has no children*/
	if(removeBlocked(&sem[2])!= NULL)
		adderrbuf("ERROR: removeBlocked(): nonNULL for a nonexistent queue (2)  ");
	if(removeBlocked(&sem[3])!= NULL)
		adderrbuf("ERROR: removeBlocked(): nonNULL for a nonexistent queue (3)  ");
	if(removeBlocked(&sem[4])!= NULL)
		adderrbuf("ERROR: removeBlocked(): nonNULL for a nonexistent queue (4)  ");

	if(removeBlocked(&sem[0])== NULL)
		adderrbuf("ERROR: removeBlocked(): nonNULL for a nonexistent queue (0)  ");
	if(removeBlocked(&sem[1])== NULL)
		adderrbuf("ERROR: removeBlocked(): nonNULL for a nonexistent queue (1)  ");
#endif

    addokbuf("ASL module OK\n");
    addokbuf("1f u c4n r34d th1s 1t m34ns th4t y430s m19ht w0rk\n");
    addokbuf("\n");

    return 0;
}
