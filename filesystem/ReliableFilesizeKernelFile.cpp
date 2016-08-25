#include "ReliableFilesizeKernelFile.h"

ReliableFilesizeKernelFile::ReliableFilesizeKernelFile(FileHandle* file_handle) : KernelFile(file_handle)
{
}


ReliableFilesizeKernelFile::~ReliableFilesizeKernelFile()
{
}

BytesCnt ReliableFilesizeKernelFile::read(BytesCnt n, char* buffer) {
	BytesCnt len = min(n, this->getFileSize() - this->filePos());
	return KernelFile::read(len, buffer);
}