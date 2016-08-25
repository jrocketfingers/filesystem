#include <algorithm>
#include "OperationIterator.h"

using namespace std;

OperationIterator::OperationIterator(KernelFile* file, BytesCnt len)
{
	this->file = file;
	this->len = len;
}

OperationIterator::~OperationIterator()
{
}

void OperationIterator::Walk()
{
	contiguous_offset += current_operation_length;

	if (file->filepos % ClusterSize != 0) {
		/* first cluster operation */
		/* how do we know that? we're not aligned to the cluster size */
		clustered_offset = file->filepos % ClusterSize;
		current_operation_length = min((file->_get_current_cluster() + 1) * 2048 - file->filepos, len);

		len -= current_operation_length;

		op = Operation::PARTIAL;
	}
	else if(len > ClusterSize) {
		clustered_offset = 0;
		current_operation_length = ClusterSize;
		len -= current_operation_length;

		op = Operation::FULL_CLUSTER;
	}
	else {
		/* final cluster operation */
		current_operation_length = len;
		clustered_offset = 0;
		len -= current_operation_length;

		op = Operation::PARTIAL;
	}
}

bool OperationIterator::EOP()
{
	return len == 0;
}

BytesCnt OperationIterator::GetOperationLength()
{
	return current_operation_length;
}

BytesCnt OperationIterator::GetOperationClusteredOffset()
{
	return clustered_offset;
}

BytesCnt OperationIterator::GetOperationContiguousOffset()
{
	return contiguous_offset;
}

OperationIterator::Operation OperationIterator::GetOperation()
{
	return op;
}
