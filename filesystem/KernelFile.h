#pragma once

#include "FSPartition.h"
#include "FSIndex.h"
#include "File.h"
#include "part.h"
#include "OperationIterator.h"

class FSIndex;
class KernelFile {
public:
	KernelFile(FileHandle* file_handle);
	virtual ~KernelFile();
	virtual char write(BytesCnt, const char* buffer);
	virtual BytesCnt read(BytesCnt, char* buffer);
	virtual char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
protected:
	friend class OperationIterator;

	ClusterNo _get_current_cluster();
	ClusterNo _get_and_read_current_phys_cluster_or_allocate(char* data);
	FSIndex* index;

	char data[ClusterSize] = { 0 };

	BytesCnt filepos = 0;
	BytesCnt size = 0;
	ClusterNo n_clusters = 0;

	FSPartition* partition = nullptr;
	KernelFile* directory = nullptr;
	FileHandle* file_handle = nullptr;
	RWLock* lock = nullptr;
};