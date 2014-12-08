#include "mbed.h"
#include "Calculate.h"
#include "HWneed.h"
#include <stdio.h>
//LocalFileSystem local("local");
//Serial pc(USBTX, USBRX);
 //address may use//

#define PCLKSEL0    0x400FC1A8 

#define PCLKSEL1    0x400FC1AC

#define ISER0       0xE000E100

#define ICPR0       0xE000E280

bool debug = true;

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
tv timeval;
coeff coefficient;
taskqueue tq;
//******************************************This is initial part of the system****************************//
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
/*    NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_SetPriority(UART2_IRQn, 0);
    NVIC_SetPriority(TIMER1_IRQn, 1);
//    NVIC_SetPriority(UART0_IRQn, 2);
    NVIC_SetPriority(UART3_IRQn, 0);
    NVIC_SetPriority(UART1_IRQn, 0);
*/
    tq.front = 0;
    tq.tail = 0;
}


void WriteOut(int a, int b, int d, double c)
{
    FILE *outfp = fopen("/local/out.txt", "w");  // Open "out.txt" on the local file system for writing
    if ( outfp == NULL)
        error("Could not open OUT.txt\n");
    else
        fprintf(outfp, "%d \r\n%d\r\n%f\r\n%d\r\n", a, b, c, d);        
    fclose(outfp);
}


/*void printReg() {
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
        pc.printf("The T0TC is: %X\n", LPC_TIM0->TC);
        pc.putc(13);
        pc.printf("The T0PC is: %X\n", LPC_TIM0->PC);
        pc.putc(13);
        pc.printf("The T0PR is: %X\n", LPC_TIM0->PR);
        pc.putc(13);
        pc.printf("The T0TC is: %X\n", LPC_TIM0->TC);
        pc.putc(13);
        pc.printf("The T0PC is: %X\n", LPC_TIM0->PC);
        pc.putc(13);
        pc.printf("The T0PR is: %X\n", LPC_TIM0->PR);
        pc.putc(13);
        int p1,p2,t1,t2;
        int time_buffer=1;
        t1 = LPC_TIM0->TC;
        p1 = LPC_TIM0->PC;
        wait(time_buffer);
        p2 = LPC_TIM0->PC;
        t2 = LPC_TIM0->TC;
        pc.printf("The T1 is: %d\n", t1);
        pc.putc(13);
        pc.printf("The T2 is: %d\n", t2);
        pc.putc(13);
        pc.printf("The P1 is: %d\n", p1);
        pc.putc(13);
        pc.printf("The P2 is: %d\n", p2);
        pc.putc(13);
        pc.printf("The clock frequency is: %d\n", (p2-p1+ LPC_TIM0->PR*(t2-t1))/time_buffer);
        pc.putc(13);
    }
}
*/
///////////////////////////////////////////////////////synchronize///////////////////////////////////////////////////
Serial Master(p9, p10); //(tx, rx)
//Serial Master(USBTX, USBRX);
void MasterRecord(int tsec, int tusec)
{
    Master.putc(char((tsec&0xFF000000)>>24));
    Master.putc(char((tsec&0x00FF0000)>>16));
    Master.putc(char((tsec&0x0000FF00)>>8));
    Master.putc(char((tsec&0x000000FF)>>0));
    
    Master.putc(char((tusec&0xFF000000)>>24));
    Master.putc(char((tusec&0x00FF0000)>>16));
    Master.putc(char((tusec&0x0000FF00)>>8));
    Master.putc(char((tusec&0x000000FF)>>0));
}


void ReceiveInfo()
{
if (Master.readable()) {

    char c = Master.getc();    
    tv arrive = {LPC_TIM0->TC, LPC_TIM0->PC};
    //__disable_irq();
    //printf("receive@%d\r\n", arrive.tv_usec/96);
    int t=0;
    tv receive = {0, 0};
    tv send;
    if (c == '7')
    //switch (Master.getc())
    {
        //case '7':
                                 //10 values of Tglobal and Tlocal to calculate drift////
            while (t < 10)
            {
                coefficient.y[t] = arrive;
                for (int i=3; i>=0; i--)
                {
                    while (!Master.readable());
                    receive.tv_sec = receive.tv_sec << 8;
                    receive.tv_sec |= int(Master.getc()); 
                }
                for (int i=3; i>=0; i--)
                {
                    while (!Master.readable());
                    receive.tv_usec = receive.tv_usec << 8;
                    receive.tv_usec |= int(Master.getc()); 
                }
                coefficient.x[t] = receive;
                t++;
                if (t == 10) break;
                while (!Master.readable());
                arrive.tv_usec = LPC_TIM0->PC;
                arrive.tv_sec = LPC_TIM0->TC;
                Master.getc();
                //led1 = ~led1;
            }  
            ////////////////////////////////////HandShake to calculate Offset////////////////////////
            //led2 =1;
            while (!Master.writeable());
            send.tv_usec = LPC_TIM0->PC;
            send.tv_sec = LPC_TIM0->TC;
            Master.putc('7');
            while (!Master.readable());
            receive.tv_usec = LPC_TIM0->PC;
            receive.tv_sec = LPC_TIM0->TC;
            char c2 = Master.getc();
            //led3 =1;
            if (c2 != '3') error("aa");
            tv T1 = send;
            tv T4 = receive;
            tv T23;
            for (int i=3; i>=0; i--)
            {
                while (!Master.readable());
                    T23.tv_sec = T23.tv_sec << 8;
                    T23.tv_sec |= int(Master.getc()); 
            }
            for (int i=3; i>=0; i--)
            {
                while (!Master.readable());
                    T23.tv_usec = T23.tv_usec << 8;
                    T23.tv_usec |= int(Master.getc()); 
            }
            
            /////////////////////////Computing////////////////////////////////
            coefficient.Drift = LinearRegression(& coefficient);
            int temp1 = T1.tv_sec + T4.tv_sec;
            int temp2 = T1.tv_usec + T4.tv_usec;
            coefficient.off_persec = 1000000*(coefficient.Drift-1);
            T23.tv_usec = T23.tv_usec * coefficient.Drift;
            temp1 = (temp1 - T23.tv_sec)/2;
            temp2 = (temp2 - T23.tv_usec)/2;
            coefficient.Offset_usec = temp2+500000*96;
            coefficient.Offset_sec = temp1;
            if (coefficient.Offset_usec>96000000) {coefficient.Offset_usec-=96000000; coefficient.Offset_sec++;}
            led4 = !led4;                                   //used for test!
            //while (!Master.writeable());
            //Master.putc('3');
            //while (!Master.writeable());
            //WriteOut(coefficient.Offset_sec, coefficient.Offset_usec, coefficient.off_persec, coefficient.Drift*100000000);
            //printf("finish@%d\r\n", LPC_TIM0->PC/96);
    }
    //__enable_irq();
    }
}

void getLocalTime(tv *gt)   //return local time from global tv(divided by 96)
{
    int tmp_us = gt->tv_usec;
    unsigned int tmp_s = gt->tv_sec;
    tmp_s += coefficient.Offset_sec;
    tmp_us += coefficient.Offset_usec/96;
    //tmp_us += 96*coefficient.off_persec*(tmp_s - LPC_TIM0->TC);
    while (tmp_us > 1000000) {tmp_s++; tmp_us-=1000000;}
    while (tmp_us < 0) {tmp_s--; tmp_us+=1000000;}
    gt->tv_usec = tmp_us;
    gt->tv_sec = tmp_s;
    //printf("%d, %d\r\n", tmp_s, tmp_us);
    
}
//////////////////////////////////////////////These functions helps////////////////////////////////////////////////////
bool tqempty()
{
    return (tq.tail == tq.front);
}

bool tqfull()
{
    return (tq.front == (tq.tail + 1)%MAXSIZE);
} 

//////////////////////////////////////////////This is the required functions///////////////////////////////////////////

void getTime(tv * result)
{
    tv current = {LPC_TIM0->TC, LPC_TIM0->PC};    
    long long tmp = current.tv_usec;
    tmp += current.tv_sec*96000000;
    long long off = coefficient.Offset_usec;
    off += coefficient.Offset_sec*96000000;
    tmp -= off;
    tmp = tmp/coefficient.Drift;
    result->tv_usec = ((long long)tmp % 96000000)/96;
    result->tv_sec = (long long)tmp/96000000;
}

void executeI()
{
    NVIC_DisableIRQ(TIMER1_IRQn);
    //__disable_irq();
    //led2 = !led2;
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
    }*/
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
    //led4 = !led4;
        NVIC_DisableIRQ(TIMER0_IRQn);
        timeval.tv_sec++;
        NVIC_DisableIRQ(TIMER1_IRQn);
        LPC_TIM1->TCR = 0x2;
        LPC_TIM1->TC = 0;
        LPC_TIM1->PC = 0;
//        LPC_TIM1->IR = 0x1;
        LPC_TIM1->TCR = 0x1;
        
        if (!tqempty()) 
        {
            if ((tq.sche[tq.front].tv_sec == timeval.tv_sec))
        {
    //led3 = !led3;
            LPC_TIM1->MR0 = tq.sche[tq.front].tv_usec;
            LPC_TIM1->MCR = 0x1;
            NVIC_EnableIRQ(TIMER1_IRQn);
            //printf("%d  %d %d\r\n", tq.front, tq.sche[tq.front].tv_sec, tq.sche[tq.front].tv_usec);
        }
           else if (tq.sche[tq.front].tv_sec < LPC_TIM0->TC) tq.front = (tq.front + 1)%MAXSIZE;
        }
        LPC_TIM0->IR = 0x1;
        LPC_TIM0->MR0++;
        coefficient.Offset_usec += coefficient.off_persec*96;
        coefficient.Offset_usec += 43*96;
        if (coefficient.Offset_usec > 96000000) {coefficient.Offset_sec++; coefficient.Offset_usec-=96000000;}
        if (coefficient.Offset_usec < 0) {coefficient.Offset_sec--; coefficient.Offset_usec+=96000000;}
        NVIC_EnableIRQ(TIMER0_IRQn);
}
    
}


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
    getLocalTime(time);
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
    else { ScheQueue(schedFunc, time);}

    return 1;
}    


void runAtTrigger(void(*trigFunc)(void))
{
    slaveToggle.rise(trigFunc);
    slaveToggle.fall(trigFunc);    
}
/////////////////////////////////////////////////////////////////////////

void my1()
{led1=!led1;}
void my2()
{led2=!led2;}
void my3()
{led3=!led3;}
void my4()
{led4=!led4;}

//////////////////////////////////////////////////////Problem 2////////////////////////////////////////////////////////
void pinToggle()
{
    togglepin = !togglepin;
    led1 = !led1;
    pintime.tv_sec += 1;
    tv act = pintime;
    //printf("@%d, %d\r\n", LPC_TIM0->TC, LPC_TIM0->PC/96);
    //getLocalTime(&act);
//    if (act.tv_sec < LPC_TIM0->TC || (act.tv_sec == LPC_TIM0->TC && act.tv_usec < (LPC_TIM0->PC/96 - 10))) printf("error@%d, %d, %d\r\n", act.tv_sec, act.tv_usec, LPC_TIM0->PC/96);
    runAtTime(&pinToggle, &act);
}

void ReceiveTim()
{
    unsigned int usec = 0;
    unsigned int sec = 0;
    for (int i=3; i>=0; i--)
    {
        while (!Master.readable());
        sec = sec << 8;
        sec |= int(Master.getc()); 
    }
    for (int i=3; i>=0; i--)
    {
        while (!Master.readable());
        usec = usec << 8;
        usec |= int(Master.getc()); 
    }
    pintime.tv_sec = sec;
    pintime.tv_usec = usec;
    runAtTime(&pinToggle, &pintime);
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main() {
    coefficient.Drift = 1;
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;
    Master.attach(&ReceiveInfo, Master.RxIrq);
    //TimeReceive.attach(&ReceiveTim, TimeReceive.RxIrq);
    t_init();
    
    wait(2);
    tv act = pintime;
    //printf("%d  %d %d\r\n", act.tv_sec, act.tv_usec, tq.sche[tq.front].tv_usec);
    //getLocalTime(&act);
    runAtTime(&pinToggle, &act);
/*    tv time[4];
    for(int i=0;i<4;i++)
    {
        time[i].tv_sec=i+7;
        time[i].tv_usec=(i+1)*100000;
    }
    runAtTime(my1,&time[0]);
    //time[2].tv_sec = 0;
    runAtTime(my2,&time[1]);
    runAtTime(my3,&time[2]);
    runAtTime(my4,&time[3]);
*/
/*    while(i<10) {
        wait(0.4);

        timeval.tv_usec += 400000;
        if (timeval.tv_usec >= 96000000)
        {
            timeval.tv_usec -= 96000000;
            timeval.tv_sec ++;
        }
        coefficient.x[i] = timeval;
        us = LPC_TIM0->PC;
        s = LPC_TIM0->TC;
        
        time1.tv_usec = us;
        time1.tv_sec = s;
        coefficient.y[i] = time1;
        pc.printf("time should be %d.%d\n", timeval.tv_sec, timeval.tv_usec);
        pc.putc(13);
        
        pc.printf("time test is %d.%d\n", time1.tv_sec, time1.tv_usec);
        pc.putc(13);
        i++;        
    }
        coefficient.Drift = LinearRegression(&coefficient);
        pc.printf("The drift is %f\n", coefficient.Drift);
        pc.putc(13);
*/
}
