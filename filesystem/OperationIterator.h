#pragma once

#include "FS.h"
#include "KernelFile.h"

class OperationIterator
{
public:

	enum Operation { PARTIAL, FULL_CLUSTER };
	OperationIterator(KernelFile* file, BytesCnt len);
	~OperationIterator();

	void Walk();

	bool EOP();

	BytesCnt GetOperationLength();
	BytesCnt GetOperationClusteredOffset();
	BytesCnt GetOperationContiguousOffset();
	Operation GetOperation();

private:
	BytesCnt len;
	BytesCnt clustered_offset;
	BytesCnt contiguous_offset;
	KernelFile* file;

	BytesCnt current_operation_length = 0;
	Operation op;
};
