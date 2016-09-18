#ifndef __H_RWLOCK__
#define __H_RWLOCK__

#include <Windows.h>

class RWLock {
public:
	RWLock();
	~RWLock();

	void acquireRead();
	void releaseRead();

	void acquireWrite();
	void acquireWrite(bool upgrade);
	bool tryAcquireWrite(bool upgrade);
	void releaseWrite();
private:
	void _waitForReaders(unsigned int n_readers);
	CRITICAL_SECTION readers_crit_section;
	CRITICAL_SECTION global_crit_section;

	DWORD writer_thread;

	unsigned int n_readers = 0;
	bool readers_blocked = false;
};
#endif
