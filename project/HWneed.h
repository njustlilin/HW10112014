#ifndef HWNEED_H
#define HWNEED_H
#include "Calculate.h"

#define MAXSIZE 11
typedef void(*func_pointer)();

void getTime(tv* result);
int runAtTime(func_pointer schedFunc, tv *time);
void runAtTrigger(func_pointer trigFunc(tv * sche));
void executeI();

typedef struct queue_to_run {
    func_pointer func[MAXSIZE];
    tv sche[MAXSIZE];
    int front;
    int tail;
} taskqueue;

void en_uscount(tv * sche);

DigitalOut myled(LED2);
DigitalOut togglepin(p21);
InterruptIn slaveToggle(p22);

tv pintime = {5, 100000};

void pinToggle();
Serial TimeReceive(p13, p14);

#endif