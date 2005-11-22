#include "MultiTimer.h"

#ifdef USE_TIMERS
	CTimerFactory timerFactory;
	CHiFreqTimer::TimingDetails CHiFreqTimer::timerDetails;

#ifdef _CLOCKTICKS
int CHiFreqTimer::TimingDetails::testfun()
{
	int k = 0;
	for(int i = 1; i < 10000; i++)
		for(int j = 1; j < 1000; j++)
			k += i + j;
	return k;
}
#endif

void CHiFreqTimer::TimingDetails::Setup()
{
	LARGE_INTEGER freq;
	if (QueryPerformanceFrequency(&freq) == 0)
	{
		MessageBox(0, "Timer not supported", "Error", MB_OK);
		exit(0);
	}
	perFreq = ((double)freq.QuadPart) / 1000;
	nextTimer = curBase = curManualBase = 0;

	int i, loops = 1000;
	void* Fred = GetCurrentThread();
	int CurPri = GetThreadPriority(Fred);
	SetThreadPriority(Fred, THREAD_PRIORITY_HIGHEST);
	Sleep(100);

	LARGE_INTEGER pTest;
	{TIME_SCOPE("ss overhead")
	for (i = 0; i < loops; i++)
	{
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
		QueryPerformanceCounter(&pTest);
	}
	}
	startstopOverhead = GET_TIMER("ss overhead").getTotalTime() / (loops * 10);
	CLEAR_TIMERS

	for (i = 0; i < loops; i++)
	{TIME_SCOPE("nested")
		{TIME_SCOPE("sub")
	}}
	nestedOverhead = GET_TIMER("nested").getAverageTime();
	CLEAR_TIMERS
	for (i = 0; i < loops; i++)
	{TIME_SCOPE("nested")
		{
			TIMER_START("sub")
			TIMER_STOP("sub")
		}
	}
	manualNestedOverhead = GET_TIMER("nested").getAverageTime();// * 1.4;
	CLEAR_TIMERS

#ifdef _CLOCKTICKS
	// Look at clock speed
	__int64 res;
	{TIME_SCOPE("ticktock")
	_asm 
	{
		rdtsc
		push eax
		push edx
	}
	testfun();
	_asm
	{
		rdtsc
		pop ebx
		sub edx, ebx
		pop ebx
		sub eax, ebx
		mov dword ptr [res], eax
		mov dword ptr [res+4], edx
	}
	}
	clockSpeed = res / GET_TIMER("ticktock").getTotalTime();
	CLEAR_TIMERS
#endif

	SetThreadPriority(Fred, CurPri);
}
#endif	// USE_TIMERS


