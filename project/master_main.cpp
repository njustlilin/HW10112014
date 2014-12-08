#include "mbed.h"
#include <stdio.h>

#define MAXSIZE 11

Serial Slave(p9, p10);
Serial SendTime(p13,p14);
Serial pc(USBTX, USBRX);
InterruptIn MasterToggle(p22); 
//Serial Slave(p9,p10);//p9 TX p10 RX
 //address may use//
DigitalOut myled(LED1);
DigitalOut testPin(p25);



#define PCLKSEL0    0x400FC1A8 

#define PCLKSEL1    0x400FC1AC

#define ISER0       0xE000E100

#define ICPR0       0xE000E280

bool debug = true;

typedef struct timeval {

    unsigned int tv_sec;

    unsigned int tv_usec;

} tv;

typedef void(*func_pointer)();
tv pintime;

typedef struct queue_to_run {
    func_pointer func[MAXSIZE];
    tv sche[MAXSIZE];
    int front;
    int tail;
} taskqueue;

void executeI();
void en_uscount(tv * sche);
void test2();
void test1()
{
    LPC_TIM0->IR = 0x1;
    printf("haha\r\n");
    LPC_TIM0->MR0 ++;
    NVIC_SetVector(TIMER0_IRQn, (uint32_t)&test2);
    
}

void test2()
{
    LPC_TIM0->IR = 0x1;
    printf("xixi\r\n");
    LPC_TIM0->MR0 ++;
    NVIC_SetVector(TIMER0_IRQn, (uint32_t)&test1);
    
}


tv timeval = {0, 0};

void t_init(void)
{
    LPC_TIM0->PR = 96000000-1;
    LPC_TIM1->PR = 96-1;
    *(int*)PCLKSEL0 |= 0x154;
    *(int*)PCLKSEL1 |= 0x50000;
    LPC_TIM0->TC=0;
    LPC_TIM0->PC=0;
    LPC_TIM1->TC=0;
    LPC_TIM1->PC=0;
    LPC_TIM0->MR0 = 0x1;
    LPC_TIM1->MR0 = 10000000;
    LPC_TIM0->MCR = 0x1;
    LPC_TIM1->MCR = 0x0;
    NVIC_SetVector(TIMER0_IRQn, (uint32_t)&en_uscount);
//    NVIC_SetVector(TIMER0_IRQn, (uint32_t)&test1);
    NVIC_SetVector (TIMER1_IRQn,  (uint32_t)(executeI));
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_DisableIRQ(TIMER1_IRQn);
    LPC_TIM0->TCR |= 0x1;
    LPC_TIM1->TCR |= 0x1;    
/*    NVIC_SetPriority(TIMER0_IRQn, 0);
    NVIC_SetPriority(UART1_IRQn, 2);
    NVIC_SetPriority(TIMER1_IRQn, 0);
    NVIC_SetPriority(UART0_IRQn, 2);
    NVIC_SetPriority(UART3_IRQn, 2);
*/
}

void printReg() {
    if (debug) {
        pc.printf("SystemCoreClock = %d Hz\n", SystemCoreClock);
        pc.putc(13);
        pc.printf("CCLKCFG = %X \n", LPC_SC->CCLKCFG);
        pc.putc(13);
        pc.printf("PCLKSEL0 = %X \n", *(int*)PCLKSEL0);
        pc.putc(13);
        pc.printf("PCLKSEL1 = %X \n", *(int*)PCLKSEL1);
        pc.putc(13);
        pc.printf("The PCONP is: %X\n", LPC_SC->PCONP);
        pc.putc(13);
        pc.printf("The T0TC is: %X\n", LPC_TIM1->TC);
        pc.putc(13);
        pc.printf("The T0PC is: %X\n", LPC_TIM1->PC);
        pc.putc(13);
        pc.printf("The T0PR is: %X\n", LPC_TIM1->PR);
        pc.putc(13);
        pc.printf("The T0TC is: %X\n", LPC_TIM1->TC);
        pc.putc(13);
        pc.printf("The T0PC is: %X\n", LPC_TIM1->PC);
        pc.putc(13);
        pc.printf("The T0PR is: %X\n", LPC_TIM1->PR);
        pc.putc(13);
        int p1,p2,t1,t2;
        int time_buffer=1;
        t1 = LPC_TIM1->TC;
        p1 = LPC_TIM1->PC;
        wait(time_buffer);
        p2 = LPC_TIM1->PC;
        t2 = LPC_TIM1->TC;
        pc.printf("The T1 is: %d\n", t1);
        pc.putc(13);
        pc.printf("The T2 is: %d\n", t2);
        pc.putc(13);
        pc.printf("The P1 is: %d\n", p1);
        pc.putc(13);
        pc.printf("The P2 is: %d\n", p2);
        pc.putc(13);
        pc.printf("The clock frequency is: %d\n", (p2-p1+ LPC_TIM1->PR*(t2-t1))/time_buffer);
        pc.putc(13);
    }
}

void SlaveRecord(unsigned int tsec, unsigned int tusec)
{
    Slave.putc(char((tsec&0xFF000000)>>24));
    Slave.putc(char((tsec&0x00FF0000)>>16));
    Slave.putc(char((tsec&0x0000FF00)>>8));
    Slave.putc(char((tsec&0x000000FF)>>0));
    
    Slave.putc(char((tusec&0xFF000000)>>24));
    Slave.putc(char((tusec&0x00FF0000)>>16));
    Slave.putc(char((tusec&0x0000FF00)>>8));
    Slave.putc(char((tusec&0x000000FF)>>0));
}

void Offset()
{
    unsigned int Tsum_sec,Tsum_usec;
    tv T2, T3;
    
    if(Slave.readable())
    {
//        __disable_irq();
        T2.tv_sec= LPC_TIM0->TC;
        T2.tv_usec= LPC_TIM0->PC;
        char c =Slave.getc();
        if (c == '7') {
        while (!Slave.writeable());
    
//    printf("stepin3\r\n");
        
        Slave.putc('3');
        T3.tv_sec= LPC_TIM0->TC;
        T3.tv_usec= LPC_TIM0->PC;
    
        Tsum_sec = T2.tv_sec+T3.tv_sec;
        Tsum_usec = T2.tv_usec+T3.tv_usec;
        if(Tsum_usec>=96000000)
        {
            Tsum_sec++;
            Tsum_usec-=96000000;
        }
    
        SlaveRecord(Tsum_sec,Tsum_usec);
      //  pc.printf("End of offset!\r\n");
//        __enable_irq();
        return;
    }
  }
}

void SendMesg()
{
    tv t0;
    while (!Slave.writeable()) 
    {
     ;
    }
    Slave.putc('7');
    t0.tv_sec= LPC_TIM0->TC;
    t0.tv_usec= LPC_TIM0->PC;
//    printf("stepin1\r\n");
    SlaveRecord(t0.tv_sec,t0.tv_usec);
/*    
    wait_ms(20);
    
    while (!Slave.writeable())
    {
     ;//pc.printf("readok!\n\r");;
    }
    t1.tv_sec= LPC_TIM0->TC;
    t1.tv_usec= LPC_TIM0->PC;
    Slave.putc('7');
//    printf("stepin1\r\n");
    
    SlaveRecord(t1.tv_sec,t1.tv_usec);
*/}

///////////////////////FOR DUE 1/////////////////////////////////////////////////
void getTime(tv * Master)
{
   Master->tv_sec= LPC_TIM0->TC;
   Master->tv_usec= LPC_TIM0->PC/96;
}


taskqueue tq;

bool tqempty()
{
    return (tq.tail == tq.front);
}

bool tqfull()
{
    return (tq.front == (tq.tail + 1)%MAXSIZE);
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////Execute of schedule//////////////////////////////////////////////////////////////////
void executeI()
{
    //__disable_irq();
    
    tq.func[tq.front]();
    LPC_TIM1->IR = 0x1;
    LPC_TIM1->MCR = 0x0;    
    /////////////////taskqueue is empty after execute//////////////////
/*    if (tq.tail == (tq.front+1)%MAXSIZE) 
    {
        tq.front = (tq.front+1)%MAXSIZE;
        __enable_irq();
        LPC_TIM1->MCR = 0;
        return;
printf("aaaa\r\n");    }*/
    /////////////////the next event will be executed in the same second//////////////////
    if (tq.sche[tq.front].tv_sec == tq.sche[(tq.front+1)%MAXSIZE].tv_sec)
    {
        if (tq.sche[tq.front].tv_usec >= tq.sche[(tq.front+1)%MAXSIZE].tv_usec - 1) //time between events cannot be smaller than 1us
        {
            tq.front = (tq.front+2)%MAXSIZE;
            //__enable_irq();
           return;
        }
        else
        {
            tq.front = (tq.front+1)%MAXSIZE;
            LPC_TIM1->MCR = 0x1;
            LPC_TIM1->MR0 = tq.sche[tq.front].tv_usec;
            NVIC_EnableIRQ(TIMER1_IRQn);
        }
    }
    //////////////////////////////////the next event will be executed in future second///////////////////////
    else
    {
        NVIC_DisableIRQ(TIMER1_IRQn);
        tq.front = (tq.front+1)%MAXSIZE;
//        LPC_TIM0->MR0 = tq.sche[tq.front].tv_sec;    
//        NVIC_EnableIRQ(TIMER0_IRQn);      
    }    
    //__enable_irq();
    
}

void en_uscount(tv * sche)
{
    /////////////////////firstly, normal set/////////////////////////
    //if (!tqempty())
    {
        NVIC_DisableIRQ(TIMER0_IRQn);
       // timeval.tv_sec++;
        NVIC_DisableIRQ(TIMER1_IRQn);
        LPC_TIM1->TCR = 0x2;
        LPC_TIM1->TC = 0;
        LPC_TIM1->PC = 0;
//        LPC_TIM1->IR = 0x1;
        LPC_TIM1->TCR = 0x1;
        
        if (!tqempty()) 
        {
            if ((tq.sche[tq.front].tv_sec == LPC_TIM0->TC))
        {
            LPC_TIM1->MR0 = tq.sche[tq.front].tv_usec;
            LPC_TIM1->MCR = 0x1;
            NVIC_EnableIRQ(TIMER1_IRQn);
            //printf("%d  %d %d\r\n", tq.front, tq.sche[tq.front].tv_sec, tq.sche[tq.front].tv_usec);
        }
            else if (tq.sche[tq.front].tv_sec < LPC_TIM0->TC) tq.front = (tq.front + 1)%MAXSIZE;
        }
        LPC_TIM0->IR = 0x1;
        LPC_TIM0->MR0++;
        NVIC_EnableIRQ(TIMER0_IRQn);
}
//    printf("aaaa1111\r\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int ScheQueue(void (*schedFunc)(void), tv * time)
int ScheQueue(func_pointer schedFunc, tv * time)
{
    unsigned int np;
    unsigned int p=tq.tail;
    //unsigned int fp;
    //////////////////member==1///////////////////////
    if(tqempty())
    {
        tq.func[tq.tail] = schedFunc;
        tq.sche[tq.tail].tv_sec= time->tv_sec;
        tq.sche[tq.tail].tv_usec= time->tv_usec;
        tq.tail=(tq.tail+1)%MAXSIZE;
        return 1;
        
    }
    if(!tqfull())
    { 
        while(p!=tq.front)
        {
            
            if((tq.sche[(p-1+MAXSIZE)%MAXSIZE].tv_sec > time->tv_sec) || ( (tq.sche[(p-1+MAXSIZE)%MAXSIZE].tv_sec==time->tv_sec) && (tq.sche[(p-1+MAXSIZE)%MAXSIZE].tv_usec > time->tv_usec) )  ) 
            {
                tq.sche[p]=tq.sche[(p-1+MAXSIZE)%MAXSIZE];
                tq.func[p] = tq.func[(p-1+MAXSIZE)%MAXSIZE];                
                p=(p-1+MAXSIZE)%MAXSIZE;
            }
            else break;
       }
            np=p;
            tq.sche[np].tv_sec=time->tv_sec;
            tq.sche[np].tv_usec=time->tv_usec;
            tq.func[np] = schedFunc;
            tq.tail=(tq.tail + 1)%MAXSIZE;
            
            if(tq.sche[tq.front].tv_sec==time->tv_sec && tq.sche[tq.front].tv_usec==time->tv_sec ) return 2;
            
            return 1;
        
    }
    else return 0;
}


int runAtTime(func_pointer schedFunc, tv * time)

{
    if (time->tv_usec > 999998) return 0;
    if ((LPC_TIM0->TC > time->tv_sec) || (LPC_TIM0->TC == time->tv_sec && LPC_TIM0->PC/96 > time->tv_usec-10)) return 0;
    if (tqempty())
    {
        ScheQueue(schedFunc, time);
        //insert function and time
        if (LPC_TIM0->TC == tq.sche[tq.front].tv_sec)
        {
            if (LPC_TIM0->PC < (tq.sche[tq.front].tv_usec-1) * 96)
            {
                LPC_TIM1->MCR = 0x1;
                LPC_TIM1->MR0 = tq.sche[tq.front].tv_usec;
                NVIC_EnableIRQ(TIMER1_IRQn);
            }
            
            else return 0;
        }
    }
    else { ScheQueue(schedFunc, time);             //printf("%d another step1\r\n", tq.sche[tq.tail-1].tv_sec);
    }
    return 1;
///////////////////////////////////////////////////////////////////////////////    
}
//////////////////////////END OF RUNATTME///////////////////////////////////////////////////
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

void my1()
{led1=!led1;}
void my2()
{led2=!led2;}
void my3()
{led3=!led3;}
void my4()
{led4=!led4;}


/////////////////BEGIN OF RUNATTRIIGER///////////////////////////////////////////////////////////////

void runAtTrigger(void(*trigFunc)(void))
{
    MasterToggle.rise(trigFunc);
    MasterToggle.fall(trigFunc);    
}



//////////////////End of runattrigger//////////////////////////////////////////////////////////////////////

/////////////////////HOST/////////////////////////////////////////////////////////////////////////////////////////

int temp[10];
int datalength=0;
bool start = false;
void pinToggle();
void FromHost(int);

void receiveH()
{
    char c;
    if (pc.readable())
    {
        c = pc.getc();
        pc.putc(c);
        if (c == 'S') {
        start = true;
        }
        else if (start)
        {
            if (c>='0' && c<='9')
            {  
                temp[datalength++] = c -'0';
                if (datalength >= 10) 
                {
                    datalength = 0;
                    start = false;
                }
                return;
            }
            else if (c == 'E')
            {
                FromHost(datalength);
                datalength = 0;
                start = false;
                pc.putc('\n');
                return;
            }
            else if (c == ' ' ||  c == '\t' || c == '\n') return;
            else 
            {
                datalength = 0;
                start = false;
            }
        }
    }
}

void FromHost(int length)
{
    unsigned int usec=0;
    for(int i=0;i<datalength;i++)
    {
        usec=usec*10+temp[i];
    }
    getTime(&pintime);
    pintime.tv_usec += usec;
    pintime.tv_sec += pintime.tv_usec/1000000;
    pintime.tv_usec = pintime.tv_usec%1000000;    
    runAtTime(&pinToggle, &pintime);
    SendTime.putc(char((pintime.tv_sec&0xFF000000)>>24));
    SendTime.putc(char((pintime.tv_sec&0x00FF0000)>>16));
    SendTime.putc(char((pintime.tv_sec&0x0000FF00)>>8));
    SendTime.putc(char((pintime.tv_sec&0x000000FF)>>0));
        
    SendTime.putc(char((pintime.tv_usec&0xFF000000)>>24));
    SendTime.putc(char((pintime.tv_usec&0x00FF0000)>>16));
    SendTime.putc(char((pintime.tv_usec&0x0000FF00)>>8));
    SendTime.putc(char((pintime.tv_usec&0x000000FF)>>0));
}

int protocol()
{   
   
    char c;
    int i=0;
//    pc.printf("Please Enter the number as SnnnnE:\r\n");
    c = pc.getc();
    if(c == 'S')
    {   
        pc.putc(c); 
        while(1)
        {
            c=pc.getc();
            pc.putc(c);
            if(c <= '9' && c >= '0')
            {
                temp[datalength]=c-'0';
                //pc.print("%d    \n\r",temp[]);
                datalength++;
                if(datalength>10){pc.printf("Out of range!\r\n");return 0;}
            }
            else if (c == ' ' || c == '\t' || c == '\n') continue;
            else if (c == 'E') {//pc.printf("DATALENFTH= %d\r\n",datalength);
                                return 1;}
            else return 0;
        }
        return 1;
    }
}

void HostInput()
{
    datalength=0;
    int sec=0;
    int usec=0;
    if(protocol()) 
    {
        for(int i=0;i<datalength;i++)
        {
            usec=usec*10+temp[i];
        }
        sec=usec/1000000;
        usec=usec%1000000;
        pintime.tv_sec = sec;
        pintime.tv_usec = usec;
        runAtTime(&pinToggle, &pintime);
        SendTime.putc(char((sec&0xFF000000)>>24));
        SendTime.putc(char((sec&0x00FF0000)>>16));
        SendTime.putc(char((sec&0x0000FF00)>>8));
        SendTime.putc(char((sec&0x000000FF)>>0));
        
        SendTime.putc(char((usec&0xFF000000)>>24));
        SendTime.putc(char((usec&0x00FF0000)>>16));
        SendTime.putc(char((usec&0x0000FF00)>>8));
        SendTime.putc(char((usec&0x000000FF)>>0));
        pc.printf("MSEC= %d        \r\n",usec);
    }
}
/////////////////////////end of HOST//////////////////////////////////////////////////////////////////////////////////
//////////////report toggle//////////////////////////////
DigitalOut led(LED1);
void pinToggle()
{
    testPin=!testPin;
    led = !led;
    pintime.tv_sec += 1;
    runAtTime(&pinToggle, &pintime);

}


void ReportToggle()
{
    signed long Diff;
    tv t;
//    pintime.tv_sec++;
    getTime(&t);
    //pintime.tv_sec++;
    Diff=-((pintime.tv_sec-t.tv_sec)*1000000+(pintime.tv_usec-t.tv_usec));
    if (Diff < -100000) Diff += 1000000;
    pc.printf("%d, %d\r\n",Diff, LPC_TIM0->TC);
}


tv syncTime= {0, 501000};
int syncount = 0;
void SyncRequest()
{
    if (syncount < 9)
    {
        //if (!syncount) pc.printf("%d\r\n", LPC_TIM0->PC/96);
        SendMesg();
        syncount ++;
        syncTime.tv_usec += 20000;
        runAtTime(&SyncRequest,&syncTime);
    }
    else 
    {
        SendMesg();
        syncTime.tv_usec = 501000;
        syncTime.tv_sec += 10;
        runAtTime(&SyncRequest, &syncTime);
        //pc.printf("%d, %d\r\n", syncount, LPC_TIM0->PC/96);
        syncount = 0;
    }
    /*
    syncTime.tv_sec += 10;
    int i=0;
    while(i<5) 
    {
        SendMesg();
        wait_ms (20);
        i++;
    }
    //Offset();
    runAtTime(&SyncRequest,&syncTime);
*/}

//////////////end of report toggle//////////////////////////////

int main() {
    t_init();
    tq.tail=0;
    tq.front=0;
//    printReg();
    Slave.attach(&Offset,Slave.RxIrq);
    runAtTime(&SyncRequest,&syncTime);
    //__disable_irq();
    //while (!Slave.readable());
    ///////////////////////PB 2/////////////////(SnnnnE)//////////////////////////////
    pc.attach(&receiveH);
    runAtTrigger(&ReportToggle);
    
    wait(2);
    ////////////////////end of PB 2////////////////////////////////////////////////////
    pintime.tv_sec = 5;
    pintime.tv_usec = 100000;
    runAtTime(&pinToggle, &pintime);
/*    tv time[4];
    for(int i=0;i<4;i++)
    {
        time[i].tv_sec=i*2+4;
        time[i].tv_usec=(i+1)*100000;
    }
    //runAtTime(&pinToggle, &pintime);
    //time[2].tv_sec = 0;
    runAtTime(my2,&time[1]);
    runAtTime(my3,&time[2]);
    runAtTime(my4,&time[3]);
    for(int i=0;i<5;i++)
    {
        printf("%d   %d\r\n", tq.sche[i].tv_sec,tq.sche[i].tv_usec);
    }
*/    wait(2);    
    wait(10);
}