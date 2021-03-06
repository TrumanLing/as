/**
 * AS - the open source Automotive Software on https://github.com/parai
 *
 * Copyright (C) 2018  AS <parai@foxmail.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
/* ============================ [ INCLUDES  ] ====================================================== */
#include "kernel_internal.h"
#include "asdebug.h"
/* ============================ [ MACROS    ] ====================================================== */
#define AS_LOG_OS 0
/* ============================ [ TYPES     ] ====================================================== */
/* ============================ [ DECLARES  ] ====================================================== */
extern void Os_PortResume(void);
/* ============================ [ DATAS     ] ====================================================== */
uint32 ISR2Counter;
/* ============================ [ LOCALS    ] ====================================================== */
/* ============================ [ FUNCTIONS ] ====================================================== */
void Os_PortActivate(void)
{
	/* get internal resource or NON schedule */
	RunningVar->priority = RunningVar->pConst->runPriority;

	ASLOG(OS, "%s(%d) is running\n", RunningVar->pConst->name,
			RunningVar->pConst->initPriority);

	OSPreTaskHook();

	CallLevel = TCL_TASK;
	Irq_Enable();

	RunningVar->pConst->entry();

	/* Should not return here */
	TerminateTask();
}

void Os_PortInit(void)
{
	ISR2Counter = 0;
}

void Os_PortInitContext(TaskVarType* pTaskVar)
{
	pTaskVar->context.sp = pTaskVar->pConst->pStack + pTaskVar->pConst->stackSize-4;
	pTaskVar->context.pc = Os_PortActivate;
}

void EnterISR(void)
{
	/* do nothing */
}

void LeaveISR(void)
{
	/* do nothing */
}
#ifdef USE_PTHREAD_SIGNAL
void Os_PortCallSignal(int sig, void (*handler)(int), void* sp, void (*pc)(void))
{
	asAssert(NULL != handler);

	handler(sig);

	/* restore its previous stack */
	RunningVar->context.sp = sp;
	RunningVar->context.pc = pc;
}

void Os_PortExitSignalCall(void)
{
	Sched_GetReady();
	Os_PortStartDispatch();
}

int Os_PortInstallSignal(TaskVarType* pTaskVar, int sig, void* handler)
{
	void* sp;
	uint32_t* stk;

	sp = pTaskVar->context.sp;

	if((sp - pTaskVar->pConst->pStack) < (pTaskVar->pConst->stackSize*3/4))
	{
		/* stack 75% usage, ignore this signal call */
		ASLOG(OS,"install signal %d failed\n", sig);
		return -1;
	}

	stk = sp;

	pTaskVar->context.sp = stk;
	pTaskVar->context.pc = Os_PortResume;

	return 0;
}
#endif
