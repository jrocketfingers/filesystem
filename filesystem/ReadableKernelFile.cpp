#include "ReadableKernelFile.h"



ReadableKernelFile::ReadableKernelFile(FileHandle* file_handle) : ReliableFilesizeKernelFile(file_handle)
{
	this->file_handle->incReaders();
}


ReadableKernelFile::~ReadableKernelFile()
{
	this->file_handle->decReaders();
}

char ReadableKernelFile::write(BytesCnt n, const char * buffer)
{
	return 0;
}
