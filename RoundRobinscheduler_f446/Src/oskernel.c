#include "oskernel.h"
#define num_of_threads 	3

#define stackSize 		100 //100 32bit values==> 100*4 = 400bytes
// creating a task control block

#define BusFrequency 16000000 //16MHz internal clock

#define CTRL_ENABLE 	(1U<<0)
#define CTRL_TICKINT 	(1U<<1)
#define CTRL_CLCKSRC 	(1U<<2)
#define CTRL_COUNTFLAG  (1U<<16)
#define systick_rst 	 0
uint32_t ms_Prescaler;
struct tcb{
	int32_t *stackpt;
	struct tcb *nextpt;
};

typedef struct tcb tcbType;
void osSchedulerLaunch(void);

tcbType tcbs[num_of_threads];
tcbType *currentPt;
/* Each thread will have a stackSize of 100 i.e. 400bytes*/
int32_t tcb_stack[num_of_threads][stackSize];

//Create osKernelStackInit(), i=number of threads;

void osKernelStackInit(int i)
{
	tcbs[i].stackpt=&tcb_stack[i][stackSize-16]; //stack pointer

	tcb_stack[i][stackSize - 1]=(1U<<24);//T-bit of PSR register
	//This variable will correspond to the position of the PSR register in out stack
}

// Create a KernelAddThreads()

uint8_t osKernelAddThreads(void(*task0)(void),void(*task1)(void),void(*task2)(void) )
{
	//disable global interrupts
	__disable_irq();
	tcbs[0].nextpt=&tcbs[1];
	tcbs[1].nextpt=&tcbs[2];
	tcbs[2].nextpt=&tcbs[0];
	//Initial stack for thread 0
	osKernelStackInit(0);
	//Initialize PC
	tcb_stack[0][stackSize- 2]=(int32_t)(task0); //address of task0 loaded in pc

	//Initial stack for thread 1
		osKernelStackInit(1);
		//Initialize PC
		tcb_stack[1][stackSize- 2]=(int32_t)(task1);

	//Initial stack for thread 2
		osKernelStackInit(2);
		//Initialize PC
		tcb_stack[2][stackSize- 2]=(int32_t)(task2 );

	//start from thread 0
		currentPt= &tcbs[0];
	//enable global interrupt
		__enable_irq();

	return 1;
}

//kernel Initializing function
void oskernelInit(void)
{
	ms_Prescaler= BusFrequency/1000;
}
//Implement the kernel Launch function
void oskernelLaunch(uint32_t quanta)
{
	//Reset systick
	SysTick->CTRL =systick_rst;
	//clear systick current value register
	SysTick->VAL=0;
	//load quanta
	SysTick->LOAD=(quanta * ms_Prescaler) -1;
	//set systick to low priority
	NVIC_SetPriority(SysTick_IRQn,15);
	//enable systick, select internal clock
	SysTick->CTRL =CTRL_CLCKSRC | CTRL_ENABLE;
	// enable systick interrupt
	SysTick->CTRL |= CTRL_TICKINT;
	//launch scheduler
	osSchedulerLaunch();

}

__attribute__((naked)) void SysTick_Handler(void)
{
	//suspend current thread
	//disable global interrupt
	__asm("CPSID I");
	//save r4,r5,r6,r7,r8,r9r10,r11
	__asm("PUSH {R4-R11}");
	//load address of currentPt into r0.
	__asm("LDR R0,=currentPt");
	//load r1 from address equals r0 ,i.e. r1=currentPt.
	__asm("LDR R1,[R0]");
	//Store cortex-M SP at address equals r1, i.e. save SP into tcb
	__asm("STR SP,[R1]");
	//choose next thread
	//Load r1 from a location 4-byte above address r1, i.e. r1=currPt->next
	__asm("LDR R1, [R1,#4]");
	//store r1 at address equals r0	 i.e. currentPt= r1
	__asm("STR R1,[R0]");
	//load cortex-M SP from address equals r1, i.e. SP=currentPt->stackPt
	__asm("LDR SP, [R1]");

	//Restore r4,r5,r6,r7,r8,r9,r10,r11
	__asm("POP {R4-R11}");
	//Enable global interrupts
	__asm("CPSIE I");
	//Return from exception and restore r0,r1,r2,r3,r12,lr,pc,xPSR
	__asm("BX LR");
	/*When exception ocurs these register (r0,r1,r2,r3,r12,lr,pc,xPSR)
	 * are automatically	saved onto the stack. */
}

//Implementing osScheduler launch function
void osSchedulerLaunch(void)
{
	//load address of currentPt into the R0
	__asm("LDR R0,=currentPt ");
	//load r2 from address equals r0, r2=currentPt
	__asm("LDR R2,[r0]");
	//load cortex-M SP from address equals r2, i.e. SP=CurrentPt->stackPt
	__asm("LDR SP,[R2]");
	//Restore r4,r5,r6,r7,r8,r9,r10,r11
	__asm("POP {R4-R11}");

	//Restore r12
	__asm("POP {R12}");

	//Restore  r0,r1,r2,r3
	__asm("POP {R0-R3}");
	//skip LR
	__asm("ADD SP,SP,#4");
	//create a new start location by popping LR
	__asm("POP {LR}");

	//skip PSR by adding 4 to SP
	__asm("ADD SP,SP,#4");

	//Enable global interrupts
	__asm("CPSIE I");

	//Return from exception
	__asm("BX LR");
}
