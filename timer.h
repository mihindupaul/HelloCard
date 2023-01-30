int init_timers();
void close_timers();
void create_timer(HANDLE* pHandle,long duration,void* lpParam);
int cancel_timer(HANDLE hTimer);
VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);