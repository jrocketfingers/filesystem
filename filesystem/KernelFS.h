#ifndef __H_KERNELFS__
#define __H_KERNELFS__

#include <string>
#include <map>
#include "FS.h"
#include "FSPartition.h"
#include "FileHandle.h"
#include "KernelFSInterface.h"

#include "RWLock.h"

using namespace std;

class KernelFS : public KernelFSInterface {
public:
	KernelFS();
	~KernelFS();

	char mount(Partition* partition);
	bool unmount(char part);
	char format(char part);
	char readRootDir(char part, EntryNum n, Directory &d);
	char doesExist(char* fname, FileHandle** file_handle);

	File* open(char* fname, char mode);
	void close(string fpath);
	char deleteFile(char* fname);
private:
	FileHandle* _createPhysicalFile(char * fpath);
	File* _createFileInstance(FileHandle* file_handle, char mode);

	FSPartition **partitions;
	char lastFreeLetter = 'A';

	map<string, FileHandle*> file_table;

	RWLock fsLock;
};
#endif