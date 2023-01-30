#include <stdafx.h>
#include "timer.h"
#include "phoneline.h"

HANDLE hTimerQueue = NULL;
extern CLineManager g_LineManager;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	//	this is get called by some other thread
//	DWORD h = ::GetCurrentThreadId();

	if (lpParam != NULL)
    {
		g_LineManager.TimerTrigger(lpParam);
    }
}

int init_timers()
{
    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
//	DWORD h = ::GetCurrentThreadId();
    if (!hTimerQueue)
    {
        printf("CreateTimerQueue failed (%d)\n", GetLastError());
        return 2;
    }

    return 0;
}

void close_timers()
{
    // Delete all timers in the timer queue.
    if (!DeleteTimerQueue(hTimerQueue))
        printf("DeleteTimerQueue failed (%d)\n", GetLastError());
}

void create_timer(HANDLE* pHandle,long duration,void* lpParam)
{
	if(!::CreateTimerQueueTimer(pHandle,hTimerQueue,TimerRoutine,lpParam,duration,0,WT_EXECUTEONLYONCE)) //WT_EXECUTEINTIMERTHREAD
		pHandle = NULL;
}

int cancel_timer(HANDLE hTimer)
{
	if(DeleteTimerQueueTimer( hTimerQueue, hTimer, INVALID_HANDLE_VALUE))
	{
		//	success completion
		return 0;
	}
	return -1;
}