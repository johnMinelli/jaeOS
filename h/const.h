#ifndef CONST_H
#define CONST_H


#define MAXSEMD MAXPROC
#define MAXPROC 20
#define ASHDSIZE 8
#define MAXPRIO 10

//sysCall values
#define CREATEPROCESS       1
#define TERMINATEPROCESS    2
#define SEMP                3
#define SEMV                4
#define SPECHDL             5
#define GETTIME             6
#define WAITCLOCK           7
#define IODEVOP             8
#define GETPIDS             9
#define WAITCHLD            10

//sysCall ret value
#define SCSUCC              0
#define SCFAIL              -1

//type of trap
#define SYSBK               0
#define TLB                 1
#define PGMTRAP             2

//time constants
#define QUANTUM			3000
#define INTERVALTIME	100000
#define INTERVALAGE     10000

//for aging
#define MASK ((MAXPRIO+MAXPRIO)|MAXPRIO)>>1;


#endif //CONST_H