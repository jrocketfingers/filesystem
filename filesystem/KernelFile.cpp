#include <algorithm>
#include <cassert>
#include "KernelFile.h"
#include "FSDirectory.h"

using namespace std;

KernelFile::KernelFile(FileHandle* file_handle)
{
	partition = file_handle->GetPartition();
	index = file_handle->GetIndex();
	lock = file_handle->GetLock();
	this->file_handle = file_handle;
}

KernelFile::~KernelFile()
{
}

char KernelFile::write(BytesCnt len, const char* data_buffer) {
	if (len / ClusterSize > partition->GetNumOfFreeclusters())
		return 0;

	OperationIterator it(this, len);

	while (!it.EOP()) {
		it.Walk();

		if (it.GetOperation() == OperationIterator::Operation::PARTIAL) {
			char prep_buffer[2048];
			BytesCnt target_cluster = _get_and_read_current_phys_cluster_or_allocate(prep_buffer);

			memcpy(prep_buffer + it.GetOperationClusteredOffset(), data_buffer + it.GetOperationContiguousOffset(), it.GetOperationLength());

			partition->WriteCluster(target_cluster, prep_buffer);
			filepos += it.GetOperationLength();
		}
		else if (it.GetOperation() == OperationIterator::Operation::FULL_CLUSTER) {
			char prep_buffer[2048] = { 0 };

			ClusterNo target_cluster;
			if ((target_cluster = index->GetPhysCluster(_get_current_cluster())) == 0) {
				target_cluster = index->GetPhysCluster(index->Allocate());
			}

			partition->WriteCluster(target_cluster, data_buffer + it.GetOperationContiguousOffset());
			filepos += it.GetOperationLength();
		}
	}

	return 1;
}

BytesCnt KernelFile::read(BytesCnt len, char* data_buffer)
{
	OperationIterator it(this, len);

	BytesCnt bytes_read = 0;

	while (!it.EOP()) {
		it.Walk();

		if (it.GetOperation() == OperationIterator::Operation::PARTIAL) {
			char prep_buffer[2048];
			BytesCnt target_cluster = index->GetPhysCluster(_get_current_cluster());

			if (target_cluster == 0)
				break;

			partition->ReadCluster(target_cluster, prep_buffer);

			memcpy(data_buffer + it.GetOperationContiguousOffset(), prep_buffer + it.GetOperationClusteredOffset(), it.GetOperationLength());
			filepos += it.GetOperationLength();
			bytes_read += it.GetOperationLength();
		}
		else if (it.GetOperation() == OperationIterator::Operation::FULL_CLUSTER) {
			char prep_buffer[2048];
			BytesCnt target_cluster = index->GetPhysCluster(_get_current_cluster());

			if (target_cluster == 0)
				break;

			partition->ReadCluster(target_cluster, prep_buffer);
			memcpy(data_buffer + it.GetOperationContiguousOffset(), prep_buffer + it.GetOperationClusteredOffset(), it.GetOperationLength());
			filepos += it.GetOperationLength();
			bytes_read += it.GetOperationLength();
		}
	}

	return bytes_read;
}

char KernelFile::seek(BytesCnt loc)
{
	char ret;

	if (loc > file_handle->GetFilesize())
		ret = 0;
	else {
		filepos = loc;
		ret = 1;
	}

	return ret;
}

BytesCnt KernelFile::filePos()
{
	return filepos;
}

char KernelFile::eof()
{
	return filepos == file_handle->GetFilesize();
}

BytesCnt KernelFile::getFileSize()
{
	return file_handle->GetFilesize();
}

char KernelFile::truncate()
{
	index->DeallocateAll();
	file_handle->UpdateFilesize(0);

	return 1;
}

ClusterNo KernelFile::_get_current_cluster()
{
	return filepos / ClusterSize;
}

ClusterNo KernelFile::_get_and_read_current_phys_cluster_or_allocate(char* data)
{
	ClusterNo target_cluster;
	if ((target_cluster = index->GetPhysCluster(_get_current_cluster())) == 0) {
		target_cluster = index->GetPhysCluster(index->Allocate());
		memset(data, 0, ClusterSize);
	}
	else {
		partition->ReadCluster(target_cluster, data);
	}

	return target_cluster;
}
