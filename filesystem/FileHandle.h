#pragma once

#include <string>
#include "part.h"
#include "FS.h"
#include "RWLock.h"
#include "KernelFSInterface.h"

using namespace std;

/* forward declarations */
class FSPartition;
class FSIndex;
/* end of forward declarations */

class FileHandle
{
public:
	FileHandle(KernelFSInterface *fs);
	FileHandle(KernelFSInterface *fs, const char *fpath);
	~FileHandle();

	void SetPartition(FSPartition* partition);
	FSPartition* GetPartition();

	void SetIndex(FSIndex* index);
	FSIndex* GetIndex();

	void SetDirectoryCluster(ClusterNo cluster);
	ClusterNo GetDirectoryCluster();

	void SetDirectoryEntry(EntryNum entry);
	EntryNum GetDirectoryEntry();

	void SetFilename(char* filename);
	char* GetFilename();

	void SetExtension(char* extension);
	char* GetExtension();

	void SetFilesize(unsigned long filesize); /* just update the value within the instance*/
	void UpdateFilesize(unsigned long filesize); /* update the directory entry of the file */
	unsigned long GetFilesize();

	void incReaders();
	void decReaders();

	void incWriters();
	void decWriters();

	RWLock* GetLock();
private:
	FSPartition* partition = nullptr;
	ClusterNo index_cluster = 0;
	FSIndex* index = nullptr;
	ClusterNo directory_cluster = 0;
	EntryNum directory_entry;

	KernelFSInterface* fs = nullptr;

	unsigned long n_readers = 0;
	unsigned long n_writers = 0;

	char filename[8];
	char ext[3];

	string fpath;

	unsigned long filesize;

	RWLock lock;
	RWLock access_count_lock;
};

