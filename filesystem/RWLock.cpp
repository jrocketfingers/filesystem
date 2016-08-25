#include <cassert>
#include "RWLock.h"

RWLock::RWLock() {
	InitializeCriticalSection(&readers_crit_section);
	InitializeCriticalSection(&global_crit_section);
}

RWLock::~RWLock() {

}

void RWLock::acquireRead() {
	while (1) {
		EnterCriticalSection(&readers_crit_section);
		if (!readers_blocked) {
			n_readers++;
			LeaveCriticalSection(&readers_crit_section);
			return;
		}
		LeaveCriticalSection(&readers_crit_section);
		Sleep(0);
	}
	assert(0);
}

void RWLock::releaseRead() {
	EnterCriticalSection(&readers_crit_section);
	assert(n_readers != 0);
	n_readers--;
	LeaveCriticalSection(&readers_crit_section);
}

void RWLock::acquireWrite() {
	EnterCriticalSection(&global_crit_section);
	_waitForReaders(0);

	writer_thread = GetCurrentThreadId();
}

bool RWLock::tryAcquireWrite(bool upgrade)
{
	if (!TryEnterCriticalSection(&global_crit_section))
		return false;

	_waitForReaders(upgrade ? 1 : 0);

	writer_thread = GetCurrentThreadId();

	return true;
}

void RWLock::_waitForReaders(unsigned int awaited_n_readers) {
	EnterCriticalSection(&readers_crit_section);
	readers_blocked = true;
	LeaveCriticalSection(&readers_crit_section);

	while (1) {
		EnterCriticalSection(&readers_crit_section);
		if (n_readers == awaited_n_readers) {
			LeaveCriticalSection(&readers_crit_section);
			break;
		}
		LeaveCriticalSection(&readers_crit_section);
		Sleep(0);
	}

	assert(n_readers == awaited_n_readers);
}

void RWLock::releaseWrite() {
	EnterCriticalSection(&readers_crit_section);
	readers_blocked = false;
	LeaveCriticalSection(&readers_crit_section);

	LeaveCriticalSection(&global_crit_section);
}
